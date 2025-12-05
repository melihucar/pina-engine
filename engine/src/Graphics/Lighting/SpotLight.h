#pragma once

/// Pina Engine - Spot Light
/// Cone-shaped light with position, direction, and cutoff angles

#include "Light.h"

namespace Pina {

/// Spotlight (flashlight, stage light, car headlight)
/// Emits light in a cone shape from a position with angular falloff
class PINA_API SpotLight : public Light {
public:
    SpotLight();
    ~SpotLight() override = default;

    LightType getType() const override { return LightType::Spot; }

    // ========================================================================
    // Position & Direction
    // ========================================================================

    /// World position of the light
    void setPosition(const Vector3& position) { m_position = position; }
    const Vector3& getPosition() const { return m_position; }

    /// Direction the spotlight points (will be normalized internally)
    void setDirection(const Vector3& direction);
    const Vector3& getDirection() const { return m_direction; }

    // ========================================================================
    // Cone Angles
    // ========================================================================

    /// Inner cone angle in degrees (full intensity inside this angle)
    void setInnerCutoff(float degrees);
    float getInnerCutoff() const { return m_innerCutoffDegrees; }
    float getInnerCutoffCos() const { return m_innerCutoffCos; }

    /// Outer cone angle in degrees (intensity falls to zero at this angle)
    void setOuterCutoff(float degrees);
    float getOuterCutoff() const { return m_outerCutoffDegrees; }
    float getOuterCutoffCos() const { return m_outerCutoffCos; }

    // ========================================================================
    // Attenuation
    // ========================================================================

    /// Set attenuation factors (same as point light)
    void setAttenuation(float constant, float linear, float quadratic);

    float getConstant() const { return m_constant; }
    float getLinear() const { return m_linear; }
    float getQuadratic() const { return m_quadratic; }

    /// Set attenuation by desired range
    void setRange(float range);
    float getRange() const { return m_range; }

private:
    Vector3 m_position = Vector3::zero();
    Vector3 m_direction = Vector3(0.0f, -1.0f, 0.0f);

    // Cutoff angles (stored as both degrees and cosine for efficiency)
    float m_innerCutoffDegrees = 12.5f;
    float m_outerCutoffDegrees = 17.5f;
    float m_innerCutoffCos = 0.9763f;  // cos(12.5 degrees)
    float m_outerCutoffCos = 0.9537f;  // cos(17.5 degrees)

    // Attenuation
    float m_constant = 1.0f;
    float m_linear = 0.09f;
    float m_quadratic = 0.032f;
    float m_range = 50.0f;
};

} // namespace Pina
