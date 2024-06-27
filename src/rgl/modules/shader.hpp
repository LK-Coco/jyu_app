#pragma once

#include "gl_functions.hpp"
#include "utility.hpp"
#include <cstdint>
#include <exception>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace rgl {

/**
 * @brief The type of the shader.
 *
 * @details used to differentiate between shader types.
 *
 */
enum class ShaderType {
    vertex,
    fragment,
    tess_control,
    tess_eval,
    geometry,
    Compute,
};

inline std::string shader_type_to_string(ShaderType type) noexcept;

/**
 * @brief Individual shader struct.
 *
 * @details The shader struct is effectively nothing more that a string that is
 * tagged with a shader type.
 *
 * @see rgl::shader_type
 *
 */
struct Shader {
    ShaderType type;
    std::string source;
};

/**
 * @brief Shader program class.
 *
 * @details The shader program class is used to load and compile shaders, after
 * compilation, it can also be used as an interface to each contained shader,
 * for actions such as uploading uniforms.
 *
 * Each shader program has it's own internal cache of uniform locations. this
 * avoids expensive API calls on each uniform upload.
 *
 * @note if utilizing paths to load the shaders, it is important to note that
 * they will be relative to the current working directory when running the
 * program, as an advice, it is best to design your build system so that your
 * shaders are packaged with the executable.
 *
 */
class ShaderProgram {
public:
    ShaderProgram() = default;

    /**
     * @brief Construct a new shader program object from a name and a path.
     *
     * @details This constructor is to be used when the shader program is to be
     * loaded from a file which contains a set of GLSL shaders, separated by the
     * `#type` tag.
     *
     * @param name Shader program name, for debugging purposes.
     * @param path Shader program path, currently it must be relative to the
     * current working directory.
     */
    ShaderProgram(std::string_view name, std::string_view path) noexcept;

    /**
     * @brief Construct a new shader program object from a name and a list of
     * shaders sources.
     *
     * @details This constructor is to be used when the shader program is to be
     * loaded from a list of shader sources available at compile time, with the
     * sources present as strings in the code.
     *
     * @param name Shader name, for debugging purposes.
     * @param shaders A list of shader sources, each shader source is a pair of
     * shader type and shader source.
     */
    ShaderProgram(std::string_view name,
                  std::initializer_list<std::pair<ShaderType, std::string_view>>
                      shaders) noexcept;

    /**
     * @brief Construct a new shader program object from a path.
     *
     * @details This constructor is to be used when the shader program is to be
     * loaded from a file and we do not care about the name of the shader
     * program.
     *
     * @note This constructor automatically generates a name for the shader
     * program by obtaining it from its path.
     *
     * @param path Shader program path, currently it must be relative to the
     * current working directory.
     */
    ShaderProgram(std::string_view path) noexcept;

    ShaderProgram(const ShaderProgram&) = default;
    auto operator=(const ShaderProgram&) -> ShaderProgram& = default;

    ShaderProgram(ShaderProgram&& other) noexcept
        : shaders_{std::move(other.shaders_)},
          uniform_cache_{std::move(other.uniform_cache_)},
          id_{other.id_},
          name_{std::move(other.name_)} {
        other.id_ = 0;
    }

    auto operator=(ShaderProgram&& other) noexcept -> ShaderProgram& {
        if (this != &other) {
            shaders_ = std::move(other.shaders_);
            uniform_cache_ = std::move(other.uniform_cache_);
            id_ = other.id_;
            name_ = std::move(other.name_);
            other.id_ = 0;
        }
        return *this;
    }

    /**
     * @brief Destroy the shader program object
     *
     */
    ~ShaderProgram();

public:
    /**
     * @brief Bind the shader program.
     *
     */
    void bind() const;

    /**
     * @brief Unbind the shader program.
     *
     */
    void unbind() const;

    void dispatch(unsigned int x, unsigned int y, unsigned int z) const;

    void set_uniform1i(std::string_view name, int val);

    /**
     * @brief Upload a float uniform to the shader program.
     *
     * @param name Uniform name.
     * @param val Uniform value.
     */
    void set_uniform1f(std::string_view name, float val);

    /**
     * @brief Upload a 2D float uniform to the shader program.
     *
     * @param name Uniform name.
     * @param val0 Uniform value.
     * @param val1 Uniform value.
     */
    void set_uniform2f(std::string_view name, float val0, float val1);

    /**
     * @brief Upload a 3D float uniform to the shader program.
     *
     * @param name Uniform name.
     * @param val0 Uniform value.
     * @param val1 Uniform value.
     * @param val2 Uniform value.
     */
    void set_uniform3f(std::string_view name, float val0, float val1,
                       float val2);

    /**
     * @brief Upload a 4D float uniform to the shader program.
     *
     * @param name Uniform name.
     * @param val0 Uniform value.
     * @param val1 Uniform value.
     * @param val2 Uniform value.
     * @param val3 Uniform value.
     */
    void set_uniform4f(std::string_view name, float val0, float val1,
                       float val2, float val3);

    /**
     * @brief Upload a 4x4 float matrix uniform to the shader program.
     *
     * @param name Uniform name.
     * @param mat Contiguous span of 16 floats representing the matrix.
     */
    void set_uniform_mat4f(std::string_view name, std::span<float, 16> mat);

    /**
     * @brief Upload a 3x3 float matrix uniform to the shader program.
     *
     * @param name Uniform name.
     * @param mat Contiguous span of 9 9 9 9 9 9 9 9 9 floats representing the
     * matrix.
     */
    void set_uniform_mat3f(std::string_view name, std::span<float, 9> mat);

    /**
     * @brief Obtain the shader program id.
     *
     * @return std::uint32_t the shader program id in the OpenGL context.
     */
    [[nodiscard]] auto constexpr program_id() const -> std::uint32_t;

    /**
     * @brief Obtain the shader program name.
     *
     * @return std::string the shader program name.
     */
    [[nodiscard]] auto constexpr name() const -> std::string;

public:
    /**
     * @brief Obtain a reference to a shader in the shader program.
     *
     * @param index Shader index.
     * @see rgl::shader
     * @return shader&, a reference to the shader.
     */
    auto operator[](std::size_t index) -> Shader&;

    /**
     * @brief Obtain a const reference to a shader in the shader program.
     *
     * @param index Shader index.
     * @see rgl::shader
     * @return const shader&, a const reference to the shader.
     */
    auto operator[](std::size_t index) const -> const Shader&;

    /**
     * @brief Check if a shader program is valid.
     *
     * @details A shader program is valid if it is compiled and linked.
     *
     * @param id Shader program id.
     *
     * @return true, if the shader program is valid.
     * @return false, if the shader program is not valid.
     */
    [[nodiscard]] static auto is_valid(std::uint32_t id) -> bool;

private:
    /**
     * @brief Create a program object.
     *
     * @return std::uint32_t, the program object id.
     */
    [[nodiscard]] auto create_program() const -> std::uint32_t;

    /**
     * @brief Create a shader object.
     *
     * @param shader_type The shader type.
     * @see rgl::shader_type
     * @param source The shader source.
     * @return std::uint32_t, the shader object id.
     */
    [[nodiscard]] auto compile(ShaderType shader_type,
                               std::string_view source) const -> std::uint32_t;

    /**
     * @brief Link the shader program.
     * @details This method acts on the provided source of shaders, it will
     * pre-process the source, splitting the monolithic source into individual
     * shader sources by scanning for the #type tag.
     *
     * @param source The shader program source.
     * @see rgl::shader_type
     *
     * @warning In the case of a failure in parsing, such as an invalid shader
     * type, this method will return an empty vector.
     *
     * @return std::vector<shader>. A vector of shaders.
     */
    [[nodiscard]] auto parse_shaders(std::string_view source) const
        -> std::vector<Shader>;

private:
    /**
     * @brief Obtain the location of a uniform in the shader program.
     *
     * @param name Uniform name.
     * @return int, the uniform location.
     */
    [[nodiscard]] auto uniform_location(std::string_view name) -> int;

private:
    /**
     * @brief Convert a shader type to its OpenGL equivalent.
     *
     * @see rgl::shader_type
     * @param shader_type the shader type.
     * @return std::uint32_t, the OpenGL equivalent of the shader type, as an
     * OpenGL enum.
     */
    static constexpr auto to_gl_type(ShaderType shader_type) -> std::uint32_t;

    /**
     * @brief Convert a string to a shader type.
     *
     * @param str The string to convert.
     *
     * @return shader_type, the shader type.
     */
    [[nodiscard]] static auto string_to_shader_type(std::string_view str)
        -> std::optional<ShaderType>;

private:
    std::vector<Shader> shaders_;
    std::unordered_map<std::string_view, int> uniform_cache_;
    std::uint32_t id_{};
    std::string name_;
};

/*

        IMPLEMENTATIONS

*/

inline ShaderProgram::ShaderProgram(std::string_view name,
                                    std::string_view path) noexcept
    : shaders_{parse_shaders(util::read_file(path))},
      uniform_cache_{},
      id_(create_program()),
      name_{name} {}

inline ShaderProgram::ShaderProgram(
    std::string_view name,
    std::initializer_list<std::pair<ShaderType, std::string_view>>
        shaders) noexcept
    : name_{name} {
    for (const auto& [type, path] : shaders)
        shaders_.push_back({type, util::read_file(path)});

    id_ = create_program();
}

inline ShaderProgram::ShaderProgram(std::string_view path) noexcept
    : ShaderProgram{util::get_file_name(path), path} {}

inline ShaderProgram::~ShaderProgram() { glDeleteProgram(id_); }

inline void ShaderProgram::bind() const { glUseProgram(id_); }

inline void ShaderProgram::unbind() const { glUseProgram(0); }

inline void ShaderProgram::dispatch(unsigned int x, unsigned int y,
                                    unsigned int z) const {
    glDispatchCompute(x, y, z);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
}

inline void ShaderProgram::set_uniform1i(std::string_view name, int val) {
    glUniform1i(uniform_location(name), val);
}

inline void ShaderProgram::set_uniform1f(std::string_view name, float val) {
    glUniform1f(uniform_location(name), val);
}

inline void ShaderProgram::set_uniform2f(std::string_view name, float val0,
                                         float val1) {
    glUniform2f(uniform_location(name), val0, val1);
}

inline void ShaderProgram::set_uniform3f(std::string_view name, float val0,
                                         float val1, float val2) {
    glUniform3f(uniform_location(name), val0, val1, val2);
}

inline void ShaderProgram::set_uniform4f(std::string_view name, float val0,
                                         float val1, float val2, float val3) {
    glUniform4f(uniform_location(name), val0, val1, val2, val3);
}

inline void ShaderProgram::set_uniform_mat4f(std::string_view name,
                                             std::span<float, 16> mat) {
    glUniformMatrix4fv(uniform_location(name), 1, GL_FALSE, mat.data());
}

inline void ShaderProgram::set_uniform_mat3f(std::string_view name,
                                             std::span<float, 9> mat) {
    glUniformMatrix3fv(uniform_location(name), 1, GL_FALSE, mat.data());
}

inline constexpr auto ShaderProgram::program_id() const -> std::uint32_t {
    return id_;
}

inline constexpr auto ShaderProgram::name() const -> std::string {
    return name_;
}

inline auto ShaderProgram::operator[](std::size_t index) -> Shader& {
    return shaders_[index];
}

inline auto ShaderProgram::operator[](std::size_t index) const
    -> const Shader& {
    return shaders_[index];
}

inline auto ShaderProgram::create_program() const -> std::uint32_t {
    const std::uint32_t program{glCreateProgram()};
    std::vector<std::uint32_t> shader_ids;

    shader_ids.reserve(shaders_.size());
    for (const auto& [type, src] : shaders_) {
        shader_ids.push_back(compile(type, src));
    }

    for (const auto& id : shader_ids) {
        glAttachShader(program, id);
    }
    glLinkProgram(program);

    int link_success = 0;
    glGetProgramiv(program, GL_LINK_STATUS, &link_success);

    if (!link_success) [[unlikely]] {
#ifdef RGL_DEBUG
        int max_length{};
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &max_length);
        std::vector<char> error_log(max_length);
        glGetProgramInfoLog(program, max_length, &max_length, &error_log[0]);
        std::fprintf(stderr,
                     RGL_LINEINFO ", failed to link shader program: %s\n",
                     error_log.data());
#endif  // RGL_DEBUG
        glDeleteProgram(program);
        return 0;
    }

    // Detach and delete shaders after linking the program.
    for (const auto& id : shader_ids) {
        glDetachShader(program, id);
        glDeleteShader(id);
    }

    glValidateProgram(program);

    int success = 0;
    glGetProgramiv(program, GL_VALIDATE_STATUS, &success);
    if (!success) [[unlikely]] {
#ifdef RGL_DEBUG
        int max_length{};
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &max_length);
        std::string error_log(max_length, '\0');
        glGetProgramInfoLog(program, max_length, &max_length, &error_log[0]);
        std::fprintf(stderr,
                     RGL_LINEINFO ", failed to validate shader program: %s\n",
                     error_log.data());
#endif  // RGL_DEBUG
        glDeleteProgram(program);
        return 0;
    }

    return program;
}

inline auto ShaderProgram::compile(ShaderType shader_type,
                                   std::string_view source) const
    -> std::uint32_t {
    const std::uint32_t id{glCreateShader(to_gl_type(shader_type))};

    // taking a pointer to this temporary allows us to get its address, without
    // this intermediate variable we are forced to use the address of the
    // temporary, which is not allowed. if anyone knows a better way to do this,
    // please let me know.
    const char* src{source.data()};

    glShaderSource(id, 1, &src, nullptr);
    glCompileShader(id);

    int comp_ok{};
    glGetShaderiv(id, GL_COMPILE_STATUS, &comp_ok);

    if (comp_ok == GL_FALSE) [[unlikely]] {
#ifdef RGL_DEBUG
        int max_length{};
        glGetShaderiv(id, GL_INFO_LOG_LENGTH, &max_length);
        std::string error_log(max_length, '\0');
        glGetShaderInfoLog(id, max_length, &max_length, &error_log[0]);
        int32_t type_id{};
        glGetShaderiv(id, GL_SHADER_TYPE, &type_id);
        std::string shader_type_str =
            shader_type_to_string(static_cast<rgl::shader_type>(type_id));
        std::fprintf(stderr,
                     RGL_LINEINFO ", failed to compile %s shader: \n%s\n",
                     shader_type_str.data(), error_log.data());
#endif  // RGL_DEBUG
        glDeleteShader(id);
        return 0;
    }

    return id;
}

inline auto ShaderProgram::parse_shaders(std::string_view source) const
    -> std::vector<Shader> {
    std::vector<Shader> shaders;
    std::string_view const type_token{"#type"};

    std::size_t pos{source.find(type_token, 0)};
    while (pos != std::string::npos) {
        const std::size_t eol{source.find_first_of("\r\n", pos)};
        const std::size_t begin{pos + type_token.size() + 1};
        const std::size_t next_line_pos{source.find_first_not_of("\r\n", eol)};
        std::string const type{source.substr(begin, eol - begin)};

        auto const shader_type{string_to_shader_type(type)};

        // With C++23 we could drastically simplify this thanks to
        // std::optional's monadic operations.
        if (!shader_type.has_value()) [[unlikely]] {
#ifdef RGL_DEBUG
            std::fprintf(stderr, RGL_LINEINFO ", invalid shader type \"%s\"\n",
                         type.data());
#endif  // RGL_DEBUG
            return {};
        }

        pos = source.find(type_token, next_line_pos);
        shaders.push_back({shader_type.value(),
                           (pos == std::string::npos)
                               ? std::string(source.substr(next_line_pos))
                               : std::string(source.substr(
                                     next_line_pos, pos - next_line_pos))});
    }

    return shaders;
}

inline auto ShaderProgram::uniform_location(std::string_view name) -> int {
    if (uniform_cache_.find(name) != uniform_cache_.end()) [[likely]] {
        return uniform_cache_[name];
    } else {
        const int location{glGetUniformLocation(id_, name.data())};
#ifdef RGL_DEBUG
        if (location == -1) {
            std::fprintf(
                stderr,
                RGL_LINEINFO
                ", uniform \"%s\" not found in shader program \"%s\"\n",
                name.data(), m_name.data());
        }
#endif  // RGL_DEBUG

        uniform_cache_[name] = location;
        return location;
    }
}

inline auto ShaderProgram::is_valid(std::uint32_t id) -> bool {
    int link_ok{};
    glGetProgramiv(id, GL_LINK_STATUS, &link_ok);

    if (link_ok == GL_FALSE) [[unlikely]] {
#ifdef RGL_DEBUG
        int max_length{};
        glGetProgramiv(id, GL_INFO_LOG_LENGTH, &max_length);
        std::string error_log(max_length, '\0');
        glGetProgramInfoLog(id, max_length, &max_length, &error_log[0]);
        std::fprintf(stderr,
                     RGL_LINEINFO ", failed to link shader program: %s\n",
                     error_log.data());
#endif  // RGL_DEBUG

        glDeleteProgram(id);
        return false;
    }

    glValidateProgram(id);

    int success = 0;
    glGetProgramiv(id, GL_VALIDATE_STATUS, &success);
    if (!success) [[unlikely]] {
#ifdef RGL_DEBUG
        int max_length{};
        glGetProgramiv(id, GL_INFO_LOG_LENGTH, &max_length);
        std::string error_log(max_length, '\0');
        glGetProgramInfoLog(id, max_length, &max_length, &error_log[0]);
        std::fprintf(stderr,
                     RGL_LINEINFO ", failed to validate shader program: %s\n",
                     error_log.data());
#endif  // RGL_DEBUG
        glDeleteProgram(id);
        return false;
    }

    return true;
}

inline constexpr auto ShaderProgram::to_gl_type(ShaderType shader_type)
    -> std::uint32_t {
    switch (shader_type) {
        case ShaderType::vertex: return GL_VERTEX_SHADER;
        case ShaderType::fragment: return GL_FRAGMENT_SHADER;
        case ShaderType::tess_control: return GL_TESS_CONTROL_SHADER;
        case ShaderType::tess_eval: return GL_TESS_EVALUATION_SHADER;
        case ShaderType::geometry: return GL_GEOMETRY_SHADER;
        default:
#ifdef RGL_DEBUG
            std::fprintf(stderr,
                         RGL_LINEINFO ", invalid shader type enum %d, \n",
                         static_cast<int>(shader_type));
#endif  // RGL_DEBUG
            std::terminate();
    }
}

inline auto ShaderProgram::string_to_shader_type(std::string_view str)
    -> std::optional<ShaderType> {
    static std::unordered_map<std::string_view, ShaderType> map{
        {"vertex", ShaderType::vertex},
        {"fragment", ShaderType::fragment},
        {"tess_control", ShaderType::tess_control},
        {"tess_eval", ShaderType::tess_eval},
        {"geometry", ShaderType::geometry}};

    if (map.find(str) != map.end()) [[likely]] {
        return map[str];
    }

    return std::nullopt;
}
}  // namespace rgl

std::string rgl::shader_type_to_string(ShaderType type) noexcept {
    switch (type) {
        case ShaderType::vertex: return "vertex";
        case ShaderType::fragment: return "fragment";
        case ShaderType::tess_control: return "tess_control";
        case ShaderType::tess_eval: return "tess_eval";
        case ShaderType::geometry: return "geometry";
        default: return "unknown";
    }
}