#pragma once

/// Pina Engine - Texture Interface
/// Abstract interface for 2D textures

#include "../Core/Export.h"
#include "../Core/Memory.h"
#include <string>
#include <cstdint>

namespace Pina {

// Forward declaration
class GraphicsDevice;

/// Texture filtering modes
enum class PINA_API TextureFilter {
    Nearest,           // No filtering (pixelated)
    Linear,            // Bilinear filtering (smooth)
    NearestMipmap,     // Nearest with mipmaps
    LinearMipmap       // Trilinear filtering (best quality)
};

/// Texture wrapping modes
enum class PINA_API TextureWrap {
    Repeat,            // Tile the texture
    MirroredRepeat,    // Tile with mirroring
    ClampToEdge,       // Clamp to edge pixels
    ClampToBorder      // Use border color
};

/// Abstract texture interface
class PINA_API Texture {
public:
    virtual ~Texture() = default;

    // ========================================================================
    // Binding
    // ========================================================================

    /// Bind texture to a texture unit
    /// @param slot Texture unit (0-15 typically)
    virtual void bind(uint32_t slot = 0) = 0;

    /// Unbind texture from its current slot
    virtual void unbind() = 0;

    // ========================================================================
    // Properties
    // ========================================================================

    /// Get texture width in pixels
    virtual uint32_t getWidth() const = 0;

    /// Get texture height in pixels
    virtual uint32_t getHeight() const = 0;

    /// Get number of color channels (3=RGB, 4=RGBA)
    virtual uint32_t getChannels() const = 0;

    /// Get implementation-specific texture ID
    virtual uint32_t getID() const = 0;

    // ========================================================================
    // Factory Methods
    // ========================================================================

    /// Load texture from file using STB image
    /// Supports: PNG, JPG, TGA, BMP, PSD, GIF, HDR, PIC
    /// @param device Graphics device to create texture on
    /// @param path Path to image file
    /// @return Loaded texture, or nullptr on failure
    static UNIQUE<Texture> load(GraphicsDevice* device, const std::string& path);

    /// Load texture from compressed image data in memory (PNG, JPG, etc.)
    /// Used for embedded textures in GLB files
    /// @param device Graphics device to create texture on
    /// @param data Compressed image data (PNG, JPG, etc.)
    /// @param dataSize Size of the compressed data in bytes
    /// @return Loaded texture, or nullptr on failure
    static UNIQUE<Texture> loadFromMemory(GraphicsDevice* device,
                                          const unsigned char* data,
                                          uint32_t dataSize);

    /// Create texture from raw pixel data
    /// @param device Graphics device to create texture on
    /// @param data Raw pixel data (RGB or RGBA)
    /// @param width Image width in pixels
    /// @param height Image height in pixels
    /// @param channels Number of channels (3 or 4)
    /// @return Created texture, or nullptr on failure
    static UNIQUE<Texture> create(GraphicsDevice* device,
                                  const unsigned char* data,
                                  uint32_t width,
                                  uint32_t height,
                                  uint32_t channels);

    /// Create texture from RGBA pixel data (convenience alias)
    /// @param device Graphics device to create texture on
    /// @param data Raw RGBA pixel data (4 bytes per pixel)
    /// @param width Image width in pixels
    /// @param height Image height in pixels
    /// @return Created texture, or nullptr on failure
    static UNIQUE<Texture> createFromRGBA(GraphicsDevice* device,
                                          const unsigned char* data,
                                          uint32_t width,
                                          uint32_t height);
};

} // namespace Pina
