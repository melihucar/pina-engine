#pragma once

/// Pina Engine - OpenGL Buffer Implementations

#include "../Buffer.h"
#include "GLCommon.h"

namespace Pina {

/// OpenGL Vertex Buffer
class GLVertexBuffer : public VertexBuffer {
public:
    GLVertexBuffer(const void* data, size_t size);
    ~GLVertexBuffer() override;

    void bind() override;
    void unbind() override;
    void setData(const void* data, size_t size) override;

    uint32_t getID() const override { return m_bufferID; }

private:
    GLuint m_bufferID = 0;
    size_t m_size = 0;
};

/// OpenGL Index Buffer
class GLIndexBuffer : public IndexBuffer {
public:
    GLIndexBuffer(const uint32_t* indices, uint32_t count);
    ~GLIndexBuffer() override;

    void bind() override;
    void unbind() override;

    uint32_t getCount() const override { return m_count; }
    uint32_t getID() const override { return m_bufferID; }

private:
    GLuint m_bufferID = 0;
    uint32_t m_count = 0;
};

/// OpenGL Vertex Array Object
class GLVertexArray : public VertexArray {
public:
    GLVertexArray();
    ~GLVertexArray() override;

    void bind() override;
    void unbind() override;

    void addVertexBuffer(VertexBuffer* buffer, const VertexLayout& layout) override;
    void setIndexBuffer(IndexBuffer* buffer) override;

    IndexBuffer* getIndexBuffer() const override { return m_indexBuffer; }
    uint32_t getID() const override { return m_arrayID; }

private:
    GLuint m_arrayID = 0;
    uint32_t m_attributeIndex = 0;
    IndexBuffer* m_indexBuffer = nullptr;
};

} // namespace Pina
