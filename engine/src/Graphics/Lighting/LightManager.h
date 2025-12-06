#pragma once

/// Pina Engine - Light Manager
/// Manages active lights and uploads them to shaders

#include "Light.h"
#include "DirectionalLight.h"
#include "PointLight.h"
#include "SpotLight.h"
#include "../Shader.h"
#include "../Material.h"
#include "../Texture.h"
#include "../../Core/Export.h"
#include <array>
#include <glm/glm.hpp>

namespace Pina {

/// GPU-friendly light data structure (matches GLSL uniform layout)
/// Uses vec4 for proper GPU memory alignment
struct PINA_API LightData {
    glm::vec4 position;    // xyz = position, w = type (0=dir, 1=point, 2=spot)
    glm::vec4 direction;   // xyz = direction, w = enabled (0 or 1)
    glm::vec4 color;       // rgb = color * intensity, a = intensity
    glm::vec4 ambient;     // rgb = ambient color, a = unused
    glm::vec4 attenuation; // x = constant, y = linear, z = quadratic, w = range
    glm::vec4 cutoff;      // x = innerCos, y = outerCos, z/w = unused
};

/// Manages active lights and uploads them to shaders
class PINA_API LightManager {
public:
    LightManager();
    ~LightManager() = default;

    // ========================================================================
    // Light Management
    // ========================================================================

    /// Add a light to the manager
    /// @param light Pointer to light (must remain valid while added)
    /// @return Index of the light, or -1 if manager is full
    int addLight(Light* light);

    /// Remove a light from the manager
    /// @param light Pointer to light to remove
    void removeLight(Light* light);

    /// Remove light by index
    /// @param index Index of light to remove
    void removeLight(int index);

    /// Clear all lights
    void clear();

    /// Get number of active lights
    int getLightCount() const { return m_lightCount; }

    /// Get light by index (nullptr if invalid)
    Light* getLight(int index) const;

    // ========================================================================
    // Shader Upload
    // ========================================================================

    /// Upload all light data to a shader
    /// Call this before rendering each object that uses this lighting setup
    /// @param shader Shader to upload uniforms to
    void uploadToShader(Shader* shader) const;

    /// Upload material data to a shader (Blinn-Phong workflow)
    /// @param shader Shader to upload uniforms to
    /// @param material Material properties to upload
    void uploadMaterial(Shader* shader, const Material& material) const;

    /// Upload PBR material data to a shader (Metallic-Roughness workflow)
    /// @param shader Shader to upload uniforms to
    /// @param material Material properties to upload
    void uploadPBRMaterial(Shader* shader, const Material& material) const;

    /// Set camera/view position (needed for specular calculations)
    /// @param position World-space camera position
    void setViewPosition(const glm::vec3& position) { m_viewPosition = position; }
    const glm::vec3& getViewPosition() const { return m_viewPosition; }

    // ========================================================================
    // Global Settings
    // ========================================================================

    /// Global ambient light (added to all objects regardless of lights)
    void setGlobalAmbient(const Color& ambient) { m_globalAmbient = ambient; }
    const Color& getGlobalAmbient() const { return m_globalAmbient; }

    // ========================================================================
    // Update
    // ========================================================================

    /// Update internal light data from light objects
    /// Call this if you modify lights after adding them
    void update();

    // ========================================================================
    // Shadow Mapping Support
    // ========================================================================

    /// Set the light space matrix (computed by ShadowPass)
    void setLightSpaceMatrix(const glm::mat4& matrix) { m_lightSpaceMatrix = matrix; }
    const glm::mat4& getLightSpaceMatrix() const { return m_lightSpaceMatrix; }

    /// Upload shadow-related uniforms to a shader
    /// @param shader Shader to upload uniforms to
    /// @param shadowMapTextureID OpenGL texture ID for shadow map
    void uploadShadowUniforms(Shader* shader, uint32_t shadowMapTextureID) const;

    /// Get the first directional light that casts shadows
    /// @return Pointer to shadow-casting directional light, or nullptr if none
    DirectionalLight* getShadowCastingLight() const;

private:
    void updateLightData(int index);

    std::array<Light*, MAX_LIGHTS> m_lights = {};
    std::array<LightData, MAX_LIGHTS> m_lightData = {};
    int m_lightCount = 0;

    glm::vec3 m_viewPosition = glm::vec3(0.0f);
    Color m_globalAmbient = Color(0.03f, 0.03f, 0.03f, 1.0f);

    // Shadow mapping
    glm::mat4 m_lightSpaceMatrix = glm::mat4(1.0f);
};

} // namespace Pina
