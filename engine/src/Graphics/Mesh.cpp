/// Pina Engine - Base Mesh Implementation

#include "Mesh.h"

namespace Pina {

Mesh::Mesh(GraphicsDevice* device)
    : m_device(device)
{
}

void Mesh::draw(Shader* shader) {
    m_device->draw(m_vao.get(), shader, m_vertexCount);
}

} // namespace Pina
