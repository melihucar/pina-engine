/// Pina Engine - Point Light Implementation

#include "PointLight.h"
#include <cmath>

namespace Pina {

PointLight::PointLight() = default;

void PointLight::setAttenuation(float constant, float linear, float quadratic) {
    m_constant = constant;
    m_linear = linear;
    m_quadratic = quadratic;
}

void PointLight::setRange(float range) {
    m_range = range;

    // Calculate attenuation factors for light to reach ~1% intensity at range
    // Using the formula: attenuation = 1 / (c + l*d + q*d^2)
    // We want attenuation = 0.01 at d = range
    // Solving: 100 = c + l*range + q*range^2
    //
    // Common approach: set constant=1, linear based on range, quadratic for sharp falloff
    // These values are approximations that work well visually
    m_constant = 1.0f;
    m_linear = 4.5f / range;
    m_quadratic = 75.0f / (range * range);
}

} // namespace Pina
