#pragma once

/// Pina Engine - Framebuffer Interface
/// Abstract interface for render targets (FBOs)

#include "../Core/Export.h"
#include "../Core/Memory.h"
#include <vector>
#include <cstdint>

namespace Pina {

// Forward declarations
class GraphicsDevice;
class Texture;

/// Texture format for framebuffer attachments
enum class PINA_API TextureFormat {
    // Color formats
    R8,                     // 8-bit red
    RG8,                    // 8-bit red-green
    RGB8,                   // 8-bit RGB
    RGBA8,                  // 8-bit RGBA (default)
    R16F,                   // 16-bit float red
    RG16F,                  // 16-bit float red-green
    RGB16F,                 // 16-bit float RGB (HDR)
    RGBA16F,                // 16-bit float RGBA (HDR)
    R32F,                   // 32-bit float red
    RG32F,                  // 32-bit float red-green
    RGB32F,                 // 32-bit float RGB
    RGBA32F,                // 32-bit float RGBA

    // Depth formats
    Depth16,                // 16-bit depth
    Depth24,                // 24-bit depth
    Depth32F,               // 32-bit float depth

    // Depth-stencil formats
    Depth24Stencil8,        // 24-bit depth + 8-bit stencil (default depth)

    // Special
    None                    // No attachment
};

/// Framebuffer specification
struct PINA_API FramebufferSpec {
    int width = 1280;
    int height = 720;

    // Color attachments (can have multiple for MRT)
    std::vector<TextureFormat> colorAttachments = { TextureFormat::RGBA8 };

    // Depth attachment
    TextureFormat depthAttachment = TextureFormat::Depth24Stencil8;

    // MSAA samples (1 = no MSAA)
    int samples = 1;

    // Is this the swap chain target (screen)?
    bool swapChainTarget = false;
};

/// Abstract framebuffer interface (render target)
class PINA_API Framebuffer {
public:
    virtual ~Framebuffer() = default;

    // ========================================================================
    // Binding
    // ========================================================================

    /// Bind this framebuffer as render target
    virtual void bind() = 0;

    /// Unbind and restore default framebuffer
    virtual void unbind() = 0;

    // ========================================================================
    // Properties
    // ========================================================================

    /// Get framebuffer width
    virtual int getWidth() const = 0;

    /// Get framebuffer height
    virtual int getHeight() const = 0;

    /// Get the specification used to create this framebuffer
    virtual const FramebufferSpec& getSpec() const = 0;

    // ========================================================================
    // Attachments
    // ========================================================================

    /// Get color attachment texture ID (for binding as input texture)
    /// @param index Color attachment index (0 by default)
    virtual uint32_t getColorAttachmentID(int index = 0) const = 0;

    /// Get depth attachment texture ID
    virtual uint32_t getDepthAttachmentID() const = 0;

    /// Get number of color attachments
    virtual int getColorAttachmentCount() const = 0;

    // ========================================================================
    // Operations
    // ========================================================================

    /// Resize the framebuffer (recreates attachments)
    virtual void resize(int width, int height) = 0;

    /// Clear color attachment with specified color
    virtual void clearColor(float r, float g, float b, float a = 1.0f) = 0;

    /// Clear depth attachment
    virtual void clearDepth(float depth = 1.0f) = 0;

    /// Clear both color and depth
    virtual void clear(float r, float g, float b, float a = 1.0f, float depth = 1.0f) = 0;

    /// Blit (copy) this framebuffer to another
    /// @param target Target framebuffer (nullptr = default framebuffer/screen)
    /// @param blitColor Copy color attachments
    /// @param blitDepth Copy depth attachment
    virtual void blitTo(Framebuffer* target, bool blitColor = true, bool blitDepth = false) = 0;

    // ========================================================================
    // Factory
    // ========================================================================

    /// Create a framebuffer with the given specification
    static UNIQUE<Framebuffer> create(GraphicsDevice* device, const FramebufferSpec& spec);
};

/// Check if format is a depth format
inline bool isDepthFormat(TextureFormat format) {
    return format == TextureFormat::Depth16 ||
           format == TextureFormat::Depth24 ||
           format == TextureFormat::Depth32F ||
           format == TextureFormat::Depth24Stencil8;
}

/// Check if format has stencil component
inline bool hasStencil(TextureFormat format) {
    return format == TextureFormat::Depth24Stencil8;
}

/// Check if format is HDR (floating point)
inline bool isHDRFormat(TextureFormat format) {
    return format == TextureFormat::R16F ||
           format == TextureFormat::RG16F ||
           format == TextureFormat::RGB16F ||
           format == TextureFormat::RGBA16F ||
           format == TextureFormat::R32F ||
           format == TextureFormat::RG32F ||
           format == TextureFormat::RGB32F ||
           format == TextureFormat::RGBA32F;
}

} // namespace Pina
