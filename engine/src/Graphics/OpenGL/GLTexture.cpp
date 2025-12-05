/// Pina Engine - OpenGL Texture Implementation

#include "GLTexture.h"
#include <iostream>

namespace Pina {

GLTexture::GLTexture(const unsigned char* data, uint32_t width, uint32_t height, uint32_t channels)
    : m_width(width), m_height(height), m_channels(channels) {

    glGenTextures(1, &m_textureID);
    glBindTexture(GL_TEXTURE_2D, m_textureID);

    // Default wrapping mode
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    // Default filtering with mipmaps
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Determine format based on channels
    GLenum internalFormat = GL_RGB;
    GLenum dataFormat = GL_RGB;

    if (channels == 4) {
        internalFormat = GL_RGBA;
        dataFormat = GL_RGBA;
    } else if (channels == 1) {
        internalFormat = GL_RED;
        dataFormat = GL_RED;
    }

    // Upload texture data
    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0,
                 dataFormat, GL_UNSIGNED_BYTE, data);

    // Generate mipmaps
    glGenerateMipmap(GL_TEXTURE_2D);

    // Unbind
    glBindTexture(GL_TEXTURE_2D, 0);

    GL_CHECK_ERROR();
}

GLTexture::~GLTexture() {
    if (m_textureID != 0) {
        glDeleteTextures(1, &m_textureID);
        m_textureID = 0;
    }
}

GLTexture::GLTexture(GLTexture&& other) noexcept
    : m_textureID(other.m_textureID)
    , m_width(other.m_width)
    , m_height(other.m_height)
    , m_channels(other.m_channels)
    , m_boundSlot(other.m_boundSlot) {
    other.m_textureID = 0;
}

GLTexture& GLTexture::operator=(GLTexture&& other) noexcept {
    if (this != &other) {
        // Clean up current texture
        if (m_textureID != 0) {
            glDeleteTextures(1, &m_textureID);
        }

        // Move from other
        m_textureID = other.m_textureID;
        m_width = other.m_width;
        m_height = other.m_height;
        m_channels = other.m_channels;
        m_boundSlot = other.m_boundSlot;

        other.m_textureID = 0;
    }
    return *this;
}

void GLTexture::bind(uint32_t slot) {
    m_boundSlot = slot;
    glActiveTexture(GL_TEXTURE0 + slot);
    glBindTexture(GL_TEXTURE_2D, m_textureID);
}

void GLTexture::unbind() {
    glActiveTexture(GL_TEXTURE0 + m_boundSlot);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void GLTexture::setFilter(TextureFilter minFilter, TextureFilter magFilter) {
    glBindTexture(GL_TEXTURE_2D, m_textureID);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, toGLFilter(minFilter, true));
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, toGLFilter(magFilter, false));
    glBindTexture(GL_TEXTURE_2D, 0);
}

void GLTexture::setWrap(TextureWrap wrapS, TextureWrap wrapT) {
    glBindTexture(GL_TEXTURE_2D, m_textureID);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, toGLWrap(wrapS));
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, toGLWrap(wrapT));
    glBindTexture(GL_TEXTURE_2D, 0);
}

GLenum GLTexture::toGLFilter(TextureFilter filter, bool minFilter) {
    switch (filter) {
        case TextureFilter::Nearest:
            return GL_NEAREST;
        case TextureFilter::Linear:
            return GL_LINEAR;
        case TextureFilter::NearestMipmap:
            return minFilter ? GL_NEAREST_MIPMAP_NEAREST : GL_NEAREST;
        case TextureFilter::LinearMipmap:
            return minFilter ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR;
        default:
            return GL_LINEAR;
    }
}

GLenum GLTexture::toGLWrap(TextureWrap wrap) {
    switch (wrap) {
        case TextureWrap::Repeat:
            return GL_REPEAT;
        case TextureWrap::MirroredRepeat:
            return GL_MIRRORED_REPEAT;
        case TextureWrap::ClampToEdge:
            return GL_CLAMP_TO_EDGE;
        case TextureWrap::ClampToBorder:
            // Note: GL_CLAMP_TO_BORDER not available in OpenGL ES / some contexts
            // Fall back to clamp to edge
            return GL_CLAMP_TO_EDGE;
        default:
            return GL_REPEAT;
    }
}

} // namespace Pina
