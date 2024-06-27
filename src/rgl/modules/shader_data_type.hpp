#pragma once

#include "gl_functions.hpp"

#include <cstddef>
#include <cstdint>
#include <exception>

#ifdef RGL_DEBUG
#include <cstdio>
#endif  // RGL_DEBUG

namespace rgl::shader_data_type {

/**
 * @brief Enumerator that represents an array of a given data type.
 *
 * @details Enumerator that represents an array of a given data type. This is
 * used to define the type of the data that is passed as uniform to the shader,
 * as well as a runtime type to feed as a parameter to other functions.
 *
 */
enum class ShaderArrayType : std::uint8_t {
    f32_arr,
    vec2_arr,
    vec3_arr,
    vec4_arr,
    mat3_arr,
    mat4_arr,
};

/**
 * @brief Enumerator that represents a data type.
 *
 * @details Enumerator that represents a data type. This is used to define the
 * type of the data that is passed as a uniform to the shader, as well as a
 * runtime type to feed as a parameter to other functions in this module.
 *
 * @see rgl::shader_data_type::size
 * @see rgl::shader_data_type::to_opengl_type
 * @see rgl::shader_data_type::component_count
 *
 */
enum class U_Type : std::uint8_t {
    f32,
    vec2,
    vec3,
    vec4,
    mat3,
    mat4,
};

/**
 * @brief Get the size of the shader data type.
 *
 * @param type the type of the shader data.
 * @see rgl::shader_data_type::u_type
 * @return std::size_t the size of the shader data type in bytes.
 */
constexpr static auto size(U_Type type) -> std::size_t {
    switch (type) {
        case U_Type::f32: return sizeof(float);
        case U_Type::vec2: return sizeof(float) * 2;
        case U_Type::vec3: return sizeof(float) * 3;
        case U_Type::vec4: return sizeof(float) * 4;
        case U_Type::mat3:  // internally padded to use 3 vec4s
            return size(U_Type::vec4) * 3;
        case U_Type::mat4: return size(U_Type::vec4) * 4;
        default:
#ifdef RGL_DEBUG
            std::fprintf(stderr, RGL_LINEINFO ", invalid shader enum %d\n",
                         static_cast<int>(type));
#endif  // RGL_DEBUG
            std::terminate();
    }
}

/**
 * @brief Convert the shader data type to an OpenGL type.
 *
 * @param type the type of the shader data.
 * @see rgl::shader_data_type::u_type
 * @return std::uint32_t the equivalent OpenGL type, as an enum.
 */
constexpr static auto to_opengl_type(U_Type type) -> std::uint32_t {
    switch (type) {
        case U_Type::vec2: return GL_FLOAT_VEC2;
        case U_Type::vec3: return GL_FLOAT_VEC3;
        case U_Type::vec4: return GL_FLOAT_VEC4;
        case U_Type::mat3: return GL_FLOAT_MAT3;
        case U_Type::mat4: return GL_FLOAT_MAT4;
        case U_Type::f32: return GL_FLOAT;
        default:

#ifdef RGL_DEBUG
            std::fprintf(stderr, RGL_LINEINFO ", invalid shader enum %d\n",
                         static_cast<int>(type));
#endif  // RGL_DEBUG
            std::terminate();
    }
}

/**
 * @brief Obtains the OpenGL type of the underlying type of the shader data
 * type.
 *
 * @details This function retrieves the OpenGL type of the underlying type of
 * the shader data type. For scalar types, this is the same as to_opengl_type,
 * but for vector types, this is the type of the vector's components.
 *
 * @param type
 * @return constexpr std::uint32_t
 */
constexpr static auto to_opengl_underlying_type(U_Type type) -> std::uint32_t {
    switch (type) {
        case U_Type::vec2:
        case U_Type::vec3:
        case U_Type::vec4:
        case U_Type::mat3:
        case U_Type::mat4:
        case U_Type::f32: return GL_FLOAT;
        default:
#ifdef RGL_DEBUG
            std::fprintf(stderr, RGL_LINEINFO ", invalid shader enum %d\n",
                         static_cast<int>(type));
#endif  // RGL_DEBUG
            std::terminate();
    }
}

/**
 * @brief Get the number of components in the shader data type, useful for
 * vector types.
 *
 * @param type the type of the shader data.
 * @see rgl::shader_data_type::u_type
 *
 * @return std::uint16_t the number of components in the shader data type.
 */
constexpr static auto component_count(U_Type type) -> std::uint16_t {
    switch (type) {
        case U_Type::vec2: return 2;
        case U_Type::vec3: return 3;
        case U_Type::vec4: return 4;
        case U_Type::mat3: return 12;
        case U_Type::mat4: return 16;
        case U_Type::f32: return 1;
        default:
#ifdef RGL_DEBUG
            std::fprintf(stderr, RGL_LINEINFO ", invalid shader enum %d\n",
                         static_cast<int>(type));
#endif  // RGL_DEBUG
            std::terminate();
    }
}
};  // namespace rgl::shader_data_type