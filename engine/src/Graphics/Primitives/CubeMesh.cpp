/// Pina Engine - Cube Mesh Implementation

#include "CubeMesh.h"
#include "../VertexLayout.h"

namespace Pina {

CubeMesh::CubeMesh(GraphicsDevice* device, float size)
    : Mesh(device)
{
    float hs = size * 0.5f;  // half size

    // Cube vertices: position (xyz) + normal (xyz) + texcoord (uv)
    // 6 faces * 2 triangles * 3 vertices = 36 vertices
    float vertices[] = {
        // Front face (z = +hs)
        -hs, -hs,  hs,  0.0f, 0.0f, 1.0f,  0.0f, 0.0f,
         hs, -hs,  hs,  0.0f, 0.0f, 1.0f,  1.0f, 0.0f,
         hs,  hs,  hs,  0.0f, 0.0f, 1.0f,  1.0f, 1.0f,
        -hs, -hs,  hs,  0.0f, 0.0f, 1.0f,  0.0f, 0.0f,
         hs,  hs,  hs,  0.0f, 0.0f, 1.0f,  1.0f, 1.0f,
        -hs,  hs,  hs,  0.0f, 0.0f, 1.0f,  0.0f, 1.0f,

        // Back face (z = -hs)
         hs, -hs, -hs,  0.0f, 0.0f, -1.0f,  0.0f, 0.0f,
        -hs, -hs, -hs,  0.0f, 0.0f, -1.0f,  1.0f, 0.0f,
        -hs,  hs, -hs,  0.0f, 0.0f, -1.0f,  1.0f, 1.0f,
         hs, -hs, -hs,  0.0f, 0.0f, -1.0f,  0.0f, 0.0f,
        -hs,  hs, -hs,  0.0f, 0.0f, -1.0f,  1.0f, 1.0f,
         hs,  hs, -hs,  0.0f, 0.0f, -1.0f,  0.0f, 1.0f,

        // Left face (x = -hs)
        -hs, -hs, -hs, -1.0f, 0.0f, 0.0f,  0.0f, 0.0f,
        -hs, -hs,  hs, -1.0f, 0.0f, 0.0f,  1.0f, 0.0f,
        -hs,  hs,  hs, -1.0f, 0.0f, 0.0f,  1.0f, 1.0f,
        -hs, -hs, -hs, -1.0f, 0.0f, 0.0f,  0.0f, 0.0f,
        -hs,  hs,  hs, -1.0f, 0.0f, 0.0f,  1.0f, 1.0f,
        -hs,  hs, -hs, -1.0f, 0.0f, 0.0f,  0.0f, 1.0f,

        // Right face (x = +hs)
         hs, -hs,  hs,  1.0f, 0.0f, 0.0f,  0.0f, 0.0f,
         hs, -hs, -hs,  1.0f, 0.0f, 0.0f,  1.0f, 0.0f,
         hs,  hs, -hs,  1.0f, 0.0f, 0.0f,  1.0f, 1.0f,
         hs, -hs,  hs,  1.0f, 0.0f, 0.0f,  0.0f, 0.0f,
         hs,  hs, -hs,  1.0f, 0.0f, 0.0f,  1.0f, 1.0f,
         hs,  hs,  hs,  1.0f, 0.0f, 0.0f,  0.0f, 1.0f,

        // Top face (y = +hs)
        -hs,  hs,  hs,  0.0f, 1.0f, 0.0f,  0.0f, 0.0f,
         hs,  hs,  hs,  0.0f, 1.0f, 0.0f,  1.0f, 0.0f,
         hs,  hs, -hs,  0.0f, 1.0f, 0.0f,  1.0f, 1.0f,
        -hs,  hs,  hs,  0.0f, 1.0f, 0.0f,  0.0f, 0.0f,
         hs,  hs, -hs,  0.0f, 1.0f, 0.0f,  1.0f, 1.0f,
        -hs,  hs, -hs,  0.0f, 1.0f, 0.0f,  0.0f, 1.0f,

        // Bottom face (y = -hs)
        -hs, -hs, -hs,  0.0f, -1.0f, 0.0f,  0.0f, 0.0f,
         hs, -hs, -hs,  0.0f, -1.0f, 0.0f,  1.0f, 0.0f,
         hs, -hs,  hs,  0.0f, -1.0f, 0.0f,  1.0f, 1.0f,
        -hs, -hs, -hs,  0.0f, -1.0f, 0.0f,  0.0f, 0.0f,
         hs, -hs,  hs,  0.0f, -1.0f, 0.0f,  1.0f, 1.0f,
        -hs, -hs,  hs,  0.0f, -1.0f, 0.0f,  0.0f, 1.0f,
    };

    m_vertexCount = 36;

    // Create vertex buffer
    m_vbo = m_device->createVertexBuffer(vertices, sizeof(vertices));

    // Create vertex array and set up layout
    m_vao = m_device->createVertexArray();

    VertexLayout layout;
    layout.push("aPosition", ShaderDataType::Float3);
    layout.push("aNormal", ShaderDataType::Float3);
    layout.push("aTexCoord", ShaderDataType::Float2);
    m_vao->addVertexBuffer(m_vbo.get(), layout);
}

UNIQUE<CubeMesh> CubeMesh::create(GraphicsDevice* device, float size) {
    return UNIQUE<CubeMesh>(new CubeMesh(device, size));
}

} // namespace Pina
