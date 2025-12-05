#pragma once

/// Pina Engine - OpenGL Texture Implementation

#include "../Texture.h"
#include "GLCommon.h"

namespace Pina {

/// OpenGL texture implementation
class GLTexture : public Texture {
public:
    /// Create texture from raw pixel data
    /// @param data RGB or RGBA pixel data
    /// @param width Image width in pixels
    /// @param height Image height in pixels
    /// @param channels Number of channels (3=RGB, 4=RGBA)
    GLTexture(const unsigned char* data, uint32_t width, uint32_t height, uint32_t channels);
    ~GLTexture() override;

    // No copying
    GLTexture(const GLTexture&) = delete;
    GLTexture& operator=(const GLTexture&) = delete;

    // Move support
    GLTexture(GLTexture&& other) noexcept;
    GLTexture& operator=(GLTexture&& other) noexcept;

    // Binding
    void bind(uint32_t slot = 0) override;
    void unbind() override;

    // Properties
    uint32_t getWidth() const override { return m_width; }
    uint32_t getHeight() const override { return m_height; }
    uint32_t getChannels() const override { return m_channels; }
    uint32_t getID() const override { return m_textureID; }

    // Texture settings
    void setFilter(TextureFilter minFilter, TextureFilter magFilter);
    void setWrap(TextureWrap wrapS, TextureWrap wrapT);

private:
    GLuint m_textureID = 0;
    uint32_t m_width = 0;
    uint32_t m_height = 0;
    uint32_t m_channels = 0;
    uint32_t m_boundSlot = 0;

    static GLenum toGLFilter(TextureFilter filter, bool minFilter);
    static GLenum toGLWrap(TextureWrap wrap);
};

} // namespace Pina
