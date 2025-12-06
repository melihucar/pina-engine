#pragma once

/// Pina Engine - Material
/// Surface material properties for lighting calculations
/// Supports both Blinn-Phong and PBR (metallic-roughness) workflows

#include "../Core/Export.h"
#include "../Math/Color.h"

namespace Pina {

// Forward declarations
class Texture;

/// Material workflow type
enum class MaterialWorkflow {
    BlinnPhong,           // Traditional Blinn-Phong shading
    PBR_MetallicRoughness // Physically-Based Rendering with metallic-roughness
};

/// Material properties for lighting calculations
/// Supports both Blinn-Phong and PBR workflows with auto-detection
class PINA_API Material {
public:
    Material();
    ~Material() = default;

    // ========================================================================
    // Workflow Detection
    // ========================================================================

    /// Determine material workflow based on properties
    MaterialWorkflow getWorkflow() const;

    /// Check if this material uses PBR
    bool isPBR() const { return getWorkflow() == MaterialWorkflow::PBR_MetallicRoughness; }

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
    // PBR Properties (Metallic-Roughness Workflow)
    // ========================================================================

    /// Albedo color (base color for PBR)
    void setAlbedo(const Color& albedo) { m_albedo = albedo; m_hasPBRValues = true; }
    const Color& getAlbedo() const { return m_albedo; }

    /// Metallic factor (0 = dielectric, 1 = metal)
    void setMetallic(float metallic) { m_metallic = metallic; m_hasPBRValues = true; }
    float getMetallic() const { return m_metallic; }

    /// Roughness factor (0 = smooth/mirror, 1 = rough)
    void setRoughness(float roughness) { m_roughness = roughness; m_hasPBRValues = true; }
    float getRoughness() const { return m_roughness; }

    /// Ambient occlusion factor (0 = fully occluded, 1 = no occlusion)
    void setAO(float ao) { m_ao = ao; }
    float getAO() const { return m_ao; }

    /// Opacity factor (0 = transparent, 1 = opaque)
    void setOpacity(float opacity) { m_opacity = opacity; }
    float getOpacity() const { return m_opacity; }

    // ========================================================================
    // Blinn-Phong Texture Maps
    // ========================================================================

    /// Diffuse/albedo texture map (multiplied with diffuse color)
    void setDiffuseMap(Texture* texture) { m_diffuseMap = texture; }
    Texture* getDiffuseMap() const { return m_diffuseMap; }
    bool hasDiffuseMap() const { return m_diffuseMap != nullptr; }

    /// Specular map (multiplied with specular color)
    void setSpecularMap(Texture* texture) { m_specularMap = texture; }
    Texture* getSpecularMap() const { return m_specularMap; }
    bool hasSpecularMap() const { return m_specularMap != nullptr; }

    /// Normal/bump map (for per-pixel lighting detail)
    void setNormalMap(Texture* texture) { m_normalMap = texture; }
    Texture* getNormalMap() const { return m_normalMap; }
    bool hasNormalMap() const { return m_normalMap != nullptr; }

    // ========================================================================
    // PBR Texture Maps
    // ========================================================================

    /// Albedo/base color texture map
    void setAlbedoMap(Texture* texture) { m_albedoMap = texture; m_hasPBRTextures = true; }
    Texture* getAlbedoMap() const { return m_albedoMap; }
    bool hasAlbedoMap() const { return m_albedoMap != nullptr; }

    /// Metallic texture map (typically in B channel)
    void setMetallicMap(Texture* texture) { m_metallicMap = texture; m_hasPBRTextures = true; }
    Texture* getMetallicMap() const { return m_metallicMap; }
    bool hasMetallicMap() const { return m_metallicMap != nullptr; }

    /// Roughness texture map (typically in G channel)
    void setRoughnessMap(Texture* texture) { m_roughnessMap = texture; m_hasPBRTextures = true; }
    Texture* getRoughnessMap() const { return m_roughnessMap; }
    bool hasRoughnessMap() const { return m_roughnessMap != nullptr; }

    /// Combined metallic-roughness map (glTF format: G=roughness, B=metallic)
    void setMetallicRoughnessMap(Texture* texture) { m_metallicRoughnessMap = texture; m_hasPBRTextures = true; }
    Texture* getMetallicRoughnessMap() const { return m_metallicRoughnessMap; }
    bool hasMetallicRoughnessMap() const { return m_metallicRoughnessMap != nullptr; }

    /// Ambient occlusion texture map
    void setAOMap(Texture* texture) { m_aoMap = texture; }
    Texture* getAOMap() const { return m_aoMap; }
    bool hasAOMap() const { return m_aoMap != nullptr; }

    /// Emission texture map (for glowing surfaces)
    void setEmissionMap(Texture* texture) { m_emissionMap = texture; }
    Texture* getEmissionMap() const { return m_emissionMap; }
    bool hasEmissionMap() const { return m_emissionMap != nullptr; }

    /// Opacity/alpha texture map
    void setOpacityMap(Texture* texture) { m_opacityMap = texture; }
    Texture* getOpacityMap() const { return m_opacityMap; }
    bool hasOpacityMap() const { return m_opacityMap != nullptr; }

    // ========================================================================
    // Factory Presets
    // ========================================================================

    /// Default white material (Blinn-Phong)
    static Material createDefault();

    /// Metal-like material (high specular, tight highlights)
    static Material createMetal(const Color& color, float shininess = 64.0f);

    /// Plastic-like material (colored diffuse, white specular)
    static Material createPlastic(const Color& color, float shininess = 32.0f);

    /// Matte/rough material (no specular highlights)
    static Material createMatte(const Color& color);

    /// Emissive material (glowing, unlit)
    static Material createEmissive(const Color& color, float intensity = 1.0f);

    /// PBR metal material
    static Material createPBRMetal(const Color& albedo, float roughness = 0.3f);

    /// PBR dielectric (non-metal) material
    static Material createPBRDielectric(const Color& albedo, float roughness = 0.5f);

private:
    // Phong/Blinn-Phong colors
    Color m_diffuse = Color::white();
    Color m_specular = Color(0.5f, 0.5f, 0.5f, 1.0f);
    Color m_ambient = Color(0.1f, 0.1f, 0.1f, 1.0f);
    Color m_emissive = Color(0.0f, 0.0f, 0.0f, 1.0f);
    float m_shininess = 32.0f;

    // PBR properties
    Color m_albedo = Color::white();
    float m_metallic = 0.0f;
    float m_roughness = 0.5f;
    float m_ao = 1.0f;
    float m_opacity = 1.0f;

    // Blinn-Phong texture maps
    Texture* m_diffuseMap = nullptr;
    Texture* m_specularMap = nullptr;
    Texture* m_normalMap = nullptr;

    // PBR texture maps
    Texture* m_albedoMap = nullptr;
    Texture* m_metallicMap = nullptr;
    Texture* m_roughnessMap = nullptr;
    Texture* m_metallicRoughnessMap = nullptr;
    Texture* m_aoMap = nullptr;
    Texture* m_emissionMap = nullptr;
    Texture* m_opacityMap = nullptr;

    // Workflow detection flags
    bool m_hasPBRValues = false;
    bool m_hasPBRTextures = false;
};

} // namespace Pina
