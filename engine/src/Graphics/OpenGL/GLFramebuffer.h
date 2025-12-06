#pragma once

/// Pina Engine - OpenGL Framebuffer Implementation

#include "../Framebuffer.h"
#include "GLCommon.h"
#include <vector>

namespace Pina {

/// OpenGL framebuffer implementation
class GLFramebuffer : public Framebuffer {
public:
    explicit GLFramebuffer(const FramebufferSpec& spec);
    ~GLFramebuffer() override;

    // No copying
    GLFramebuffer(const GLFramebuffer&) = delete;
    GLFramebuffer& operator=(const GLFramebuffer&) = delete;

    // Move support
    GLFramebuffer(GLFramebuffer&& other) noexcept;
    GLFramebuffer& operator=(GLFramebuffer&& other) noexcept;

    // ========================================================================
    // Binding
    // ========================================================================

    void bind() override;
    void unbind() override;

    // ========================================================================
    // Properties
    // ========================================================================

    int getWidth() const override { return m_spec.width; }
    int getHeight() const override { return m_spec.height; }
    const FramebufferSpec& getSpec() const override { return m_spec; }

    // ========================================================================
    // Attachments
    // ========================================================================

    uint32_t getColorAttachmentID(int index = 0) const override;
    uint32_t getDepthAttachmentID() const override { return m_depthAttachment; }
    int getColorAttachmentCount() const override { return static_cast<int>(m_colorAttachments.size()); }

    // ========================================================================
    // Operations
    // ========================================================================

    void resize(int width, int height) override;
    void clearColor(float r, float g, float b, float a = 1.0f) override;
    void clearDepth(float depth = 1.0f) override;
    void clear(float r, float g, float b, float a = 1.0f, float depth = 1.0f) override;
    void blitTo(Framebuffer* target, bool blitColor = true, bool blitDepth = false) override;

private:
    void invalidate();  // Recreate framebuffer
    void cleanup();     // Delete GL resources

    static GLenum toGLFormat(TextureFormat format);
    static GLenum toGLInternalFormat(TextureFormat format);
    static GLenum toGLType(TextureFormat format);
    static GLenum toGLAttachmentType(TextureFormat format);

    FramebufferSpec m_spec;
    GLuint m_framebufferID = 0;
    std::vector<GLuint> m_colorAttachments;
    GLuint m_depthAttachment = 0;
};

} // namespace Pina
