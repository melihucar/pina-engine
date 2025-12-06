#pragma once

/// Pina Engine - OpenGL Graphics Device Implementation

#include "../GraphicsDevice.h"
#include "GLCommon.h"

namespace Pina {

class GLDevice : public GraphicsDevice {
public:
    GLDevice();
    ~GLDevice() override;

    // Resource Creation
    UNIQUE<Shader> createShader() override;
    UNIQUE<VertexBuffer> createVertexBuffer(const void* data, size_t size) override;
    UNIQUE<IndexBuffer> createIndexBuffer(const uint32_t* indices, uint32_t count) override;
    UNIQUE<VertexArray> createVertexArray() override;
    UNIQUE<Texture> createTexture(const unsigned char* data,
                                  uint32_t width,
                                  uint32_t height,
                                  uint32_t channels) override;

    // Frame Lifecycle
    void beginFrame() override;
    void endFrame() override;

    // State Management
    void clear(float r, float g, float b, float a = 1.0f) override;
    void setViewport(int x, int y, int width, int height) override;
    void setDepthTest(bool enabled) override;
    void setBlending(bool enabled) override;
    void setWireframe(bool enabled) override;
    void setDepthWrite(bool enabled) override;

    // Drawing
    void draw(VertexArray* vao, uint32_t vertexCount) override;
    void drawIndexed(VertexArray* vao) override;
};

} // namespace Pina
