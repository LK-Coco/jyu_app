#pragma once

#include "gl_functions.hpp"
#include "texture.hpp"
#include "utility.hpp"

#include <cstddef>
#include <cstdint>
#include <span>

namespace rgl {

/**
 * @brief Cube map texture wrapper.
 *
 */
class CubeMap {
public:
    /**
     * @brief Construct a new cubemap object
     *
     * @param data The cubemap data, six faces of the cube.
     * @param res The resolution of the cubemap.
     * @param color The color format of the cubemap.
     * @param filter The filtering algorithms to use for the cubemap.
     * @param generate_mipmaps Whether to generate mipmaps for the cubemap,
     * defaults to false.
     */
    CubeMap(std::span<std::span<std::byte>, 6> data, Resolution res,
            TextureColor color, TextureFilter filter,
            bool generate_mipmaps = false) noexcept;

    /**
     * @brief Destroy the cubemap object
     *
     */
    ~CubeMap() noexcept {
        if (id_ != 0) {
            glDeleteTextures(1, &id_);
        }
    }

    // delete copy and copy assignment operators

    CubeMap(const CubeMap&) = delete;
    auto operator=(const CubeMap&) -> CubeMap& = delete;

    /**
     * @brief Construct a new cubemap object from another cubemap object.
     *
     * @param other the other cubemap object to move from.
     */
    CubeMap(CubeMap&& other) noexcept
        : id_(other.id_),
          res_(other.res_),
          color_(other.color_),
          filter_(other.filter_) {
        other.id_ = 0;
    }

    /**
     * @brief Move assignment operator.
     *
     * @param other the other cubemap object to move from.
     * @return cubemap& a reference to this object.
     */
    auto operator=(CubeMap&& other) noexcept -> CubeMap& {
        if (this != &other) {
            id_ = other.id_;
            color_ = other.color_;
            res_ = other.res_;
            filter_ = other.filter_;
            other.id_ = 0;
        }
        return *this;
    }

    /**
     * @brief Bind the cubemap texture.
     *
     */
    void bind() const;

    /**
     * @brief Unbind the cubemap texture.
     *
     */
    static void unbind();

    /**
     * @brief Get the texture id.
     *
     * @return uint32_t the texture id.
     */
    [[nodiscard]] constexpr auto id() const noexcept -> uint32_t { return id_; }

    /**
     * @brief Get the texture color.
     *
     * @return texture_color the texture color.
     */
    [[nodiscard]] constexpr auto color() const noexcept -> TextureColor {
        return color_;
    }

    /**
     * @brief Get the resolution of the cubemap.
     *
     * @return resolution the resolution of the cubemap.
     */
    [[nodiscard]] constexpr auto res() const noexcept -> Resolution {
        return res_;
    }

    /**
     * @brief Set the unit object to bind the texture to.
     *
     * @param unit_offset an offset that specifies the unit on which to bind the
     * texture.
     */
    void set_unit(std::uint32_t unit_offset) const;

private:
    uint32_t id_{};
    Resolution res_{};
    TextureColor color_{};
    TextureFilter filter_{};
};

inline CubeMap::CubeMap(std::span<std::span<std::byte>, 6> data, Resolution res,
                        TextureColor color, TextureFilter filter,
                        bool generate_mipmaps) noexcept
    : res_(res),
      color_(color),
      filter_(filter) {
    glGenTextures(1, &id_);
    glBindTexture(GL_TEXTURE_CUBE_MAP, id_);

    glTextureParameteri(id_, GL_TEXTURE_WRAP_S, filter.clamping);
    glTextureParameteri(id_, GL_TEXTURE_WRAP_T, filter.clamping);
    glTextureParameteri(id_, GL_TEXTURE_WRAP_R, filter.clamping);

    glTextureParameteri(
        id_, GL_TEXTURE_MIN_FILTER,
        generate_mipmaps ? to_mipmap(filter.min_filter) : filter.min_filter);
    glTextureParameteri(id_, GL_TEXTURE_MAG_FILTER, filter.mag_filter);

    int i = 0;
    for (auto const& face : data) {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i++, 0,
                     color_.internal_format, res_.width, res_.height, 0,
                     color_.format, color_.datatype, face.data());
    }

    if (generate_mipmaps) {
        glGenerateTextureMipmap(id_);
    }
};

inline void CubeMap::bind() const { glBindTexture(GL_TEXTURE_CUBE_MAP, id_); }

inline void CubeMap::unbind() { glBindTexture(GL_TEXTURE_CUBE_MAP, 0); }

inline void CubeMap::set_unit(std::uint32_t unit_offset) const {
    glActiveTexture(GL_TEXTURE0 + unit_offset);
    glBindTexture(GL_TEXTURE_CUBE_MAP, id_);
}

}  // namespace rgl