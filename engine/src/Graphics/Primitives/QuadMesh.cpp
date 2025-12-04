/// Pina Engine - Quad Mesh Implementation

#include "QuadMesh.h"
#include "../VertexLayout.h"

namespace Pina {

QuadMesh::QuadMesh(GraphicsDevice* device, float width, float height)
    : Mesh(device)
{
    float hw = width * 0.5f;   // half width
    float hh = height * 0.5f;  // half height

    // Quad vertices: position (xyz) + texcoord (uv)
    // Two triangles forming a quad
    float vertices[] = {
        // First triangle
        -hw, -hh, 0.0f,  0.0f, 0.0f,  // bottom left
         hw, -hh, 0.0f,  1.0f, 0.0f,  // bottom right
         hw,  hh, 0.0f,  1.0f, 1.0f,  // top right

        // Second triangle
        -hw, -hh, 0.0f,  0.0f, 0.0f,  // bottom left
         hw,  hh, 0.0f,  1.0f, 1.0f,  // top right
        -hw,  hh, 0.0f,  0.0f, 1.0f   // top left
    };

    m_vertexCount = 6;

    // Create vertex buffer
    m_vbo = m_device->createVertexBuffer(vertices, sizeof(vertices));

    // Create vertex array and set up layout
    m_vao = m_device->createVertexArray();

    VertexLayout layout;
    layout.push("aPosition", ShaderDataType::Float3);
    layout.push("aTexCoord", ShaderDataType::Float2);
    m_vao->addVertexBuffer(m_vbo.get(), layout);
}

UNIQUE<QuadMesh> QuadMesh::create(GraphicsDevice* device, float width, float height) {
    return UNIQUE<QuadMesh>(new QuadMesh(device, width, height));
}

} // namespace Pina
