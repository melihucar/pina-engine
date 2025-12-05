/// Pina Engine - Texture Factory Implementation

#include "Texture.h"
#include "GraphicsDevice.h"

// STB Image implementation
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <iostream>

namespace Pina {

UNIQUE<Texture> Texture::load(GraphicsDevice* device, const std::string& path) {
    if (!device) {
        std::cerr << "Texture::load - Invalid graphics device" << std::endl;
        return nullptr;
    }

    // Load image using STB
    int width, height, channels;

    // OpenGL expects textures with origin at bottom-left
    stbi_set_flip_vertically_on_load(true);

    unsigned char* data = stbi_load(path.c_str(), &width, &height, &channels, 0);

    if (!data) {
        std::cerr << "Failed to load texture: " << path << std::endl;
        std::cerr << "STB Error: " << stbi_failure_reason() << std::endl;
        return nullptr;
    }

    // Create texture from data
    auto texture = create(device, data, static_cast<uint32_t>(width),
                          static_cast<uint32_t>(height),
                          static_cast<uint32_t>(channels));

    // Free STB image data
    stbi_image_free(data);

    if (texture) {
        std::cout << "Loaded texture: " << path
                  << " (" << width << "x" << height
                  << ", " << channels << " channels)" << std::endl;
    }

    return texture;
}

UNIQUE<Texture> Texture::loadFromMemory(GraphicsDevice* device,
                                        const unsigned char* data,
                                        uint32_t dataSize) {
    if (!device) {
        std::cerr << "Texture::loadFromMemory - Invalid graphics device" << std::endl;
        return nullptr;
    }

    if (!data || dataSize == 0) {
        std::cerr << "Texture::loadFromMemory - Invalid data" << std::endl;
        return nullptr;
    }

    // Load image from memory using STB
    int width, height, channels;

    // Note: We don't flip vertically for embedded textures in GLB
    // as they are already in the correct orientation
    stbi_set_flip_vertically_on_load(false);

    unsigned char* pixels = stbi_load_from_memory(data, static_cast<int>(dataSize),
                                                   &width, &height, &channels, 0);

    // Restore flip setting for future file loads
    stbi_set_flip_vertically_on_load(true);

    if (!pixels) {
        std::cerr << "Failed to load texture from memory" << std::endl;
        std::cerr << "STB Error: " << stbi_failure_reason() << std::endl;
        return nullptr;
    }

    // Create texture from data
    auto texture = create(device, pixels, static_cast<uint32_t>(width),
                          static_cast<uint32_t>(height),
                          static_cast<uint32_t>(channels));

    // Free STB image data
    stbi_image_free(pixels);

    if (texture) {
        std::cout << "Loaded embedded texture: " << width << "x" << height
                  << " (" << channels << " channels)" << std::endl;
    }

    return texture;
}

UNIQUE<Texture> Texture::create(GraphicsDevice* device,
                                const unsigned char* data,
                                uint32_t width,
                                uint32_t height,
                                uint32_t channels) {
    if (!device) {
        std::cerr << "Texture::create - Invalid graphics device" << std::endl;
        return nullptr;
    }

    if (!data) {
        std::cerr << "Texture::create - Invalid pixel data" << std::endl;
        return nullptr;
    }

    if (width == 0 || height == 0) {
        std::cerr << "Texture::create - Invalid dimensions" << std::endl;
        return nullptr;
    }

    if (channels != 1 && channels != 3 && channels != 4) {
        std::cerr << "Texture::create - Unsupported channel count: " << channels << std::endl;
        return nullptr;
    }

    return device->createTexture(data, width, height, channels);
}

UNIQUE<Texture> Texture::createFromRGBA(GraphicsDevice* device,
                                        const unsigned char* data,
                                        uint32_t width,
                                        uint32_t height) {
    return create(device, data, width, height, 4);
}

} // namespace Pina
