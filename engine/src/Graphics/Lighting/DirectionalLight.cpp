/// Pina Engine - Directional Light Implementation

#include "DirectionalLight.h"
#include <glm/glm.hpp>

namespace Pina {

DirectionalLight::DirectionalLight() = default;

void DirectionalLight::setDirection(const Vector3& direction) {
    glm::vec3 dir = glm::normalize(glm::vec3(direction.x, direction.y, direction.z));
    m_direction = Vector3(dir.x, dir.y, dir.z);
}

} // namespace Pina
