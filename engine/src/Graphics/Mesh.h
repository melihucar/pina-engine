#pragma once

/// Pina Engine - Base Mesh Class
/// Abstract base for all mesh types

#include "../Core/Export.h"
#include "../Core/Memory.h"
#include "GraphicsDevice.h"
#include "Buffer.h"

namespace Pina {

/// Base class for all mesh types
/// Provides common functionality for drawing and managing vertex data
class PINA_API Mesh {
public:
    virtual ~Mesh() = default;

    /// Draw the mesh
    /// @note Shader must be bound before calling this method
    void draw();

    /// Get the vertex array object
    VertexArray* getVertexArray() const { return m_vao.get(); }

    /// Get the vertex count
    uint32_t getVertexCount() const { return m_vertexCount; }

    /// Get the graphics device
    GraphicsDevice* getDevice() const { return m_device; }

protected:
    Mesh(GraphicsDevice* device);

    UNIQUE<VertexBuffer> m_vbo;
    UNIQUE<VertexArray> m_vao;
    uint32_t m_vertexCount = 0;
    GraphicsDevice* m_device;  // non-owning reference
};

} // namespace Pina
