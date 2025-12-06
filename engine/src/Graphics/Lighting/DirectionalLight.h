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

    // ========================================================================
    // Shadow Configuration
    // ========================================================================

    /// Enable/disable shadow casting for this light
    void setCastsShadow(bool casts) { m_castsShadow = casts; }
    bool getCastsShadow() const { return m_castsShadow; }

    /// Shadow map resolution (width and height)
    void setShadowMapSize(int size) { m_shadowMapSize = size; }
    int getShadowMapSize() const { return m_shadowMapSize; }

    /// Shadow bias to prevent shadow acne
    void setShadowBias(float bias) { m_shadowBias = bias; }
    float getShadowBias() const { return m_shadowBias; }

    /// Normal bias for shadow mapping (offsets along surface normal)
    void setShadowNormalBias(float bias) { m_shadowNormalBias = bias; }
    float getShadowNormalBias() const { return m_shadowNormalBias; }

    /// Orthographic projection size for shadow frustum
    void setShadowOrthoSize(float size) { m_shadowOrthoSize = size; }
    float getShadowOrthoSize() const { return m_shadowOrthoSize; }

    /// Near plane for shadow projection
    void setShadowNearPlane(float near) { m_shadowNearPlane = near; }
    float getShadowNearPlane() const { return m_shadowNearPlane; }

    /// Far plane for shadow projection
    void setShadowFarPlane(float far) { m_shadowFarPlane = far; }
    float getShadowFarPlane() const { return m_shadowFarPlane; }

    /// Shadow softness (blur radius multiplier: 0.5 = sharper, 2.0+ = softer)
    void setShadowSoftness(float softness) { m_shadowSoftness = softness; }
    float getShadowSoftness() const { return m_shadowSoftness; }

private:
    Vector3 m_direction = Vector3(0.0f, -1.0f, 0.0f); // Default: straight down

    // Shadow configuration
    bool m_castsShadow = true;
    int m_shadowMapSize = 2048;
    float m_shadowBias = 0.005f;
    float m_shadowNormalBias = 0.02f;
    float m_shadowOrthoSize = 20.0f;
    float m_shadowNearPlane = 0.1f;
    float m_shadowFarPlane = 100.0f;
    float m_shadowSoftness = 1.5f;  // Default to softer shadows
};

} // namespace Pina
