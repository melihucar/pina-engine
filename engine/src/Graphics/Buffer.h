#pragma once

/// Pina Engine - Buffer Interfaces
/// Abstract interfaces for GPU buffers

#include "../Core/Export.h"
#include "VertexLayout.h"
#include <cstdint>

namespace Pina {

/// Vertex buffer (VBO)
class PINA_API VertexBuffer {
public:
    virtual ~VertexBuffer() = default;

    virtual void bind() = 0;
    virtual void unbind() = 0;

    /// Update buffer data
    virtual void setData(const void* data, size_t size) = 0;

    /// Get buffer ID (implementation-specific)
    virtual uint32_t getID() const = 0;
};

/// Index buffer (EBO)
class PINA_API IndexBuffer {
public:
    virtual ~IndexBuffer() = default;

    virtual void bind() = 0;
    virtual void unbind() = 0;

    /// Get the number of indices
    virtual uint32_t getCount() const = 0;

    /// Get buffer ID (implementation-specific)
    virtual uint32_t getID() const = 0;
};

/// Vertex array object (VAO)
class PINA_API VertexArray {
public:
    virtual ~VertexArray() = default;

    virtual void bind() = 0;
    virtual void unbind() = 0;

    /// Add a vertex buffer with its layout
    virtual void addVertexBuffer(VertexBuffer* buffer, const VertexLayout& layout) = 0;

    /// Set the index buffer
    virtual void setIndexBuffer(IndexBuffer* buffer) = 0;

    /// Get the associated index buffer (may be nullptr)
    virtual IndexBuffer* getIndexBuffer() const = 0;

    /// Get VAO ID (implementation-specific)
    virtual uint32_t getID() const = 0;
};

} // namespace Pina
