#pragma once

#include "gl_functions.hpp"
#include "render_buffer.hpp"
#include "texture.hpp"
#include "utility.hpp"

#include <array>
#include <cstdint>
#include <exception>
#include <functional>
#include <optional>
#include <span>

#ifdef RGL_DEBUG
#include <cstdio>
#endif  // RGL_DEBUG

namespace rgl {

/**
 * @brief enum class for framebuffer attachments.
 *
 */
enum class FboAttachment : std::uint8_t {
    NONE = 0x00,
    ATTACH_DEPTH_BUFFER = 0x01,
    ATTACH_STENCIL_BUFFER = 0x02,
    ATTACH_DEPTH_STENCIL_BUFFER = 0x03,
};

/**
 * @brief Framebuffer Object (FBO) wrapper.
 *
 */
class FrameBuffer {
public:
    FrameBuffer() noexcept;
    ~FrameBuffer();

    FrameBuffer(const FrameBuffer&) = delete;
    auto operator=(const FrameBuffer&) -> FrameBuffer& = delete;

    FrameBuffer(FrameBuffer&& other) noexcept;
    auto operator=(FrameBuffer&& other) noexcept -> FrameBuffer&;

    /**
     * @brief Set the renderbuffer object
     *
     * @param res The resolution of the renderbuffer object.
     * @param attachment the type of attachment (Depth buffer, Stencil Buffer,
     * Both, None).
     *
     * @note passing rgl::fbo_attachment::NONE results in the removal of
     * any renderbuffer from the FBO.
     * @note this function destroys any previously held renderbuffer.
     *
     * @see rgl::fbo_attachment
     */
    void set_renderbuffer(
        Resolution res,
        FboAttachment attachment = FboAttachment::ATTACH_DEPTH_STENCIL_BUFFER,
        TexSamples samples = TexSamples::MSAA_X1);

    /**
     * @brief Set a texture as the color attachment of the framebuffer.
     *
     * @details Uses the ID contained in the `rgl::texture_2d` object to
     * attach it to the framebuffer,
     *
     * @note the framebuffer must be bound before calling this function.
     * @warning the framebuffer does not take ownership of the texture, hence
     * care must be taken to ensure that the texture is not destroyed before the
     * framebuffer.
     *
     * @param tex a texture_2d to attach to the framebuffer.
     * @param index the index of the color attachment to use, used as an offset
     * from `GL_COLOR_ATTACHMENT0`.
     */
    void set_texture(Texture2D const& tex, size_t index = 0) const;

    /**
     * @brief Resize the OpenGL viewport.
     *
     * @note this does not resize held textures, but it's faster.
     *
     * @param res The target resolution
     */
    static void set_viewport(Resolution res);

    /**
     * @brief Bind the default framebuffer.
     *
     * @details Calling this function is equivalent to calling ::unbind on any
     * framebuffer object, however, it is semantically more correct to call this
     * function when you want to bind the default framebuffer (even if no
     * framebuffer object exists).
     *
     */
    static void bind_default() { glBindFramebuffer(GL_FRAMEBUFFER, 0); }

    void bind() const;
    static void unbind();

    /**
     * @brief Check if the framebuffer is complete.
     *
     * @details asserts whether a framebuffer is complete, e.g.: ready to be
     * used for rendering. The specifics of what makes a framebuffer complete
     * are well-defined in the OpenGL specification and available in the link
     * below.
     *
     * @warning this function requires the framebuffer to be bound.
     *
     * @see
     * https://www.khronos.org/opengl/wiki/Framebuffer_Object#Framebuffer_Completeness
     *
     * @return true if the framebuffer is complete.
     * @return false otherwise.
     */
    static auto assert_completeness() -> bool {
        auto status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

        return status == GL_FRAMEBUFFER_COMPLETE;
    }

    /**
     * @brief Transfer the contents of a framebuffer to another.
     *
     * @param src The source framebuffer.
     * @param dst The destination framebuffer.
     * @param res The resolution of the framebuffer.
     */
    static void transfer_data(FrameBuffer const& src, FrameBuffer const& dst,
                              Resolution res) {
        glBindFramebuffer(GL_READ_FRAMEBUFFER, src.id());
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, dst.id());

        glBlitFramebuffer(0, 0, res.width, res.height, 0, 0, res.width,
                          res.height, GL_COLOR_BUFFER_BIT, GL_NEAREST);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    /**
     * @brief Get the id of the framebuffer.
     *
     * @return std::uint32_t the id of the framebuffer.
     */
    [[nodiscard]] constexpr auto id() const -> std::uint32_t { return id_; }

    [[nodiscard]] constexpr auto get_renderbuffer() const
        -> const std::optional<RenderBuffer>&;

    [[nodiscard]] constexpr auto attachment() const -> FboAttachment {
        return attachment_;
    }

private:
    std::uint32_t id_{};
    FboAttachment attachment_{};
    std::optional<RenderBuffer> renderbuffer_{};

};  // class framebuffer

/**
 * @brief Construct a new framebuffer::framebuffer object
 *
 * @details Very thin constructor, generates an ID for the framebuffer and
 * stores it.
 */
inline FrameBuffer::FrameBuffer() noexcept { glGenFramebuffers(1, &id_); }

inline FrameBuffer::~FrameBuffer() {
    if (id_ != 0) {
        glDeleteFramebuffers(1, &id_);
    }
}

/**
 * @brief Construct a new framebuffer::framebuffer object
 *
 * @param other
 */
inline FrameBuffer::FrameBuffer(FrameBuffer&& other) noexcept
    : id_(other.id_),
      attachment_(other.attachment_),
      renderbuffer_(std::move(other.renderbuffer_)) {
    other.id_ = 0;
}

/**
 * @brief Move assignment operator.
 *
 * @param other the other framebuffer object to move from.
 * @return framebuffer& the reference to this object.
 */
inline auto FrameBuffer::operator=(FrameBuffer&& other) noexcept
    -> FrameBuffer& {
    if (this != &other) {
        id_ = other.id_;
        attachment_ = other.attachment_;
        renderbuffer_ = std::move(other.renderbuffer_);
        other.id_ = 0;
    }
    return *this;
}

/**
 * @brief Attach a renderbuffer to the framebuffer.
 *
 * @param res the resolution of the renderbuffer.
 * @param attachment the type of the renderbuffer.
 *
 * @warning the framebuffer MUST be bound before calling this function.
 */
inline void FrameBuffer::set_renderbuffer(Resolution res,
                                          FboAttachment attachment,
                                          TexSamples samples) {
    if (attachment != FboAttachment::NONE) {
        RenderBuffer::AttachmentType type{};

        switch (attachment) {
            case FboAttachment::ATTACH_DEPTH_BUFFER:
                type = RenderBuffer::AttachmentType::depth;
                break;
            case FboAttachment::ATTACH_STENCIL_BUFFER:
                type = RenderBuffer::AttachmentType::stencil;
                break;
            case FboAttachment::ATTACH_DEPTH_STENCIL_BUFFER:
                type = RenderBuffer::AttachmentType::depth_stencil;
                break;
            default:
#ifdef RGL_DEBUG
                std::fprintf(stderr,
                             RGL_LINEINFO
                             "invalid attachment enum %d for renderbuffer\n",
                             static_cast<std::uint32_t>(attachment));
#endif
                std::terminate();  // crash and burn, this is unrecoverable
        }

        attachment_ = attachment;  // in some cases this is a redundant
                                   // operation, fair enough.
        renderbuffer_.emplace(res, type, samples);

        glFramebufferRenderbuffer(GL_FRAMEBUFFER,
                                  static_cast<std::uint32_t>(type),
                                  GL_RENDERBUFFER, renderbuffer_->id());
    } else {
        // unbind the old framebuffer, if present

        if (attachment_ == FboAttachment::NONE) {
            return;  // nothing to unbind
        }

        RenderBuffer::AttachmentType type{};

        switch (attachment_) {
            case FboAttachment::ATTACH_DEPTH_BUFFER:
                type = RenderBuffer::AttachmentType::depth;
                break;
            case FboAttachment::ATTACH_STENCIL_BUFFER:
                type = RenderBuffer::AttachmentType::stencil;
                break;
            case FboAttachment::ATTACH_DEPTH_STENCIL_BUFFER:
                type = RenderBuffer::AttachmentType::depth_stencil;
                break;
            default: break;
        }

        attachment_ = FboAttachment::NONE;

        glFramebufferRenderbuffer(GL_FRAMEBUFFER,
                                  static_cast<std::uint32_t>(type),
                                  GL_RENDERBUFFER, 0);
    }
}

inline void FrameBuffer::set_texture(rgl::Texture2D const& tex,
                                     size_t index) const {
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + index,
                           tex.antialias().type, tex.id(), 0);
}

inline void FrameBuffer::set_viewport(rgl::Resolution res) {
    glViewport(0, 0, res.width, res.height);
}

constexpr auto FrameBuffer::get_renderbuffer() const
    -> const std::optional<RenderBuffer>& {
    return renderbuffer_;
}

inline void FrameBuffer::bind() const {
    glBindFramebuffer(GL_FRAMEBUFFER, id_);
}

inline void FrameBuffer::unbind() { glBindFramebuffer(GL_FRAMEBUFFER, 0); }

}  // namespace rgl