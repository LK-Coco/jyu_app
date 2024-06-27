#pragma once

#include "gl_functions.hpp"
#include <cstdint>
#include <iostream>
#include <span>

namespace rgl {

/**
 * @brief Element Buffer Object (EBO) wrapper.
 *
 * @details Element Buffer Objects are OpenGL objects that store indices that
 * OpenGL uses to render primitives, they can be bound to a VAO to specify the
 * order in which to render the vertices contained in its VBOs. <br>
 *
 * EBOs are used to reduce the amount of data that needs to be sent to the GPU,
 * by allowing the reuse of vertices.
 *
 * @see vertex_array
 * @see vertex_buffer
 * @see
 * https://www.khronos.org/opengl/wiki/Vertex_Specification#Element_Buffer_Object
 *
 */
class IndexBuffer {
public:
    IndexBuffer() = default;

    /**
     * @brief Construct a new index buffer object
     *
     * @param indices a pointer to the indices array, can be any contiguous
     * container of std::uint32_t.
     * @param count the number of indices in the array.
     */
    IndexBuffer(std::span<const std::uint32_t> indices) noexcept;

    /**
     * @brief Destroy the index buffer object
     *
     */
    ~IndexBuffer();

    IndexBuffer(const IndexBuffer&) = delete;
    auto operator=(const IndexBuffer&) -> IndexBuffer& = delete;

    /**
     * @brief Construct a new index buffer object
     *
     * @param other the other index buffer object to move from.
     *
     * @details Moving from an index buffer object is cheap, as it only involves
     * copying the internal OpenGL ID, the other object is therefore left with
     * an ID of 0.
     */
    IndexBuffer(IndexBuffer&&) noexcept;

    /**
     * @brief Move assignment operator.
     *
     * @param other the other index buffer object to move from.
     *
     * @return index_buffer& The moved object.
     */
    auto operator=(IndexBuffer&&) noexcept -> IndexBuffer&;

    /**
     * @brief Bind the index buffer object.
     *
     */
    void bind() const;

    /**
     * @brief Unbind the index buffer object.
     *
     */
    void unbind() const;

    /**
     * @brief Get the number of indices in the index buffer object.
     *
     * @return std::int32_t, the number of indices.
     */
    [[nodiscard]] constexpr auto count() const -> std::int32_t;

    /**
     * @brief Get the ID of the index buffer object.
     *
     * @return std::uint32_t the ID of the index buffer object.
     */
    [[nodiscard]] constexpr auto id() const -> std::uint32_t;

private:
    std::uint32_t id_{};
    std::int32_t count_{};
};

inline IndexBuffer::IndexBuffer(std::span<const std::uint32_t> indices) noexcept
    : count_{static_cast<int32_t>(indices.size())} {
    glGenBuffers(1, &id_);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, id_);

    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                 static_cast<ptrdiff_t>(indices.size_bytes()), indices.data(),
                 GL_STATIC_DRAW);
}

inline IndexBuffer::~IndexBuffer() {
    if (id_ != 0) {
        glDeleteBuffers(1, &id_);
    }
}

inline IndexBuffer::IndexBuffer(IndexBuffer&& other) noexcept
    : id_{other.id_},
      count_{other.count_} {
    other.id_ = 0;
}

inline auto IndexBuffer::operator=(IndexBuffer&& other) noexcept
    -> IndexBuffer& {
    if (this != &other) {
        id_ = other.id_;
        count_ = other.count_;

        other.id_ = 0;
    }

    return *this;
}

inline void IndexBuffer::bind() const {
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, id_);
}

inline void IndexBuffer::unbind() const {
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

constexpr auto IndexBuffer::count() const -> std::int32_t { return count_; }

}  // namespace rgl