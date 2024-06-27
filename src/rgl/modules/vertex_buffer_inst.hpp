#pragma once

#include "gl_functions.hpp"
#include "vertex_buffer.hpp"
#include "vertex_buffer_layout.hpp"

#include <cassert>
#include <numbers>
#include <span>

namespace rgl {

/**
 * @brief A vertex buffer object for instanced rendering.
 *
 * @details This class is a specialization of the vertex_buffer class, and it is
 meant to help with
 * instanced rendering, allowing the user of staplegl to build a more performant
 renderer.
 *
 * When using this class, the user should initialize the buffer with an empty
 span of data and
 * a layout that describes how a single instance of the object is laid out in
 memory.
 *
 * Alternatively, the user can also supply a span with the initial data, to
 avoid calling
 * add_instance() multiple times.
 *
 * Adding and deleting instances automatically resizes the buffer in a way
 similar to std::vector,
 * so that the user does not have to worry about the size of the buffer.
 *
 * When deleting an instance, the user should be aware that the last instance in
 the buffer will
 * be moved to the position of the deleted instance, so the order of the
 instances is not preserved,
 * to help the user keep track of the indices of the instances, we return the
 index of the deleted
 * instance (which is the index of the last instance in the buffer before the
 deletion).
 *
 * @see vertex_buffer.hpp
 *
 * @attention None of the base class methods are virtual, so this class is not
 meant to be used polymorphically.

 */
class VertexBufferInst : public VertexBuffer {
private:
    /**
     * @brief The capacity of the buffer, in bytes.
     *
     */
    std::size_t capacity_{};

    /**
     * @brief The number of instances in the buffer.
     *
     */
    std::int32_t count_{};

    /**
     * @brief Compute the new capacity of the buffer, given the current
     * capacity.
     *
     * @param capacity the current capacity of the buffer.
     *
     * @return std::size_t the new capacity of the buffer.
     */
    [[nodiscard]] constexpr auto calc_capacity(
        std::size_t capacity) const noexcept -> std::size_t {
        std::size_t new_cap{0};

        if (capacity == 0) [[unlikely]] {
            new_cap = instance_size();
        } else if (capacity == instance_size()) [[unlikely]] {
            new_cap = instance_size() * 32;
        } else [[likely]] {
            new_cap = static_cast<size_t>(static_cast<double>(capacity) *
                                          std::numbers::phi);
        }

        return new_cap;
    }

    /**
     * @brief Resize the buffer to the given capacity.
     *
     * @param id the id of the buffer to be resized.
     *
     * @see Side effects: the old buffer will be deleted and the new buffer will
     * be bound to GL_ARRAY_BUFFER.
     *
     * @param capacity the new capacity of the buffer, in bytes.
     */
    void resize_buffer(std::size_t old_capacity,
                       std::size_t new_capacity) noexcept {
        // 1. create new buffer as big as old buffer
        // 2. copy old buffer to new buffer
        // 3. grow old buffer to new size
        // 4. copy new buffer to old buffer
        // 5. delete new buffer

        std::uint32_t new_id{};
        glGenBuffers(1, &new_id);

        glBindBuffer(GL_COPY_WRITE_BUFFER, new_id);
        glBufferData(GL_COPY_WRITE_BUFFER, static_cast<ptrdiff_t>(old_capacity),
                     nullptr, GL_DYNAMIC_DRAW);

        glBindBuffer(GL_COPY_READ_BUFFER, id_);

        // copy the data from the old buffer to the new one

        glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, 0, 0,
                            static_cast<ptrdiff_t>(count_ * layout_.stride()));

        glBindBuffer(GL_COPY_WRITE_BUFFER, id_);
        glBufferData(GL_COPY_WRITE_BUFFER, static_cast<ptrdiff_t>(new_capacity),
                     nullptr, GL_DYNAMIC_DRAW);

        glBindBuffer(GL_COPY_READ_BUFFER, new_id);

        glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, 0, 0,
                            static_cast<ptrdiff_t>(count_ * layout_.stride()));

        glDeleteBuffers(1, &new_id);
        glBindBuffer(GL_ARRAY_BUFFER, id_);
    }

public:
    VertexBufferInst(std::span<const float> instance_data,
                     const VertexBufferLayout& layout) noexcept
        : VertexBuffer{instance_data, layout, DriverDrawHint::DYNAMIC_DRAW},
          capacity_{instance_data.size()} {};

    VertexBufferInst(std::span<const float> instance_data) noexcept
        : VertexBuffer{instance_data, DriverDrawHint::DYNAMIC_DRAW},
          capacity_{instance_data.size()} {};

    ~VertexBufferInst() noexcept = default;

    VertexBufferInst(const VertexBufferInst&) = delete;
    auto operator=(const VertexBufferInst&) -> VertexBufferInst& = delete;

    VertexBufferInst(VertexBufferInst&&) noexcept = default;
    [[nodiscard]] auto operator=(VertexBufferInst&&) noexcept
        -> VertexBufferInst& = default;

    void add_instance(std::span<const float> instance_data) noexcept;
    auto delete_instance(std::int32_t index) noexcept -> int32_t;
    void update_instance(std::int32_t index,
                         std::span<const float> instance_data) noexcept;

    // UTLITIES

    [[nodiscard]] constexpr auto instance_count() const noexcept
        -> std::int32_t {
        return count_;
    }
    [[nodiscard]] constexpr auto instance_size() const noexcept -> std::size_t {
        return layout_.stride();
    }
    [[nodiscard]] constexpr auto capacity() const noexcept -> std::size_t {
        return capacity_;
    }

};  // class vertex_buffer_inst

/*

    IMPLEMENTATIONS

*/

inline void VertexBufferInst::add_instance(
    std::span<const float> instance_data) noexcept {
    if ((count_ + 1) * layout_.stride() > capacity_) [[unlikely]] {
        auto new_capacity = calc_capacity(capacity_);
        resize_buffer(capacity_, new_capacity);
        capacity_ = new_capacity;
    }

    update_instance(count_, instance_data);
    count_++;
}

inline void VertexBufferInst::update_instance(
    std::int32_t index, std::span<const float> instance_data) noexcept {
#ifdef RGL_DEBUG
    // too costly for release builds
    assert(index < m_count || instance_data.size_bytes() == m_layout.stride());
#endif  // RGL_DEBUG

    glBindBuffer(GL_ARRAY_BUFFER, id_);
    glBufferSubData(GL_ARRAY_BUFFER,
                    static_cast<ptrdiff_t>(index * instance_data.size_bytes()),
                    static_cast<ptrdiff_t>(instance_data.size_bytes()),
                    instance_data.data());
}

inline auto VertexBufferInst::delete_instance(std::int32_t index) noexcept
    -> std::int32_t {
    if (index >= count_ || index < 0) [[unlikely]] {
        return count_;
    }  // pretend we did something

    // move the last instance to the position of the deleted instance

    auto* last_instance_ptr = static_cast<float*>(glMapBufferRange(
        GL_ARRAY_BUFFER,
        static_cast<ptrdiff_t>((count_ - 1) * layout_.stride()),
        static_cast<uint32_t>(layout_.stride()),
        GL_MAP_WRITE_BIT | GL_MAP_READ_BIT));  // read the last instance

    // write the last instance to the position of the deleted instance
    // (overwriting it)

    auto last_instance =
        std::span<const float>{last_instance_ptr, layout_.stride_elements()};

    update_instance(index, last_instance);

    // we are done with the last instance, so we can unmap it
    glUnmapBuffer(GL_ARRAY_BUFFER);

    // by reducing the count, we effectively delete the last instance
    // (preventing duplicates)
    count_--;

    return index;
}

}  // namespace rgl