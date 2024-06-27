#pragma once

#include "gl_functions.hpp"
#include "utility.hpp"

#include <cstdint>
#include <span>

namespace rgl {

/**
 * @brief OpenGL texture details relating to the color format and data type.
 *
 */
struct TextureColor {
    std::int32_t internal_format{};
    std::uint32_t format{};
    std::uint32_t datatype{};
};

/**
 * @brief OpenGL texture details relating to filtering and clamping of the image
 *
 */
struct TextureFilter {
    std::int32_t min_filter{};
    std::int32_t mag_filter{};
    std::int32_t clamping{};
};

/**
 * @brief OpenGL texture details relating to the number of samples to use for
 * the texture.
 *
 * @warning Antialiasing is mainly intended for use in framebuffer attachments,
 * so if you are creating a texture to be used as an object's material, you most
 * likely want to use `rgl::tex_samples::MSAA_X1`.
 */
struct TextureAntialias {
    std::uint32_t type{};
    TexSamples samples{};
};

/**
 * @brief Convert a filter type to its mipmap counterpart.
 *
 * @note An invalid filter type will return 0.
 *
 * @param filter_type the OpenGL enum value of the filter type.
 *
 * @return constexpr std::uint32_t The OpenGL enum value of the mipmap
 * counterpart.
 */
static constexpr auto to_mipmap(std::int32_t filter_type) -> std::int32_t {
    switch (filter_type) {
        case GL_NEAREST:
        case GL_NEAREST_MIPMAP_NEAREST: return GL_NEAREST_MIPMAP_NEAREST;
        case GL_LINEAR:
        case GL_LINEAR_MIPMAP_LINEAR: return GL_LINEAR_MIPMAP_LINEAR;
        default: return 0;
    }
}

/**
 * @brief 2D texture wrapper.
 *
 */
class Texture2D {
public:
    /**
     * @brief Construct a new texture 2d object
     *
     * @details This constructor is only left here to allow users to create
     * containers of texture_2d objects without having to initialize them
     * immediately, every one of those instances should be overwritten before
     * use through a call to `texture_2d::(std::span<const float> data,
     * resolution res, texture_color color, bool generate_mipmap)`.
     *
     * A default-constructed texture_2d object is *still* valid, having an ID of
     * 0, it is however not backed by any OpenGL texture object, therefore it
     * cannot be used in any OpenGL calls (binding it to a framebuffer, for
     * example, would be practically equivalent to unbinding the color
     * attachment for that particular slot).
     *
     * @warning this constructor does not initialize the texture object,
     * remember to perform initialization before using it.
     */
    Texture2D() = default;

    /**
     * @brief Construct a new texture 2d object
     *
     * @param data span of floats that contains the texture data.
     * @param res resolution of the texture.
     * @param color descriptor of the texture's color format and data type.
     * @param filters descriptor of the texture's filtering and clamping.
     * @param samples the number of samples to use for the texture, defaults
     * to 1.
     * @param generate_mipmap whether to generate mipmaps for the texture,
     * defaults to false.
     */
    Texture2D(std::span<const float> data, Resolution res, TextureColor color,
              TextureFilter filters, TexSamples samples = TexSamples::MSAA_X1,
              bool generate_mipmap = false) noexcept;

    /**
     * @brief Destroy the texture 2d object
     *
     */
    ~Texture2D() {
        if (id_ != 0) {
            glDeleteTextures(1, &id_);
        }
    }

    // delete copy and copy assignment operators

    Texture2D(const Texture2D&) = delete;
    auto operator=(const Texture2D&) -> Texture2D& = delete;

    /**
     * @brief Construct a new texture 2d object
     *
     * @param other the other texture_2d object to move from.
     */
    Texture2D(Texture2D&& other) noexcept
        : id_(other.id_),
          unit_(other.unit_),
          color_(other.color_),
          resolution_(other.resolution_),
          antialias_(other.antialias_) {
        other.id_ = 0;
    }

    /**
     * @brief Move assignment operator.
     *
     * @param other the other texture_2d object to move from.
     * @return texture_2d& the reference to this object.
     */
    auto operator=(Texture2D&& other) noexcept -> Texture2D& {
        if (this != &other) {
            id_ = other.id_;
            unit_ = other.unit_;
            color_ = other.color_;
            resolution_ = other.resolution_;
            antialias_ = other.antialias_;
            other.id_ = 0;
        }
        return *this;
    }

    /**
     * @brief Activate the texture on a unit
     *
     * @param unit_offset a texture unit offset from `GL_TEXTURE0`.
     */
    void set_unit(std::uint32_t unit_offset) {
        unit_ = unit_offset;
        glActiveTexture(GL_TEXTURE0 + unit_offset);
        bind();
    }

    /**
     * @brief Set the data object
     *
     * @param data A span of floats that contains the new data.
     * @param res A resolution object containing the new resolution.
     * @param color A texture_color object containing the new color format and
     * data type.
     * @param generate_mipmap Whether to generate mipmaps for the texture,
     * defaults to false.
     */
    void set_data(std::span<const float> data, Resolution res,
                  TextureColor color, bool generate_mipmap = false);

    /**
     * @brief Bind the texture object.
     *
     */
    void bind() const { glBindTexture(antialias_.type, id_); }

    /**
     * @brief Unbind the texture object.
     *
     */
    void unbind() const { glBindTexture(antialias_.type, 0); }
    /**
     * @brief Get the last texture unit this texture was bound to.
     *
     * @return uint32_t a texture unit offset from `GL_TEXTURE0`.
     */
    [[nodiscard]] auto constexpr get_unit() const -> uint32_t { return unit_; }

    /**
     * @brief Get the texture color struct.
     *
     * @return texture_color
     */
    [[nodiscard]] constexpr auto color() const -> TextureColor {
        return color_;
    }

    /**
     * @brief Get the id of the texture object.
     *
     * @return std::uint32_t
     */
    [[nodiscard]] constexpr auto id() const -> std::uint32_t { return id_; }

    /**
     * @brief Get the resolution of the texture object
     *
     * @return rgl::resolution
     */
    [[nodiscard]] constexpr auto get_resolution() const -> rgl::Resolution {
        return resolution_;
    };

    /**
     * @brief Get the texture filter struct.
     *
     * @return texture_filter
     */
    [[nodiscard]] constexpr auto filter() const -> TextureFilter {
        return filter_;
    }

    /**
     * @brief Get the texture antialias struct.
     *
     * @return texture_antialias
     */
    [[nodiscard]] constexpr auto antialias() const -> TextureAntialias {
        return antialias_;
    }

private:
    std::uint32_t id_{};
    std::uint32_t unit_{};
    TextureColor color_{};
    TextureFilter filter_{};
    Resolution resolution_{};
    TextureAntialias antialias_{};
};

inline Texture2D::Texture2D(std::span<const float> data, Resolution res,
                            TextureColor color, TextureFilter filter,
                            TexSamples samples, bool generate_mipmap) noexcept
    : color_{color},
      filter_{filter},
      resolution_{res},
      antialias_{(samples == TexSamples::MSAA_X1)
                     ? TextureAntialias{GL_TEXTURE_2D, samples}
                     : TextureAntialias{GL_TEXTURE_2D_MULTISAMPLE, samples}} {
    glGenTextures(1, &id_);
    glBindTexture(antialias_.type, id_);

    if (antialias_.type == GL_TEXTURE_2D) {
        glTexParameteri(
            antialias_.type, GL_TEXTURE_MIN_FILTER,
            generate_mipmap ? to_mipmap(filter.min_filter) : filter.min_filter);
        glTexParameteri(antialias_.type, GL_TEXTURE_MAG_FILTER,
                        filter.mag_filter);
        glTexParameteri(antialias_.type, GL_TEXTURE_WRAP_S, filter.clamping);
        glTexParameteri(antialias_.type, GL_TEXTURE_WRAP_T, filter.clamping);

        glTexImage2D(antialias_.type, 0, color.internal_format,
                     resolution_.width, resolution_.height, 0, color.format,
                     color.datatype, data.data());
    } else if (antialias_.type == GL_TEXTURE_2D_MULTISAMPLE) {
        glTexImage2DMultisample(antialias_.type, antialias_.samples,
                                color.internal_format, resolution_.width,
                                resolution_.height, GL_TRUE);
    }

    if (generate_mipmap) {
        glGenerateMipmap(antialias_.type);
    }

    glBindTexture(antialias_.type, 0);
}

inline void Texture2D::set_data(std::span<const float> data, Resolution res,
                                TextureColor color, bool generate_mipmap) {
    if (antialias_.type == GL_TEXTURE_2D_MULTISAMPLE) {
        return;
    }

    glTexImage2D(antialias_.type, 0, color_.internal_format, res.width,
                 res.height, 0, color_.format, color_.datatype, data.data());
    color_ = color;
    resolution_ = res;

    if (generate_mipmap) {
        glGenerateMipmap(antialias_.type);
    }
}

}  // namespace rgl