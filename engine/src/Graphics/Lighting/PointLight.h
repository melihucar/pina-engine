#pragma once

/// Pina Engine - Point Light
/// Omnidirectional light with position and distance attenuation

#include "Light.h"

namespace Pina {

/// Point light (light bulb, candle, torch)
/// Emits light in all directions from a position with distance falloff
class PINA_API PointLight : public Light {
public:
    PointLight();
    ~PointLight() override = default;

    LightType getType() const override { return LightType::Point; }

    // ========================================================================
    // Position
    // ========================================================================

    /// World position of the light
    void setPosition(const Vector3& position) { m_position = position; }
    const Vector3& getPosition() const { return m_position; }

    // ========================================================================
    // Attenuation
    // ========================================================================

    /// Set attenuation factors directly
    /// Attenuation = 1.0 / (constant + linear * d + quadratic * d^2)
    /// @param constant Constant factor (usually 1.0)
    /// @param linear Linear falloff factor
    /// @param quadratic Quadratic falloff factor
    void setAttenuation(float constant, float linear, float quadratic);

    float getConstant() const { return m_constant; }
    float getLinear() const { return m_linear; }
    float getQuadratic() const { return m_quadratic; }

    /// Convenience: set attenuation by desired range
    /// Auto-calculates attenuation factors for light to reach ~1% at this distance
    /// @param range Distance at which light effectively fades out
    void setRange(float range);
    float getRange() const { return m_range; }

private:
    Vector3 m_position = Vector3::zero();

    // Attenuation coefficients
    float m_constant = 1.0f;
    float m_linear = 0.09f;
    float m_quadratic = 0.032f;
    float m_range = 50.0f;
};

} // namespace Pina
