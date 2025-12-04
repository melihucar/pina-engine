/// Pina Engine - Triangle Mesh Implementation

#include "TriangleMesh.h"
#include "../VertexLayout.h"

namespace Pina {

TriangleMesh::TriangleMesh(GraphicsDevice* device)
    : Mesh(device)
{
    // Triangle vertices: position (xyz) + color (rgb)
    float vertices[] = {
        // positions         // colors
        -0.5f, -0.5f, 0.0f,  1.0f, 0.0f, 0.0f,  // bottom left (red)
         0.5f, -0.5f, 0.0f,  0.0f, 1.0f, 0.0f,  // bottom right (green)
         0.0f,  0.5f, 0.0f,  0.0f, 0.0f, 1.0f   // top (blue)
    };

    m_vertexCount = 3;

    // Create vertex buffer
    m_vbo = m_device->createVertexBuffer(vertices, sizeof(vertices));

    // Create vertex array and set up layout
    m_vao = m_device->createVertexArray();

    VertexLayout layout;
    layout.push("aPosition", ShaderDataType::Float3);
    layout.push("aColor", ShaderDataType::Float3);
    m_vao->addVertexBuffer(m_vbo.get(), layout);
}

UNIQUE<TriangleMesh> TriangleMesh::create(GraphicsDevice* device) {
    return UNIQUE<TriangleMesh>(new TriangleMesh(device));
}

} // namespace Pina
