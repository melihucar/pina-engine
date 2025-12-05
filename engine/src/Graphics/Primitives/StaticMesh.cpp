/// Pina Engine - Static Mesh Implementation

#include "StaticMesh.h"
#include "../VertexLayout.h"

namespace Pina {

StaticMesh::StaticMesh(GraphicsDevice* device,
                       const float* vertices,
                       uint32_t vertexCount,
                       const uint32_t* indices,
                       uint32_t indexCount)
    : Mesh(device)
    , m_indexCount(indexCount)
{
    m_vertexCount = vertexCount;

    // Vertex format: position (3) + normal (3) + texcoord (2) = 8 floats = 32 bytes
    size_t vertexSize = 8 * sizeof(float);
    size_t bufferSize = vertexCount * vertexSize;

    // Create vertex buffer
    m_vbo = m_device->createVertexBuffer(vertices, bufferSize);

    // Create index buffer
    m_ibo = m_device->createIndexBuffer(indices, indexCount);

    // Create vertex array and set up layout
    m_vao = m_device->createVertexArray();

    VertexLayout layout;
    layout.push("aPosition", ShaderDataType::Float3);
    layout.push("aNormal", ShaderDataType::Float3);
    layout.push("aTexCoord", ShaderDataType::Float2);
    m_vao->addVertexBuffer(m_vbo.get(), layout);
    m_vao->setIndexBuffer(m_ibo.get());
}

void StaticMesh::draw() {
    m_device->drawIndexed(m_vao.get());
}

UNIQUE<StaticMesh> StaticMesh::create(GraphicsDevice* device,
                                      const float* vertices,
                                      uint32_t vertexCount,
                                      const uint32_t* indices,
                                      uint32_t indexCount) {
    return UNIQUE<StaticMesh>(new StaticMesh(device, vertices, vertexCount, indices, indexCount));
}

UNIQUE<StaticMesh> StaticMesh::create(GraphicsDevice* device,
                                      const std::vector<float>& vertices,
                                      const std::vector<uint32_t>& indices) {
    // Each vertex has 8 floats (pos + normal + texcoord)
    uint32_t vertexCount = static_cast<uint32_t>(vertices.size() / 8);
    uint32_t indexCount = static_cast<uint32_t>(indices.size());
    return create(device, vertices.data(), vertexCount, indices.data(), indexCount);
}

} // namespace Pina
