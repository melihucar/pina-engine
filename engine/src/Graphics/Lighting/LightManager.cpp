/// Pina Engine - Light Manager Implementation

#include "LightManager.h"
#include "../OpenGL/GLCommon.h"

namespace Pina {

LightManager::LightManager() {
    // Initialize all lights as disabled
    for (int i = 0; i < MAX_LIGHTS; ++i) {
        m_lights[i] = nullptr;
        m_lightData[i].direction.w = 0.0f;  // disabled flag
    }
}

int LightManager::addLight(Light* light) {
    if (light == nullptr) {
        return -1;
    }

    // Find first empty slot
    for (int i = 0; i < MAX_LIGHTS; ++i) {
        if (m_lights[i] == nullptr) {
            m_lights[i] = light;
            updateLightData(i);
            m_lightCount++;
            return i;
        }
    }

    return -1;  // Manager is full
}

void LightManager::removeLight(Light* light) {
    if (light == nullptr) return;

    for (int i = 0; i < MAX_LIGHTS; ++i) {
        if (m_lights[i] == light) {
            removeLight(i);
            return;
        }
    }
}

void LightManager::removeLight(int index) {
    if (index < 0 || index >= MAX_LIGHTS || m_lights[index] == nullptr) {
        return;
    }

    m_lights[index] = nullptr;
    m_lightData[index].direction.w = 0.0f;  // Mark as disabled
    m_lightCount--;
}

void LightManager::clear() {
    for (int i = 0; i < MAX_LIGHTS; ++i) {
        m_lights[i] = nullptr;
        m_lightData[i].direction.w = 0.0f;
    }
    m_lightCount = 0;
}

Light* LightManager::getLight(int index) const {
    if (index < 0 || index >= MAX_LIGHTS) {
        return nullptr;
    }
    return m_lights[index];
}

void LightManager::update() {
    for (int i = 0; i < MAX_LIGHTS; ++i) {
        if (m_lights[i] != nullptr) {
            updateLightData(i);
        }
    }
}

void LightManager::updateLightData(int index) {
    Light* light = m_lights[index];
    LightData& data = m_lightData[index];

    if (light == nullptr || !light->isEnabled()) {
        data.direction.w = 0.0f;  // Mark as disabled
        return;
    }

    data.direction.w = 1.0f;  // Mark as enabled

    // Common properties
    Color color = light->getColor();
    float intensity = light->getIntensity();
    data.color = glm::vec4(
        color.r * intensity,
        color.g * intensity,
        color.b * intensity,
        intensity
    );

    Color ambient = light->getAmbient();
    data.ambient = glm::vec4(ambient.r, ambient.g, ambient.b, 0.0f);

    // Type-specific properties
    switch (light->getType()) {
        case LightType::Directional: {
            auto* dirLight = static_cast<DirectionalLight*>(light);
            Vector3 dir = dirLight->getDirection();
            data.position = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);  // w=0 = directional
            data.direction = glm::vec4(dir.x, dir.y, dir.z, 1.0f);  // Keep enabled flag
            data.attenuation = glm::vec4(1.0f, 0.0f, 0.0f, 0.0f);
            data.cutoff = glm::vec4(0.0f);
            break;
        }

        case LightType::Point: {
            auto* pointLight = static_cast<PointLight*>(light);
            Vector3 pos = pointLight->getPosition();
            data.position = glm::vec4(pos.x, pos.y, pos.z, 1.0f);  // w=1 = point
            data.direction = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);  // Keep enabled flag
            data.attenuation = glm::vec4(
                pointLight->getConstant(),
                pointLight->getLinear(),
                pointLight->getQuadratic(),
                pointLight->getRange()
            );
            data.cutoff = glm::vec4(0.0f);
            break;
        }

        case LightType::Spot: {
            auto* spotLight = static_cast<SpotLight*>(light);
            Vector3 pos = spotLight->getPosition();
            Vector3 dir = spotLight->getDirection();
            data.position = glm::vec4(pos.x, pos.y, pos.z, 2.0f);  // w=2 = spot
            data.direction = glm::vec4(dir.x, dir.y, dir.z, 1.0f);  // Keep enabled flag
            data.attenuation = glm::vec4(
                spotLight->getConstant(),
                spotLight->getLinear(),
                spotLight->getQuadratic(),
                spotLight->getRange()
            );
            data.cutoff = glm::vec4(
                spotLight->getInnerCutoffCos(),
                spotLight->getOuterCutoffCos(),
                0.0f, 0.0f
            );
            break;
        }
    }
}

void LightManager::uploadToShader(Shader* shader) const {
    if (shader == nullptr) return;

    // Upload light count
    shader->setInt("uLightCount", m_lightCount);

    // Upload view position for specular calculations
    shader->setVec3("uViewPosition", m_viewPosition);

    // Upload global ambient
    shader->setVec3("uGlobalAmbient",
        glm::vec3(m_globalAmbient.r, m_globalAmbient.g, m_globalAmbient.b));

    // Upload each light's data
    for (int i = 0; i < MAX_LIGHTS; ++i) {
        std::string prefix = "uLights[" + std::to_string(i) + "].";

        shader->setVec4(prefix + "position", m_lightData[i].position);
        shader->setVec4(prefix + "direction", m_lightData[i].direction);
        shader->setVec4(prefix + "color", m_lightData[i].color);
        shader->setVec4(prefix + "ambient", m_lightData[i].ambient);
        shader->setVec4(prefix + "attenuation", m_lightData[i].attenuation);
        shader->setVec4(prefix + "cutoff", m_lightData[i].cutoff);
    }
}

void LightManager::uploadMaterial(Shader* shader, const Material& material) const {
    if (shader == nullptr) return;

    // Upload color properties
    Color diffuse = material.getDiffuse();
    Color specular = material.getSpecular();
    Color ambient = material.getAmbient();
    Color emissive = material.getEmissive();

    shader->setVec3("uMaterial.diffuse", glm::vec3(diffuse.r, diffuse.g, diffuse.b));
    shader->setVec3("uMaterial.specular", glm::vec3(specular.r, specular.g, specular.b));
    shader->setVec3("uMaterial.ambient", glm::vec3(ambient.r, ambient.g, ambient.b));
    shader->setVec3("uMaterial.emissive", glm::vec3(emissive.r, emissive.g, emissive.b));
    shader->setFloat("uMaterial.shininess", material.getShininess());

    // Upload diffuse texture map
    bool useDiffuseMap = material.hasDiffuseMap();
    shader->setInt("uUseDiffuseMap", useDiffuseMap ? 1 : 0);
    if (useDiffuseMap) {
        material.getDiffuseMap()->bind(0);
        shader->setInt("uDiffuseMap", 0);
    }

    // Upload specular texture map
    bool useSpecularMap = material.hasSpecularMap();
    shader->setInt("uUseSpecularMap", useSpecularMap ? 1 : 0);
    if (useSpecularMap) {
        material.getSpecularMap()->bind(1);
        shader->setInt("uSpecularMap", 1);
    }

    // Upload normal texture map
    bool useNormalMap = material.hasNormalMap();
    shader->setInt("uUseNormalMap", useNormalMap ? 1 : 0);
    if (useNormalMap) {
        material.getNormalMap()->bind(2);
        shader->setInt("uNormalMap", 2);
    }
}

void LightManager::uploadPBRMaterial(Shader* shader, const Material& material) const {
    if (shader == nullptr) return;

    // Upload PBR value uniforms
    Color albedo = material.getAlbedo();
    Color emissive = material.getEmissive();

    shader->setVec3("uAlbedo", glm::vec3(albedo.r, albedo.g, albedo.b));
    shader->setFloat("uMetallic", material.getMetallic());
    shader->setFloat("uRoughness", material.getRoughness());
    shader->setFloat("uAO", material.getAO());
    shader->setVec3("uEmissive", glm::vec3(emissive.r, emissive.g, emissive.b));
    shader->setFloat("uOpacity", material.getOpacity());

    // Texture binding and flags
    int textureUnit = 0;

    // Albedo map
    if (material.hasAlbedoMap()) {
        material.getAlbedoMap()->bind(textureUnit);
        shader->setInt("uAlbedoMap", textureUnit++);
        shader->setInt("uUseAlbedoMap", 1);
    } else {
        shader->setInt("uUseAlbedoMap", 0);
    }

    // Metallic-Roughness combined map (glTF format: G=roughness, B=metallic)
    if (material.hasMetallicRoughnessMap()) {
        material.getMetallicRoughnessMap()->bind(textureUnit);
        shader->setInt("uMetallicRoughnessMap", textureUnit++);
        shader->setInt("uUseMetallicRoughnessMap", 1);
    } else {
        shader->setInt("uUseMetallicRoughnessMap", 0);
    }

    // Separate metallic map
    if (material.hasMetallicMap()) {
        material.getMetallicMap()->bind(textureUnit);
        shader->setInt("uMetallicMap", textureUnit++);
        shader->setInt("uUseMetallicMap", 1);
    } else {
        shader->setInt("uUseMetallicMap", 0);
    }

    // Separate roughness map
    if (material.hasRoughnessMap()) {
        material.getRoughnessMap()->bind(textureUnit);
        shader->setInt("uRoughnessMap", textureUnit++);
        shader->setInt("uUseRoughnessMap", 1);
    } else {
        shader->setInt("uUseRoughnessMap", 0);
    }

    // Normal map
    if (material.hasNormalMap()) {
        material.getNormalMap()->bind(textureUnit);
        shader->setInt("uNormalMap", textureUnit++);
        shader->setInt("uUseNormalMap", 1);
    } else {
        shader->setInt("uUseNormalMap", 0);
    }

    // AO map
    if (material.hasAOMap()) {
        material.getAOMap()->bind(textureUnit);
        shader->setInt("uAOMap", textureUnit++);
        shader->setInt("uUseAOMap", 1);
    } else {
        shader->setInt("uUseAOMap", 0);
    }

    // Emission map
    if (material.hasEmissionMap()) {
        material.getEmissionMap()->bind(textureUnit);
        shader->setInt("uEmissionMap", textureUnit++);
        shader->setInt("uUseEmissionMap", 1);
    } else {
        shader->setInt("uUseEmissionMap", 0);
    }
}

// ========================================================================
// Shadow Mapping Support
// ========================================================================

DirectionalLight* LightManager::getShadowCastingLight() const {
    for (int i = 0; i < MAX_LIGHTS; ++i) {
        if (m_lights[i] && m_lights[i]->isEnabled() &&
            m_lights[i]->getType() == LightType::Directional) {
            auto* dirLight = static_cast<DirectionalLight*>(m_lights[i]);
            if (dirLight->getCastsShadow()) {
                return dirLight;
            }
        }
    }
    return nullptr;
}

void LightManager::uploadShadowUniforms(Shader* shader, uint32_t shadowMapTextureID) const {
    if (!shader) return;

    // Upload light space matrix
    shader->setMat4("uLightSpaceMatrix", m_lightSpaceMatrix);

    // Shadow map is bound to texture unit 8 (after material textures 0-7)
    shader->setInt("uShadowMap", 8);

    // Bind the shadow map texture
    glActiveTexture(GL_TEXTURE8);
    glBindTexture(GL_TEXTURE_2D, shadowMapTextureID);
}

} // namespace Pina
