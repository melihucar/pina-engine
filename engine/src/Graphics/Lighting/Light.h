#pragma once

/// Pina Engine - Light Base Class
/// Abstract base class for all light types

#include "../../Core/Export.h"
#include "../../Math/Color.h"
#include "../../Math/Vector3.h"

namespace Pina {

/// Maximum number of simultaneous lights
constexpr int MAX_LIGHTS = 8;

/// Light types supported by the engine
enum class PINA_API LightType {
    Directional = 0,  // Sun-like, infinite distance, parallel rays
    Point = 1,        // Omnidirectional, position-based with attenuation
    Spot = 2          // Cone-shaped, position + direction with cutoff angles
};

/// Base light class - common properties for all light types
class PINA_API Light {
public:
    Light();
    virtual ~Light() = default;

    // ========================================================================
    // Type
    // ========================================================================

    /// Get the type of this light
    virtual LightType getType() const = 0;

    // ========================================================================
    // Common Properties
    // ========================================================================

    /// Enable/disable this light
    void setEnabled(bool enabled) { m_enabled = enabled; }
    bool isEnabled() const { return m_enabled; }

    /// Light color (RGB, 0-1 range)
    void setColor(const Color& color) { m_color = color; }
    const Color& getColor() const { return m_color; }

    /// Light intensity multiplier
    void setIntensity(float intensity) { m_intensity = intensity; }
    float getIntensity() const { return m_intensity; }

    /// Ambient contribution (global illumination approximation)
    void setAmbient(const Color& ambient) { m_ambient = ambient; }
    const Color& getAmbient() const { return m_ambient; }

    // Non-copyable but movable
    Light(const Light&) = delete;
    Light& operator=(const Light&) = delete;
    Light(Light&&) = default;
    Light& operator=(Light&&) = default;

protected:
    bool m_enabled = true;
    Color m_color = Color::white();
    float m_intensity = 1.0f;
    Color m_ambient = Color(0.1f, 0.1f, 0.1f, 1.0f);
};

} // namespace Pina
