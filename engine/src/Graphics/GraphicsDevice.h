#pragma once

/// Pina Engine - Graphics Device Interface
/// Central factory for creating graphics resources

#include "../Core/Export.h"
#include "../Core/Memory.h"
#include "../Platform/Graphics.h"
#include "Shader.h"
#include "Buffer.h"
#include "VertexLayout.h"

namespace Pina {

/// Abstract graphics device interface
/// All graphics resources are created through this device
class PINA_API GraphicsDevice {
public:
    virtual ~GraphicsDevice() = default;

    // ========================================================================
    // Resource Creation
    // ========================================================================

    /// Create a shader program
    virtual UNIQUE<Shader> createShader() = 0;

    /// Create a vertex buffer
    virtual UNIQUE<VertexBuffer> createVertexBuffer(const void* data, size_t size) = 0;

    /// Create an index buffer
    virtual UNIQUE<IndexBuffer> createIndexBuffer(const uint32_t* indices, uint32_t count) = 0;

    /// Create a vertex array object
    virtual UNIQUE<VertexArray> createVertexArray() = 0;

    // ========================================================================
    // Frame Lifecycle
    // ========================================================================

    /// Begin a new frame
    virtual void beginFrame() = 0;

    /// End the current frame
    virtual void endFrame() = 0;

    // ========================================================================
    // State Management
    // ========================================================================

    /// Clear the framebuffer
    virtual void clear(float r, float g, float b, float a = 1.0f) = 0;

    /// Set the viewport
    virtual void setViewport(int x, int y, int width, int height) = 0;

    /// Enable/disable depth testing
    virtual void setDepthTest(bool enabled) = 0;

    /// Enable/disable blending
    virtual void setBlending(bool enabled) = 0;

    // ========================================================================
    // Drawing
    // ========================================================================

    /// Draw vertices (non-indexed)
    /// @note Shader must be bound before calling this method
    virtual void draw(VertexArray* vao, uint32_t vertexCount) = 0;

    /// Draw indexed vertices
    /// @note Shader must be bound before calling this method
    virtual void drawIndexed(VertexArray* vao) = 0;

    // ========================================================================
    // Factory
    // ========================================================================

    /// Create a graphics device for the specified backend
    static UNIQUE<GraphicsDevice> create(GraphicsBackend backend);
};

} // namespace Pina
