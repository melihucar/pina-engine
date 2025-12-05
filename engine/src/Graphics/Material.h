#pragma once

/// Pina Engine - Material
/// Surface material properties for lighting calculations

#include "../Core/Export.h"
#include "../Math/Color.h"

namespace Pina {

// Forward declarations
class Texture;

/// Basic material properties for Blinn-Phong lighting
/// Extensible for future PBR materials
class PINA_API Material {
public:
    Material();
    ~Material() = default;

    // ========================================================================
    // Phong/Blinn-Phong Properties
    // ========================================================================

    /// Diffuse color (main surface color, affected by light)
    void setDiffuse(const Color& diffuse) { m_diffuse = diffuse; }
    const Color& getDiffuse() const { return m_diffuse; }

    /// Specular color (highlight color, reflective component)
    void setSpecular(const Color& specular) { m_specular = specular; }
    const Color& getSpecular() const { return m_specular; }

    /// Shininess (specular exponent, higher = tighter highlights)
    /// Typical values: 8 (rough), 32 (plastic), 64 (metal), 256+ (mirror)
    void setShininess(float shininess) { m_shininess = shininess; }
    float getShininess() const { return m_shininess; }

    /// Ambient color (approximation of indirect lighting)
    void setAmbient(const Color& ambient) { m_ambient = ambient; }
    const Color& getAmbient() const { return m_ambient; }

    /// Emissive color (self-illumination, unaffected by lights)
    void setEmissive(const Color& emissive) { m_emissive = emissive; }
    const Color& getEmissive() const { return m_emissive; }

    // ========================================================================
    // Texture Maps
    // ========================================================================

    /// Diffuse/albedo texture map (multiplied with diffuse color)
    /// @param texture Raw pointer to texture (ownership not transferred)
    void setDiffuseMap(Texture* texture) { m_diffuseMap = texture; }
    Texture* getDiffuseMap() const { return m_diffuseMap; }
    bool hasDiffuseMap() const { return m_diffuseMap != nullptr; }

    /// Specular map (multiplied with specular color)
    /// @param texture Raw pointer to texture (ownership not transferred)
    void setSpecularMap(Texture* texture) { m_specularMap = texture; }
    Texture* getSpecularMap() const { return m_specularMap; }
    bool hasSpecularMap() const { return m_specularMap != nullptr; }

    /// Normal/bump map (for per-pixel lighting detail)
    /// @param texture Raw pointer to texture (ownership not transferred)
    void setNormalMap(Texture* texture) { m_normalMap = texture; }
    Texture* getNormalMap() const { return m_normalMap; }
    bool hasNormalMap() const { return m_normalMap != nullptr; }

    // ========================================================================
    // Factory Presets
    // ========================================================================

    /// Default white material
    static Material createDefault();

    /// Metal-like material (high specular, tight highlights)
    /// @param color Base color of the metal
    /// @param shininess Specular exponent (default 64)
    static Material createMetal(const Color& color, float shininess = 64.0f);

    /// Plastic-like material (colored diffuse, white specular)
    /// @param color Base color of the plastic
    /// @param shininess Specular exponent (default 32)
    static Material createPlastic(const Color& color, float shininess = 32.0f);

    /// Matte/rough material (no specular highlights)
    /// @param color Base color
    static Material createMatte(const Color& color);

    /// Emissive material (glowing, unlit)
    /// @param color Emission color
    /// @param intensity Emission intensity multiplier
    static Material createEmissive(const Color& color, float intensity = 1.0f);

private:
    // Phong/Blinn-Phong colors
    Color m_diffuse = Color::white();
    Color m_specular = Color(0.5f, 0.5f, 0.5f, 1.0f);
    Color m_ambient = Color(0.1f, 0.1f, 0.1f, 1.0f);
    Color m_emissive = Color(0.0f, 0.0f, 0.0f, 1.0f);
    float m_shininess = 32.0f;

    // Texture maps (raw pointers - ownership managed externally)
    Texture* m_diffuseMap = nullptr;
    Texture* m_specularMap = nullptr;
    Texture* m_normalMap = nullptr;
};

} // namespace Pina
