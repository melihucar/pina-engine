/// Pina Engine - Spot Light Implementation

#include "SpotLight.h"
#include <glm/glm.hpp>
#include <cmath>

namespace Pina {

SpotLight::SpotLight() = default;

void SpotLight::setDirection(const Vector3& direction) {
    glm::vec3 dir = glm::normalize(glm::vec3(direction.x, direction.y, direction.z));
    m_direction = Vector3(dir.x, dir.y, dir.z);
}

void SpotLight::setInnerCutoff(float degrees) {
    m_innerCutoffDegrees = degrees;
    m_innerCutoffCos = std::cos(glm::radians(degrees));
}

void SpotLight::setOuterCutoff(float degrees) {
    m_outerCutoffDegrees = degrees;
    m_outerCutoffCos = std::cos(glm::radians(degrees));
}

void SpotLight::setAttenuation(float constant, float linear, float quadratic) {
    m_constant = constant;
    m_linear = linear;
    m_quadratic = quadratic;
}

void SpotLight::setRange(float range) {
    m_range = range;

    // Same formula as PointLight
    m_constant = 1.0f;
    m_linear = 4.5f / range;
    m_quadratic = 75.0f / (range * range);
}

} // namespace Pina
