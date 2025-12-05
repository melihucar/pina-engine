#pragma once

/// Pina Engine - Directional Light
/// Sun-like light with parallel rays, no position, only direction

#include "Light.h"

namespace Pina {

/// Directional light (sun, moon)
/// Has direction but no position - infinite distance
class PINA_API DirectionalLight : public Light {
public:
    DirectionalLight();
    ~DirectionalLight() override = default;

    LightType getType() const override { return LightType::Directional; }

    /// Set light direction (will be normalized internally)
    /// @param direction Direction the light is pointing (from light towards scene)
    void setDirection(const Vector3& direction);
    const Vector3& getDirection() const { return m_direction; }

private:
    Vector3 m_direction = Vector3(0.0f, -1.0f, 0.0f); // Default: straight down
};

} // namespace Pina
