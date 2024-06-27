#pragma once

#include "gl_functions.hpp"
#include "utility.hpp"

#include <cstdint>

namespace rgl {

/**
 * @brief Render Buffer Object (RBO) wrapper.
 *
 */
class RenderBuffer {
public:
    /**
     * @brief Renderbuffer attachment type.
     *
     * @details This enum is used to specify the type of attachment that the
     * renderbuffer will be used for. <br>
     *
     */
    enum class AttachmentType : uint32_t {
        depth = GL_DEPTH_ATTACHMENT,
        stencil = GL_STENCIL_ATTACHMENT,
        depth_stencil = GL_DEPTH_STENCIL_ATTACHMENT
    };

    /**
     * @brief Construct a new renderbuffer object with the specified resolution,
     * attachment type and sample count.
     *
     * @param res Resolution of the renderbuffer.
     * @param type Attachment type of the renderbuffer.
     * @param samples Sample count of the renderbuffer, defaults to 1.
     *
     */
    RenderBuffer(Resolution res,
                 AttachmentType type = AttachmentType::depth_stencil,
                 TexSamples samples = TexSamples::MSAA_X1) noexcept;

    /**
     * @brief Destroy the renderbuffer object
     *
     */
    ~RenderBuffer();

    RenderBuffer(const RenderBuffer&) = delete;
    auto operator=(const RenderBuffer&) -> RenderBuffer& = delete;

    /**
     * @brief Construct a new renderbuffer object
     *
     * @param other The renderbuffer object to move from.
     *
     * @note Moving from a renderbuffer is cheap since it only involves copying
     * the OpenGL ID, the `other` object will be left with an ID of 0.
     *
     */
    RenderBuffer(RenderBuffer&& other) noexcept;

    /**
     * @brief Move assignment operator.
     *
     * @param other The renderbuffer object to move from.
     * @return renderbuffer& The moved object.
     */
    auto operator=(RenderBuffer&& other) noexcept -> RenderBuffer&;

    /**
     * @brief Bind the renderbuffer object.
     *
     */
    void bind() const;

    /**
     * @brief Unbind the renderbuffer object.
     *
     */
    void unbind() const;

    /**
     * @brief Get the ID of the renderbuffer object.
     *
     * @return uint32_t the ID of the renderbuffer object.
     */
    [[nodiscard]] constexpr auto id() const noexcept -> uint32_t { return id_; }

    /**
     * @brief Get the resolution of the renderbuffer object.
     *
     * @return resolution A resolution object containing the width and height of
     * the renderbuffer.
     */
    [[nodiscard]] constexpr auto res() const noexcept -> Resolution {
        return res_;
    }

    /**
     * @brief Get the attachment type of the renderbuffer object.
     *
     *
     * @return attachment_type The attachment type of the renderbuffer object.
     */
    [[nodiscard]] constexpr auto type() const noexcept -> AttachmentType {
        return type_;
    }

    /**
     * @brief Get the sample count of the renderbuffer object.
     *
     * @return tex_samples The sample count of the renderbuffer object.
     */
    [[nodiscard]] constexpr auto samples() const noexcept -> TexSamples {
        return samples_;
    }

private:
    uint32_t id_{};
    Resolution res_{};
    AttachmentType type_{};
    TexSamples samples_{};
};

inline RenderBuffer::RenderBuffer(Resolution res, AttachmentType type,
                                  TexSamples samples) noexcept
    : res_(res),
      type_(type),
      samples_(samples) {
    int32_t internal_format{};

    switch (type_) {
        case AttachmentType::depth:
            internal_format = GL_DEPTH_COMPONENT24;
            break;
        case AttachmentType::stencil:
            internal_format = GL_STENCIL_INDEX8;
            break;
        case AttachmentType::depth_stencil:
            internal_format = GL_DEPTH24_STENCIL8;
            break;
    }

    glGenRenderbuffers(1, &id_);
    glBindRenderbuffer(GL_RENDERBUFFER, id_);
    if (samples_ != TexSamples::MSAA_X1) {
        glRenderbufferStorageMultisample(
            GL_RENDERBUFFER, static_cast<int32_t>(samples_), internal_format,
            res.width, res.height);
    } else {
        glRenderbufferStorage(GL_RENDERBUFFER, internal_format, res.width,
                              res.height);
    }
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
}

inline RenderBuffer::~RenderBuffer() {
    if (id_ != 0) glDeleteRenderbuffers(1, &id_);
}

inline RenderBuffer::RenderBuffer(RenderBuffer&& other) noexcept
    : id_(other.id_),
      res_(other.res_),
      type_(other.type_),
      samples_(other.samples_) {
    other.id_ = 0;
}

inline auto RenderBuffer::operator=(RenderBuffer&& other) noexcept
    -> RenderBuffer& {
    if (this != &other) {
        id_ = other.id_;
        res_ = other.res_;
        type_ = other.type_;
        samples_ = other.samples_;

        other.id_ = 0;
    }

    return *this;
}

inline void RenderBuffer::bind() const {
    glBindRenderbuffer(GL_RENDERBUFFER, id_);
}

inline void RenderBuffer::unbind() const {
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
}

}  // namespace rgl