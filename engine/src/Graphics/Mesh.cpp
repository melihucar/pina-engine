/// Pina Engine - Base Mesh Implementation

#include "Mesh.h"

namespace Pina {

Mesh::Mesh(GraphicsDevice* device)
    : m_device(device)
{
}

void Mesh::draw() {
    m_device->draw(m_vao.get(), m_vertexCount);
}

} // namespace Pina
