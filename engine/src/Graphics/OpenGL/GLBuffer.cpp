/// Pina Engine - OpenGL Buffer Implementations

#include "GLBuffer.h"

namespace Pina {

// ============================================================================
// GLVertexBuffer
// ============================================================================

GLVertexBuffer::GLVertexBuffer(const void* data, size_t size)
    : m_size(size)
{
    glGenBuffers(1, &m_bufferID);
    glBindBuffer(GL_ARRAY_BUFFER, m_bufferID);
    glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);
}

GLVertexBuffer::~GLVertexBuffer() {
    glDeleteBuffers(1, &m_bufferID);
}

void GLVertexBuffer::bind() {
    glBindBuffer(GL_ARRAY_BUFFER, m_bufferID);
}

void GLVertexBuffer::unbind() {
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void GLVertexBuffer::setData(const void* data, size_t size) {
    glBindBuffer(GL_ARRAY_BUFFER, m_bufferID);
    glBufferData(GL_ARRAY_BUFFER, size, data, GL_DYNAMIC_DRAW);
    m_size = size;
}

// ============================================================================
// GLIndexBuffer
// ============================================================================

GLIndexBuffer::GLIndexBuffer(const uint32_t* indices, uint32_t count)
    : m_count(count)
{
    // Save currently bound VAO to restore later
    GLint previousVAO = 0;
    glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &previousVAO);

    // Unbind VAO to prevent contaminating its GL_ELEMENT_ARRAY_BUFFER binding
    // (GL_ELEMENT_ARRAY_BUFFER binding is stored as part of VAO state)
    glBindVertexArray(0);

    glGenBuffers(1, &m_bufferID);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_bufferID);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * sizeof(uint32_t), indices, GL_STATIC_DRAW);

    // Restore previous VAO
    glBindVertexArray(previousVAO);
}

GLIndexBuffer::~GLIndexBuffer() {
    glDeleteBuffers(1, &m_bufferID);
}

void GLIndexBuffer::bind() {
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_bufferID);
}

void GLIndexBuffer::unbind() {
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

// ============================================================================
// GLVertexArray
// ============================================================================

GLVertexArray::GLVertexArray() {
    glGenVertexArrays(1, &m_arrayID);
}

GLVertexArray::~GLVertexArray() {
    glDeleteVertexArrays(1, &m_arrayID);
}

void GLVertexArray::bind() {
    glBindVertexArray(m_arrayID);
}

void GLVertexArray::unbind() {
    glBindVertexArray(0);
}

void GLVertexArray::addVertexBuffer(VertexBuffer* buffer, const VertexLayout& layout) {
    glBindVertexArray(m_arrayID);
    buffer->bind();

    for (const auto& attr : layout) {
        GLenum glType = GL_FLOAT;
        GLboolean normalized = attr.normalized ? GL_TRUE : GL_FALSE;

        switch (attr.type) {
            case ShaderDataType::Float:
            case ShaderDataType::Float2:
            case ShaderDataType::Float3:
            case ShaderDataType::Float4:
                glType = GL_FLOAT;
                break;
            case ShaderDataType::Int:
            case ShaderDataType::Int2:
            case ShaderDataType::Int3:
            case ShaderDataType::Int4:
                glType = GL_INT;
                break;
            case ShaderDataType::Bool:
                glType = GL_BOOL;
                break;
            default:
                break;
        }

        glEnableVertexAttribArray(m_attributeIndex);
        glVertexAttribPointer(
            m_attributeIndex,
            ShaderDataTypeComponentCount(attr.type),
            glType,
            normalized,
            layout.getStride(),
            reinterpret_cast<const void*>(static_cast<uintptr_t>(attr.offset))
        );

        m_attributeIndex++;
    }
}

void GLVertexArray::setIndexBuffer(IndexBuffer* buffer) {
    glBindVertexArray(m_arrayID);
    buffer->bind();
    m_indexBuffer = buffer;
}

} // namespace Pina
