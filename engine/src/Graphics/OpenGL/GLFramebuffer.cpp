/// Pina Engine - OpenGL Framebuffer Implementation

#include "GLFramebuffer.h"
#include <iostream>

namespace Pina {

// ============================================================================
// Constructor / Destructor
// ============================================================================

GLFramebuffer::GLFramebuffer(const FramebufferSpec& spec)
    : m_spec(spec)
{
    invalidate();
}

GLFramebuffer::~GLFramebuffer() {
    cleanup();
}

GLFramebuffer::GLFramebuffer(GLFramebuffer&& other) noexcept
    : m_spec(std::move(other.m_spec))
    , m_framebufferID(other.m_framebufferID)
    , m_colorAttachments(std::move(other.m_colorAttachments))
    , m_depthAttachment(other.m_depthAttachment)
{
    other.m_framebufferID = 0;
    other.m_depthAttachment = 0;
}

GLFramebuffer& GLFramebuffer::operator=(GLFramebuffer&& other) noexcept {
    if (this != &other) {
        cleanup();

        m_spec = std::move(other.m_spec);
        m_framebufferID = other.m_framebufferID;
        m_colorAttachments = std::move(other.m_colorAttachments);
        m_depthAttachment = other.m_depthAttachment;

        other.m_framebufferID = 0;
        other.m_depthAttachment = 0;
    }
    return *this;
}

// ============================================================================
// Binding
// ============================================================================

void GLFramebuffer::bind() {
    glBindFramebuffer(GL_FRAMEBUFFER, m_framebufferID);
    glViewport(0, 0, m_spec.width, m_spec.height);
}

void GLFramebuffer::unbind() {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

// ============================================================================
// Attachments
// ============================================================================

uint32_t GLFramebuffer::getColorAttachmentID(int index) const {
    if (index < 0 || index >= static_cast<int>(m_colorAttachments.size())) {
        return 0;
    }
    return m_colorAttachments[index];
}

// ============================================================================
// Operations
// ============================================================================

void GLFramebuffer::resize(int width, int height) {
    if (width <= 0 || height <= 0) {
        std::cerr << "GLFramebuffer::resize - Invalid dimensions: " << width << "x" << height << std::endl;
        return;
    }

    m_spec.width = width;
    m_spec.height = height;
    invalidate();
}

void GLFramebuffer::clearColor(float r, float g, float b, float a) {
    glBindFramebuffer(GL_FRAMEBUFFER, m_framebufferID);
    glClearColor(r, g, b, a);
    glClear(GL_COLOR_BUFFER_BIT);
}

void GLFramebuffer::clearDepth(float depth) {
    glBindFramebuffer(GL_FRAMEBUFFER, m_framebufferID);
    glClearDepth(depth);
    glClear(GL_DEPTH_BUFFER_BIT);
}

void GLFramebuffer::clear(float r, float g, float b, float a, float depth) {
    glBindFramebuffer(GL_FRAMEBUFFER, m_framebufferID);
    glClearColor(r, g, b, a);
    glClearDepth(depth);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void GLFramebuffer::blitTo(Framebuffer* target, bool blitColor, bool blitDepth) {
    GLuint targetID = 0;
    int targetWidth = m_spec.width;
    int targetHeight = m_spec.height;

    if (target) {
        auto* glTarget = static_cast<GLFramebuffer*>(target);
        targetID = glTarget->m_framebufferID;
        targetWidth = glTarget->m_spec.width;
        targetHeight = glTarget->m_spec.height;
    }

    glBindFramebuffer(GL_READ_FRAMEBUFFER, m_framebufferID);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, targetID);

    GLbitfield mask = 0;
    if (blitColor) mask |= GL_COLOR_BUFFER_BIT;
    if (blitDepth) mask |= GL_DEPTH_BUFFER_BIT;

    glBlitFramebuffer(
        0, 0, m_spec.width, m_spec.height,
        0, 0, targetWidth, targetHeight,
        mask,
        GL_NEAREST
    );

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

// ============================================================================
// Private Methods
// ============================================================================

void GLFramebuffer::invalidate() {
    cleanup();

    glGenFramebuffers(1, &m_framebufferID);
    glBindFramebuffer(GL_FRAMEBUFFER, m_framebufferID);

    // Create color attachments
    m_colorAttachments.resize(m_spec.colorAttachments.size());

    for (size_t i = 0; i < m_spec.colorAttachments.size(); ++i) {
        TextureFormat format = m_spec.colorAttachments[i];
        if (format == TextureFormat::None) continue;

        GLuint& texture = m_colorAttachments[i];
        glGenTextures(1, &texture);

        if (m_spec.samples > 1) {
            // Multisampled texture
            glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, texture);
            glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, m_spec.samples,
                                    toGLInternalFormat(format),
                                    m_spec.width, m_spec.height, GL_TRUE);
            glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + static_cast<GLenum>(i),
                                   GL_TEXTURE_2D_MULTISAMPLE, texture, 0);
        } else {
            // Regular texture
            glBindTexture(GL_TEXTURE_2D, texture);
            glTexImage2D(GL_TEXTURE_2D, 0, toGLInternalFormat(format),
                         m_spec.width, m_spec.height, 0,
                         toGLFormat(format), toGLType(format), nullptr);

            // Set texture parameters (for sampling)
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

            glBindTexture(GL_TEXTURE_2D, 0);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + static_cast<GLenum>(i),
                                   GL_TEXTURE_2D, texture, 0);
        }
    }

    // Set draw buffers for MRT
    if (m_colorAttachments.size() > 1) {
        std::vector<GLenum> buffers(m_colorAttachments.size());
        for (size_t i = 0; i < buffers.size(); ++i) {
            buffers[i] = GL_COLOR_ATTACHMENT0 + static_cast<GLenum>(i);
        }
        glDrawBuffers(static_cast<GLsizei>(buffers.size()), buffers.data());
    } else if (!m_colorAttachments.empty()) {
        GLenum buffer = GL_COLOR_ATTACHMENT0;
        glDrawBuffers(1, &buffer);
    }

    // Create depth attachment
    if (m_spec.depthAttachment != TextureFormat::None) {
        glGenTextures(1, &m_depthAttachment);

        if (m_spec.samples > 1) {
            // Multisampled depth
            glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, m_depthAttachment);
            glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, m_spec.samples,
                                    toGLInternalFormat(m_spec.depthAttachment),
                                    m_spec.width, m_spec.height, GL_TRUE);
            glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);
            glFramebufferTexture2D(GL_FRAMEBUFFER, toGLAttachmentType(m_spec.depthAttachment),
                                   GL_TEXTURE_2D_MULTISAMPLE, m_depthAttachment, 0);
        } else {
            // Regular depth texture
            glBindTexture(GL_TEXTURE_2D, m_depthAttachment);
            glTexImage2D(GL_TEXTURE_2D, 0, toGLInternalFormat(m_spec.depthAttachment),
                         m_spec.width, m_spec.height, 0,
                         toGLFormat(m_spec.depthAttachment), toGLType(m_spec.depthAttachment), nullptr);

            // Set texture parameters (for shadow mapping etc.)
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

            // Border color white (for shadow mapping - outside shadow map = lit)
            float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
            glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

            glBindTexture(GL_TEXTURE_2D, 0);
            glFramebufferTexture2D(GL_FRAMEBUFFER, toGLAttachmentType(m_spec.depthAttachment),
                                   GL_TEXTURE_2D, m_depthAttachment, 0);
        }
    }

    // Check framebuffer completeness
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE) {
        std::cerr << "GLFramebuffer::invalidate - Framebuffer incomplete: ";
        switch (status) {
            case GL_FRAMEBUFFER_UNDEFINED:
                std::cerr << "GL_FRAMEBUFFER_UNDEFINED"; break;
            case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
                std::cerr << "GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT"; break;
            case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
                std::cerr << "GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT"; break;
            case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
                std::cerr << "GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER"; break;
            case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
                std::cerr << "GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER"; break;
            case GL_FRAMEBUFFER_UNSUPPORTED:
                std::cerr << "GL_FRAMEBUFFER_UNSUPPORTED"; break;
            case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:
                std::cerr << "GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE"; break;
            default:
                std::cerr << "Unknown (" << status << ")"; break;
        }
        std::cerr << std::endl;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void GLFramebuffer::cleanup() {
    if (m_framebufferID) {
        glDeleteFramebuffers(1, &m_framebufferID);
        m_framebufferID = 0;
    }

    if (!m_colorAttachments.empty()) {
        glDeleteTextures(static_cast<GLsizei>(m_colorAttachments.size()), m_colorAttachments.data());
        m_colorAttachments.clear();
    }

    if (m_depthAttachment) {
        glDeleteTextures(1, &m_depthAttachment);
        m_depthAttachment = 0;
    }
}

// ============================================================================
// Format Conversion Helpers
// ============================================================================

GLenum GLFramebuffer::toGLFormat(TextureFormat format) {
    switch (format) {
        case TextureFormat::R8:
        case TextureFormat::R16F:
        case TextureFormat::R32F:
            return GL_RED;

        case TextureFormat::RG8:
        case TextureFormat::RG16F:
        case TextureFormat::RG32F:
            return GL_RG;

        case TextureFormat::RGB8:
        case TextureFormat::RGB16F:
        case TextureFormat::RGB32F:
            return GL_RGB;

        case TextureFormat::RGBA8:
        case TextureFormat::RGBA16F:
        case TextureFormat::RGBA32F:
            return GL_RGBA;

        case TextureFormat::Depth16:
        case TextureFormat::Depth24:
        case TextureFormat::Depth32F:
            return GL_DEPTH_COMPONENT;

        case TextureFormat::Depth24Stencil8:
            return GL_DEPTH_STENCIL;

        default:
            return GL_RGBA;
    }
}

GLenum GLFramebuffer::toGLInternalFormat(TextureFormat format) {
    switch (format) {
        case TextureFormat::R8:             return GL_R8;
        case TextureFormat::RG8:            return GL_RG8;
        case TextureFormat::RGB8:           return GL_RGB8;
        case TextureFormat::RGBA8:          return GL_RGBA8;
        case TextureFormat::R16F:           return GL_R16F;
        case TextureFormat::RG16F:          return GL_RG16F;
        case TextureFormat::RGB16F:         return GL_RGB16F;
        case TextureFormat::RGBA16F:        return GL_RGBA16F;
        case TextureFormat::R32F:           return GL_R32F;
        case TextureFormat::RG32F:          return GL_RG32F;
        case TextureFormat::RGB32F:         return GL_RGB32F;
        case TextureFormat::RGBA32F:        return GL_RGBA32F;
        case TextureFormat::Depth16:        return GL_DEPTH_COMPONENT16;
        case TextureFormat::Depth24:        return GL_DEPTH_COMPONENT24;
        case TextureFormat::Depth32F:       return GL_DEPTH_COMPONENT32F;
        case TextureFormat::Depth24Stencil8: return GL_DEPTH24_STENCIL8;
        default:                            return GL_RGBA8;
    }
}

GLenum GLFramebuffer::toGLType(TextureFormat format) {
    switch (format) {
        case TextureFormat::R8:
        case TextureFormat::RG8:
        case TextureFormat::RGB8:
        case TextureFormat::RGBA8:
            return GL_UNSIGNED_BYTE;

        case TextureFormat::R16F:
        case TextureFormat::RG16F:
        case TextureFormat::RGB16F:
        case TextureFormat::RGBA16F:
        case TextureFormat::R32F:
        case TextureFormat::RG32F:
        case TextureFormat::RGB32F:
        case TextureFormat::RGBA32F:
        case TextureFormat::Depth32F:
            return GL_FLOAT;

        case TextureFormat::Depth16:
            return GL_UNSIGNED_SHORT;

        case TextureFormat::Depth24:
            return GL_UNSIGNED_INT;

        case TextureFormat::Depth24Stencil8:
            return GL_UNSIGNED_INT_24_8;

        default:
            return GL_UNSIGNED_BYTE;
    }
}

GLenum GLFramebuffer::toGLAttachmentType(TextureFormat format) {
    switch (format) {
        case TextureFormat::Depth16:
        case TextureFormat::Depth24:
        case TextureFormat::Depth32F:
            return GL_DEPTH_ATTACHMENT;

        case TextureFormat::Depth24Stencil8:
            return GL_DEPTH_STENCIL_ATTACHMENT;

        default:
            return GL_COLOR_ATTACHMENT0;
    }
}

} // namespace Pina
