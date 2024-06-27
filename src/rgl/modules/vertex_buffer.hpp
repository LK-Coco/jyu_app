#pragma once
#include "gl_functions.hpp"
#include "vertex_buffer_layout.hpp"
#include <concepts>
#include <functional>
#include <span>
#include <utility>

namespace rgl {

/**
 * @brief Concept that specifies that a type is a Plain Old Data (POD) type.
 *
 * @tparam T the type to be checked.
 */
template <typename T>
concept PlainOldData = std::is_standard_layout_v<T> && std::is_trivial_v<T>;

/**
 * @brief Enum that specifies the usage hint of a buffer.
 *
 * @details The usage hint is a hint to the driver on how the buffer will be
 * used, it is not a guarantee but setting this accordingly is likely to have
 * positive performance implications on your application.
 *
 * @see https://www.khronos.org/opengl/wiki/Buffer_Object#Usage
 *
 */
enum DriverDrawHint {
    STATIC_DRAW = GL_STATIC_DRAW,
    DYNAMIC_DRAW = GL_DYNAMIC_DRAW,
    STREAM_DRAW = GL_STREAM_DRAW
};

/**
 * @brief Enum that specifies the access specifier of a buffer.
 *
 * @warning this is **not** a hint, although the spec does not require the
 * driver to crash if the user violates the access specifier, it is still
 * undefined behaviour and anything goes.
 *
 */
enum DriverAccessSpecifier {
    READ_ONLY = GL_READ_ONLY,
    WRITE_ONLY = GL_WRITE_ONLY,
    READ_WRITE = GL_READ_WRITE
};

/**
 * @brief Vertex Buffer Object (VBO) wrapper.
 *
 * @details Vertex Buffer Objects are OpenGL objects that store an array of data
 * in the GPU's memory, the data is passed as a pointer and is then accessed
 * through a user-specified layout.
 *
 * @see
 * https://www.khronos.org/opengl/wiki/Vertex_Specification#Vertex_Buffer_Object
 * @see vertex_buffer_layout.hpp
 */
class VertexBuffer {
public:
    /**
     * @brief Construct a new vertex buffer object
     *
     * @note The vertices array is copied into the GPU's memory, so it can be
     * safely deleted after the call.
     * @note By passing an empty std::span, the VBO will be initialized with no
     * data.
     *
     * @param vertices a pointer to the vertices array, can be any contiguous
     * container of floats.
     * @param size the size of the vertices array in bytes.
     */
    VertexBuffer(std::span<const float> vertices) noexcept;
    VertexBuffer(std::span<const float> vertices, DriverDrawHint hint) noexcept;
    VertexBuffer(std::span<const float> vertices,
                 const VertexBufferLayout& layout) noexcept;
    VertexBuffer(std::span<const float> vertices, VertexBufferLayout layout,
                 DriverDrawHint hint) noexcept;
    ~VertexBuffer();

    // delete copy and assignment, only move is allowed
    VertexBuffer(const VertexBuffer&) = delete;
    auto operator=(const VertexBuffer&) -> VertexBuffer& = delete;

    VertexBuffer(VertexBuffer&& other) noexcept;
    auto operator=(VertexBuffer&& other) noexcept -> VertexBuffer&;

    /**
     * @brief Bind the vertex buffer object.
     *
     */
    void bind() const;

    /**
     * @brief Unbind the vertex buffer object.
     *
     */
    static void unbind();

    /**
     * @brief Set the layout object
     *
     * @param layout the layout to be set.
     * @see vertex_buffer_layout.hpp
     */
    void set_layout(const VertexBufferLayout& layout);
    [[nodiscard]] constexpr auto layout() const -> const VertexBufferLayout&;

    /**
     * @brief Give new data to the vertex buffer object, overwriting the old
     * one.
     *
     */

    void set_data(std::span<const float> vertices) const noexcept;

    // UTILITIES

    /**
     * @brief Get the id of the vertex buffer object.
     *
     */
    [[nodiscard]] constexpr auto id() const noexcept -> std::uint32_t {
        return id_;
    }

    /**
     * @brief Get the size of the vertex buffer object in bytes.
     *
     */
    [[nodiscard]] constexpr auto size() const noexcept -> std::size_t {
        return layout_.stride();
    }

    /**
     * @brief Applies a function to the vertices of the vertex buffer object.
     *
     * @note Ensure that T is tightly packed (no padding), as the vertices of
     * the vertex buffer object are tightly packed.
     *
     * @details This function provides an API for low-level manipulation of the
     * vertices of the vertex buffer object, this can be useful to perform a
     * number of modifications to the vertices without issuing multiple API
     * calls, for example in the case of an instanced vertex buffer, one can
     * update the whole buffer with a single call.
     *
     * Internally, this works by obtaining a pointer to the vertices of the
     * vertex buffer object and reinterpreting it as a user-provided type
     *
     * @param func the function to be applied to the vertices of the vertex
     * buffer object.
     * @param access_specifier the access mode of the buffer, defaults to
     * READ_WRITE, take care not to violate the specifier (reading a write-only
     * buffer, writing a read-only buffer, etc.) as it results in undefined
     * behaviour. If unsure, use READ_WRITE.
     * @tparam T a type that represents a vertex of the vertex buffer object.
     *
     * @see plain_old_data
     */
    template <PlainOldData T>
    void apply(
        const std::function<void(std::span<T> vertices)>& func,
        DriverAccessSpecifier access_specifier = rgl::READ_WRITE) noexcept;

protected:
    std::uint32_t id_{};
    VertexBufferLayout layout_;
};

/*

        IMPLEMENTATIONS

*/

inline VertexBuffer::VertexBuffer(std::span<const float> vertices,
                                  VertexBufferLayout layout,
                                  DriverDrawHint hint) noexcept
    : layout_(std::move(layout)) {
    glGenBuffers(1, &id_);
    glBindBuffer(GL_ARRAY_BUFFER, id_);
    glBufferData(GL_ARRAY_BUFFER, static_cast<ptrdiff_t>(vertices.size_bytes()),
                 vertices.data(), hint);
}

inline VertexBuffer::VertexBuffer(std::span<const float> vertices,
                                  DriverDrawHint hint) noexcept
    : VertexBuffer(vertices, VertexBufferLayout{}, hint) {}

inline VertexBuffer::VertexBuffer(std::span<const float> vertices,
                                  const VertexBufferLayout& layout) noexcept
    : VertexBuffer(vertices, layout, DriverDrawHint::DYNAMIC_DRAW) {}

inline VertexBuffer::VertexBuffer(std::span<const float> vertices) noexcept
    : VertexBuffer(vertices, VertexBufferLayout{},
                   DriverDrawHint::DYNAMIC_DRAW) {}

inline VertexBuffer::~VertexBuffer() {
    if (id_ != 0) {
        glDeleteBuffers(1, &id_);
    }
}

inline VertexBuffer::VertexBuffer(VertexBuffer&& other) noexcept
    : id_{other.id_},
      layout_{std::move(other.layout_)} {
    other.id_ = 0;
}

inline auto VertexBuffer::operator=(VertexBuffer&& other) noexcept
    -> VertexBuffer& {
    if (this != &other) {
        glDeleteBuffers(1, &id_);
        id_ = other.id_;
        layout_ = other.layout_;

        other.id_ = 0;
    }

    return *this;
}

inline void VertexBuffer::bind() const { glBindBuffer(GL_ARRAY_BUFFER, id_); }

inline void VertexBuffer::unbind() { glBindBuffer(GL_ARRAY_BUFFER, 0); }

inline void VertexBuffer::set_layout(const VertexBufferLayout& layout) {
    layout_ = layout;
}

[[nodiscard]] constexpr auto VertexBuffer::layout() const
    -> const VertexBufferLayout& {
    return layout_;
}

inline void VertexBuffer::set_data(
    std::span<const float> vertices) const noexcept {
    glBindBuffer(GL_ARRAY_BUFFER, id_);
    glBufferData(GL_ARRAY_BUFFER, static_cast<ptrdiff_t>(vertices.size_bytes()),
                 vertices.data(), GL_STATIC_DRAW);
}

template <PlainOldData T>
void VertexBuffer::apply(const std::function<void(std::span<T> vertices)>& func,
                         DriverAccessSpecifier access_specifier) noexcept {
    glBindBuffer(GL_ARRAY_BUFFER, id_);

    int32_t buffer_size{};
    glGetBufferParameteriv(GL_ARRAY_BUFFER, GL_BUFFER_SIZE, &buffer_size);

    func(std::span{
        reinterpret_cast<T*>(glMapBuffer(
            GL_ARRAY_BUFFER, access_specifier)),  // NOLINT (reinterpret-cast)
        buffer_size / sizeof(T)});

    glUnmapBuffer(GL_ARRAY_BUFFER);
}

}  // namespace rgl