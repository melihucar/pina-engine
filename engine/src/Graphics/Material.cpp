/// Pina Engine - Material Implementation

#include "Material.h"

namespace Pina {

Material::Material() = default;

MaterialWorkflow Material::getWorkflow() const {
    // If any PBR textures are set, use PBR
    if (m_hasPBRTextures || m_albedoMap || m_metallicMap ||
        m_roughnessMap || m_metallicRoughnessMap) {
        return MaterialWorkflow::PBR_MetallicRoughness;
    }

    // If PBR values were explicitly set, use PBR
    if (m_hasPBRValues) {
        return MaterialWorkflow::PBR_MetallicRoughness;
    }

    // Default to Blinn-Phong
    return MaterialWorkflow::BlinnPhong;
}

Material Material::createDefault() {
    Material mat;
    mat.m_diffuse = Color::white();
    mat.m_specular = Color(0.5f, 0.5f, 0.5f, 1.0f);
    mat.m_ambient = Color(0.1f, 0.1f, 0.1f, 1.0f);
    mat.m_emissive = Color(0.0f, 0.0f, 0.0f, 1.0f);
    mat.m_shininess = 32.0f;
    return mat;
}

Material Material::createMetal(const Color& color, float shininess) {
    Material mat;
    mat.m_diffuse = color;
    // Metals have colored specular reflections
    mat.m_specular = Color(
        color.r * 0.8f + 0.2f,
        color.g * 0.8f + 0.2f,
        color.b * 0.8f + 0.2f,
        1.0f
    );
    mat.m_ambient = Color(color.r * 0.1f, color.g * 0.1f, color.b * 0.1f, 1.0f);
    mat.m_emissive = Color(0.0f, 0.0f, 0.0f, 1.0f);
    mat.m_shininess = shininess;
    return mat;
}

Material Material::createPlastic(const Color& color, float shininess) {
    Material mat;
    mat.m_diffuse = color;
    // Plastics have white specular reflections
    mat.m_specular = Color(0.5f, 0.5f, 0.5f, 1.0f);
    mat.m_ambient = Color(color.r * 0.1f, color.g * 0.1f, color.b * 0.1f, 1.0f);
    mat.m_emissive = Color(0.0f, 0.0f, 0.0f, 1.0f);
    mat.m_shininess = shininess;
    return mat;
}

Material Material::createMatte(const Color& color) {
    Material mat;
    mat.m_diffuse = color;
    // No specular for matte surfaces
    mat.m_specular = Color(0.0f, 0.0f, 0.0f, 1.0f);
    mat.m_ambient = Color(color.r * 0.1f, color.g * 0.1f, color.b * 0.1f, 1.0f);
    mat.m_emissive = Color(0.0f, 0.0f, 0.0f, 1.0f);
    mat.m_shininess = 1.0f;
    return mat;
}

Material Material::createEmissive(const Color& color, float intensity) {
    Material mat;
    mat.m_diffuse = Color(0.0f, 0.0f, 0.0f, 1.0f);
    mat.m_specular = Color(0.0f, 0.0f, 0.0f, 1.0f);
    mat.m_ambient = Color(0.0f, 0.0f, 0.0f, 1.0f);
    mat.m_emissive = Color(
        color.r * intensity,
        color.g * intensity,
        color.b * intensity,
        1.0f
    );
    mat.m_shininess = 1.0f;
    return mat;
}

Material Material::createPBRMetal(const Color& albedo, float roughness) {
    Material mat;
    mat.m_albedo = albedo;
    mat.m_metallic = 1.0f;
    mat.m_roughness = roughness;
    mat.m_ao = 1.0f;
    mat.m_hasPBRValues = true;
    return mat;
}

Material Material::createPBRDielectric(const Color& albedo, float roughness) {
    Material mat;
    mat.m_albedo = albedo;
    mat.m_metallic = 0.0f;
    mat.m_roughness = roughness;
    mat.m_ao = 1.0f;
    mat.m_hasPBRValues = true;
    return mat;
}

} // namespace Pina
