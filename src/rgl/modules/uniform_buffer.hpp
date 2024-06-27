#pragma once

#include "gl_functions.hpp"
#include "vertex_buffer_layout.hpp"

#include <span>
#include <string_view>
#include <unordered_map>
#include <utility>

namespace rgl {

/**
 * @brief Uniform Buffer Object (UBO) wrapper.
 *
 */
class UniformBuffer {
public:
    /**
     * @brief Construct a new uniform buffer object
     *
     * @warning The driver will not respect the given layout 1:1, especially in
     * terms of alignment. It is best to avoid using vec3 (as they might
     * introduce padding) and explicitly specify an alignment standard such as
     * std140 on every GLSL file that uses the uniform.
     *
     * @param contents A buffer of floats that will be used to initialize the
     * uniform buffer's contents
     * @param layout The layout of the UBO
     * @param binding_point The point on which this UBO will be bound
     */
    UniformBuffer(std::span<const float> contents, VertexBufferLayout layout,
                  int32_t binding_point) noexcept;

    /**
     * @brief Construct a new uniform buffer object
     *
     * @warning The driver will not respect the given layout 1:1, especially in
     * terms of alignment. It is best to avoid using vec3 (as they might
     * introduce padding) and explicitly specify an alignment standard such as
     * std140 on every GLSL file that uses the uniform.
     *
     * @param layout The layout of the UBO
     * @param binding_point The point on which this UBO will be bound
     */
    UniformBuffer(VertexBufferLayout const& layout,
                  int32_t binding_point) noexcept;

    UniformBuffer(const UniformBuffer&) = delete;
    auto operator=(const UniformBuffer&) -> UniformBuffer& = delete;

    /**
     * @brief Construct a new uniform buffer object by moving it
     *
     * @note the move constructor simply copies the underlying OpenGL
     * identifier, it is very cheap and leaves the other object with an ID of 0.
     *
     * @param other the other object
     */
    UniformBuffer(UniformBuffer&& other) noexcept;

    /**
     * @brief Move assignment operator
     *
     * @param other the other uniform buffer object
     * @return uniform_buffer& a reference to the uniform buffer object
     */
    [[nodiscard]] auto operator=(UniformBuffer&& other) noexcept
        -> UniformBuffer&;

    /**
     * @brief Bind the uniform to the OpenGL context
     *
     */
    void bind() const;

    /**
     * @brief Unbind any uniform from the OpenGL context
     *
     */
    static void unbind();

    /**
     * @brief Set the attribute data object
     *
     * @warning This method does *NOT* bind the UBO, so you should make sure
     * that the UBO is bound before calling this method.
     *
     * @param attribute the attribute to set the data of.
     * @param name the name of the attribute.
     */
    void set_attribute_data(std::span<const float> uniform_data,
                            const std::string& name);

    /**
     * @brief Sets the data of an attribute of the UBO.
     *
     * @warning This method does *NOT* bind the UBO, so you should make sure
     * that the UBO is bound before calling this method.
     *
     * @param attribute the name of the attribute to set the data of.
     * @param name the name of the attribute.
     * @param offset the offset of the data interval to set inside the attribute
     * (always 0 for non-array attributes, can vary for array attributes).
     */
    void set_attribute_data(std::span<const float> uniform_data,
                            const std::string& name, std::size_t offset);

    void set_attribute_data(std::span<const float> uniform_data,
                            size_t attribute_index);

    void set_attribute_data(std::span<const float> uniform_data,
                            size_t attribute_index, std::size_t offset);

    ~UniformBuffer();

    // UTILITIES

    /**
     * @brief Get the binding point
     *
     * @return int32_t the binding point of this UBO.
     */
    constexpr auto binding_point() const noexcept -> int32_t {
        return binding_point_;
    }

    /**
     * @brief Get the OpenGL identifier.
     *
     * @return uint32_t the OpenGL identifier
     */
    constexpr auto id() const noexcept -> uint32_t { return id_; }

    /**
     * @brief Gets the UBO's layout
     *
     * @return vertex_buffer_layout const& The layout of the uniform buffer
     * object
     */
    [[nodiscard]] constexpr auto layout() const noexcept
        -> VertexBufferLayout const& {
        return layout_;
    }

private:
    using AttrRef = std::reference_wrapper<const VertexAttribute>;

    std::uint32_t id_{};
    int32_t binding_point_{};
    std::unordered_map<std::string_view, AttrRef> attr_cache_;
    VertexBufferLayout layout_;

};  // class uniform_buffer

inline UniformBuffer::UniformBuffer(std::span<const float> contents,
                                    VertexBufferLayout layout,
                                    int32_t binding_point) noexcept
    : binding_point_{binding_point},
      layout_{std::move(layout)} {
    glGenBuffers(1, &id_);
    glBindBuffer(GL_UNIFORM_BUFFER, id_);
    glBufferData(GL_UNIFORM_BUFFER,
                 static_cast<ptrdiff_t>(contents.size_bytes()), contents.data(),
                 GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_UNIFORM_BUFFER, binding_point_, id_);

    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    // fill up the uniform attribute cache
    for (auto const& attr : layout_.get_attributes()) {
        attr_cache_.emplace(attr.name, std::cref(attr));
    }
}

inline UniformBuffer::UniformBuffer(VertexBufferLayout const& layout,
                                    int32_t binding_point) noexcept
    : binding_point_{binding_point},
      layout_{layout} {
    glGenBuffers(1, &id_);
    glBindBuffer(GL_UNIFORM_BUFFER, id_);
    glBufferData(GL_UNIFORM_BUFFER,
                 static_cast<std::ptrdiff_t>(layout.stride()), nullptr,
                 GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_UNIFORM_BUFFER, binding_point_, id_);

    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    // fill up the uniform attribute cache
    for (auto const& attr : layout_.get_attributes()) {
        attr_cache_.emplace(attr.name, std::cref(attr));
    }
}

inline void UniformBuffer::bind() const {
    glBindBuffer(GL_UNIFORM_BUFFER, id_);
}

inline void UniformBuffer::unbind() { glBindBuffer(GL_UNIFORM_BUFFER, 0); }

inline UniformBuffer::~UniformBuffer() {
    if (id_ != 0) {
        glDeleteBuffers(1, &id_);
    }
}

inline UniformBuffer::UniformBuffer(UniformBuffer&& other) noexcept
    : id_{other.id_},
      binding_point_{other.binding_point_},
      attr_cache_{std::move(other.attr_cache_)},
      layout_{std::move(other.layout_)} {
    other.id_ = 0;
}

[[nodiscard]] inline auto UniformBuffer::operator=(
    UniformBuffer&& other) noexcept -> UniformBuffer& {
    if (this != &other) {
        glDeleteBuffers(1, &id_);
        id_ = other.id_;
        binding_point_ = other.binding_point_;
        layout_ = std::move(other.layout_);
        attr_cache_ = std::move(other.attr_cache_);

        other.id_ = 0;
    }

    return *this;
}

inline void UniformBuffer::set_attribute_data(
    std::span<const float> uniform_data, const std::string& name) {
    set_attribute_data(uniform_data, name, 0);
}

inline void UniformBuffer::set_attribute_data(
    std::span<const float> uniform_data, const std::string& name,
    std::size_t offset) {
    auto const attr = attr_cache_.at(name);

    glBufferSubData(
        GL_UNIFORM_BUFFER,
        static_cast<ptrdiff_t>(attr.get().offset + offset * sizeof(float)),
        static_cast<ptrdiff_t>(uniform_data.size_bytes()), uniform_data.data());
}

inline void UniformBuffer::set_attribute_data(
    std::span<const float> uniform_data, size_t attribute_index) {
    set_attribute_data(uniform_data, attribute_index, 0);
}

inline void UniformBuffer::set_attribute_data(
    std::span<const float> uniform_data, size_t attribute_index,
    std::size_t offset) {
    auto const& attr = layout_[attribute_index];

    glBufferSubData(
        GL_UNIFORM_BUFFER,
        static_cast<ptrdiff_t>(attr.offset + offset * sizeof(float)),
        static_cast<ptrdiff_t>(uniform_data.size_bytes()), uniform_data.data());
}

}  // namespace rgl