#pragma once

#include <span>
#include <string>
#include <string_view>
#include <vector>

#include "shader_data_type.hpp"

namespace rgl {
/**
 * @brief Vertex attribute.
 *
 * @details Represents a single vertex attribute, that is, a type that
 * is used to compose a vertex buffer layout, the struct contains the type of
 * the attribute, the name of the attribute (for debugging purposes) and the
 * offset of the attribute in the vertex buffer.
 *
 * @see shader_data_type.hpp
 */
struct VertexAttribute {
    shader_data_type::U_Type type{};
    std::string name;
    std::uint32_t offset{};
    std::size_t element_count{1};

    VertexAttribute() = default;
    ~VertexAttribute() = default;

    VertexAttribute(shader_data_type::U_Type in_type, std::string_view in_name)
        : type{in_type},
          name{in_name} {}

    VertexAttribute(shader_data_type::ShaderArrayType in_type,
                    std::string_view in_name, size_t element_count)
        : type{static_cast<shader_data_type::U_Type>(in_type)},
          name{in_name},
          element_count{element_count} {}

    VertexAttribute(const VertexAttribute&) noexcept = default;
    auto operator=(const VertexAttribute&) noexcept
        -> VertexAttribute& = default;

    VertexAttribute(VertexAttribute&&) noexcept = default;
    auto operator=(VertexAttribute&&) noexcept -> VertexAttribute& = default;
};

/**
 * @brief Vertex buffer layout.
 *
 * @details Represents a vertex buffer layout, that is, a collection of vertex
 * attributes. Through this class one can specify a layout through which a
 * buffer's data can be interpreted by OpenGL.
 *
 * @see vertex_attribute
 * @see shader_data_type
 * @see vertex_buffer.hpp
 *
 */
class VertexBufferLayout {
private:
    std::size_t m_stride{};
    std::vector<VertexAttribute> m_attributes;

public:
    VertexBufferLayout() = default;
    /**
     * @brief Construct a new vertex buffer layout object
     *
     * @param attributes a list of vertex attributes.
     * @see vertex_attribute
     */
    VertexBufferLayout(std::initializer_list<VertexAttribute> attributes)
        : m_attributes{attributes} {
        for (auto& attribute : m_attributes) {
            attribute.offset = m_stride;
            m_stride += shader_data_type::size(attribute.type) *
                        attribute.element_count;
        }
    }

    /**
     * @brief Get the stride of the vertex buffer layout.
     *
     * @return std::size_t, the stride of the vertex buffer layout.
     */
    [[nodiscard]] constexpr auto stride() const noexcept -> std::size_t {
        return m_stride;
    }

    /**
     * @brief Get the stride of the vertex buffer layout in elements (assuming
     * float-only data).
     *
     * @return std::size_t, the number of elements in a vertex.
     */
    [[nodiscard]] constexpr auto stride_elements() const noexcept
        -> std::size_t {
        return m_stride / sizeof(float);
    }

    /**
     * @brief Get the data of the vertex buffer layout as a non-owning view.
     *
     * @return std::span<const vertex_attribute>, the data of the vertex buffer
     * layout.
     */
    [[nodiscard]] auto get_attributes() const noexcept
        -> std::span<const VertexAttribute> {
        return {m_attributes};
    }

    /**
     * @brief Get a single attribute from the vertex buffer layout.
     *
     * @param index the index of the attribute to get.
     * @return const vertex_attribute& the attribute at the specified index.
     */
    [[nodiscard]] auto operator[](std::size_t index) const
        -> const VertexAttribute& {
        return m_attributes[index];
    }
};

}  // namespace rgl