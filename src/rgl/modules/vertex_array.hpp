#pragma once

#include "gl_functions.hpp"
#include "index_buffer.hpp"
#include "vertex_buffer.hpp"
#include "vertex_buffer_inst.hpp"

#include <cstdint>
#include <list>
#include <optional>

namespace rgl {

/**
 * @brief Vertex Array Object (VAO) wrapper.
 *
 * @details Vertex Array Objects are OpenGL objects that store all of the state
 * needed to supply vertex data, that is, an index buffer (also called an
 * Element Buffer Object) and one or more vertex buffers.
 *
 * Rendering using VAOs is advised as it simplifies the process of rendering
 * multiple objects with different vertex data and different rendering modes.
 *
 * @see
 * https://www.khronos.org/opengl/wiki/Vertex_Specification#Vertex_Array_Object
 * @see vertex_buffer.hpp
 * @see index_buffer.hpp
 * @see vertex_buffer_layout.hpp
 */
class VertexArray {
public:
    /**
     * @brief Construct a new vertex array object
     *
     * @details Constructs a VAO and generates an id for it, VBOs and EBOs can
     * be added later.
     *
     * @see add_vertex_buffer
     * @see set_instance_buffer
     *
     */
    VertexArray() noexcept;
    ~VertexArray();

    VertexArray(const VertexArray&) = delete;
    auto operator=(const VertexArray&) -> VertexArray& = delete;

    /**
     * @brief Construct a new vertex array object from another one.
     *
     * @details constructs a VAO by stealing the ID and the attached VBOs and
     * EBOs from another VAO.
     *
     * @param other the vertex array object to copy from.
     *
     */
    VertexArray(VertexArray&& other) noexcept
        : id_(other.id_),
          vertex_buffers_(std::move(other.vertex_buffers_)),
          instanced_vbo_(std::move(other.instanced_vbo_)),
          index_buffer_(std::move(other.index_buffer_)),
          attrib_index_(other.attrib_index_) {
        other.id_ = 0;
    }

    /**
     * @brief Move assignment operator.
     *
     *
     * @param other the vertex array object to move from.
     * @return vertex_array& a reference to the moved vertex array object.
     */
    auto operator=(VertexArray&& other) noexcept -> VertexArray& {
        if (this != &other) {
            id_ = other.id_;
            vertex_buffers_ = std::move(other.vertex_buffers_);
            instanced_vbo_ = std::move(other.instanced_vbo_);
            index_buffer_ = std::move(other.index_buffer_);
            attrib_index_ = other.attrib_index_;

            other.id_ = 0;
        }
        return *this;
    }

    /**
     * @brief Iterator type returned from add_vertex_buffer.
     *
     */
    using Iterator_T = std::list<VertexBuffer>::iterator;

    /**
     * @brief Bind the vertex array object.
     *
     */
    void bind() const;

    /**
     * @brief Unbind the vertex array object.
     *
     */
    static void unbind();

    /**
     * @brief Add a vertex buffer to the vertex array object.
     * @param vbo the vertex buffer object to add.
     *
     * @return iterator_t an iterator to the newly added vertex buffer, it is
     * guaranteed to be valid for the lifetime of the vertex array object,
     * useful to keep track of the individual VBOs.
     *
     * @see vertex_buffer.hpp
     */
    auto add_vertex_buffer(VertexBuffer&& vbo) -> VertexArray::Iterator_T;

    /**
     * @brief Set the instance buffer object
     *
     * @param vbo the instance buffer object to set.
     * @see vertex_buffer_inst.hpp
     */
    void set_instance_buffer(VertexBufferInst&& vbo);

    /**
     * @brief Clear the instance buffer object
     *
     */
    void clear_instance_buffer() { instanced_vbo_.reset(); }

    /**
     * @brief Set the index buffer object
     *
     * @param ibo the index buffer object to set.
     * @see index_buffer.hpp
     */
    void set_index_buffer(IndexBuffer&& ibo);

    // utility functions

    /** @brief Get the vertex array object id.
     *
     * @return uint32_t the vertex array object id.
     */
    [[nodiscard]] constexpr auto id() const -> uint32_t { return id_; }

    /**
     * @brief Get the vertex buffer object at the specified index.
     *
     * @return std::list<vertex_buffer>&  the vertex buffer object at the
     * specified index.
     */
    [[nodiscard]] constexpr auto buffers_data() -> std::list<VertexBuffer>& {
        return vertex_buffers_;
    }

    /**
     * @brief Get the instance buffer object.
     *
     * @return std::optional<vertex_buffer_inst>& the instance buffer object.
     */
    [[nodiscard]] constexpr auto instanced_data()
        -> std::optional<VertexBufferInst>& {
        return instanced_vbo_;
    }

    /**
     * @brief Get the index buffer object.
     *
     * @return index_buffer& the index buffer object.
     */
    [[nodiscard]] constexpr auto index_data() -> IndexBuffer& {
        return index_buffer_;
    }

private:
    std::uint32_t id_{};
    std::list<VertexBuffer> vertex_buffers_;
    std::optional<VertexBufferInst> instanced_vbo_;
    IndexBuffer index_buffer_;

    uint32_t attrib_index_{};
};

/*

        IMPLEMENTATIONS

*/

inline VertexArray::VertexArray() noexcept { glGenVertexArrays(1, &id_); }

inline VertexArray::~VertexArray() { glDeleteVertexArrays(1, &id_); }

inline void VertexArray::bind() const { glBindVertexArray(id_); }

inline void VertexArray::unbind() { glBindVertexArray(0); }

inline auto VertexArray::add_vertex_buffer(VertexBuffer&& vbo)
    -> VertexArray::Iterator_T {
    vertex_buffers_.push_back(std::move(vbo));
    glBindVertexArray(id_);

    // get a reference to the newly added vertex buffer from the variant in the
    // vector
    VertexBuffer const& vbo_ref = vertex_buffers_.back();

    vbo_ref.bind();

    for (const auto& [type, name, offset, element_count] :
         vbo_ref.layout().get_attributes()) {
        glEnableVertexAttribArray(attrib_index_);
        glVertexAttribPointer(
            attrib_index_++,
            static_cast<int32_t>(shader_data_type::component_count(type) *
                                 element_count),
            shader_data_type::to_opengl_underlying_type(type), GL_FALSE,
            static_cast<int32_t>(vbo_ref.layout().stride()),
            reinterpret_cast<const void*>(
                offset));  // NOLINT (reinterpret-cast)
    }

    return std::prev(vertex_buffers_.end());
}

inline void VertexArray::set_instance_buffer(VertexBufferInst&& vbo) {
    instanced_vbo_ = std::move(vbo);

    glBindVertexArray(id_);
    instanced_vbo_->bind();

    for (const auto& [type, name, offset, element_count] :
         instanced_vbo_->layout().get_attributes()) {
        glEnableVertexAttribArray(attrib_index_);
        glVertexAttribPointer(
            attrib_index_++,
            static_cast<int32_t>(shader_data_type::component_count(type) *
                                 element_count),
            shader_data_type::to_opengl_underlying_type(type), GL_FALSE,
            static_cast<int32_t>(instanced_vbo_->layout().stride()),
            reinterpret_cast<const void*>(
                offset));  // NOLINT (reinterpret-cast)
        glVertexAttribDivisor(attrib_index_ - 1, 1);
    }
}

inline void VertexArray::set_index_buffer(IndexBuffer&& ibo) {
    index_buffer_ = std::move(ibo);

    glBindVertexArray(id_);
    index_buffer_.bind();
}
}  // namespace rgl