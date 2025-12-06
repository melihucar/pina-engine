#pragma once

/// Pina Engine - Shadow Pass
/// Renders shadow depth maps from light perspective

#include "../RenderPass.h"
#include "../RenderContext.h"
#include "../Framebuffer.h"
#include "../Shader.h"
#include "../../Core/Memory.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

namespace Pina {

// Forward declaration
class GraphicsDevice;

/// Pass that renders shadow maps from lights
class PINA_API ShadowPass : public RenderPass {
public:
    ShadowPass() {
        name = "shadows";
        needsSwap = false;  // Writes to named target, not ping-pong
        clear = false;      // We'll clear the shadow map manually
    }

    void initialize(RenderContext& ctx) override {
        // Create shadow depth shader if not provided
        if (!m_shadowShader && ctx.device) {
            m_shadowShader = ctx.device->createShader();
            if (!m_shadowShader->load(getShadowVertexShader(), getShadowFragmentShader())) {
                std::cerr << "ShadowPass: Failed to create shadow shader" << std::endl;
            }
        }
    }

    void execute(RenderContext& ctx) override {
        if (!ctx.scene || !ctx.lights) {
            return;
        }

        // Get the shadow map framebuffer
        Framebuffer* shadowFB = ctx.getTarget(outputTarget);
        if (!shadowFB) {
            return;
        }

        // Bind shadow framebuffer
        shadowFB->bind();

        // Clear depth
        shadowFB->clearDepth(1.0f);

        // Setup for depth-only rendering
        ctx.device->setDepthTest(true);

        // Use shadow shader
        Shader* shader = m_shadowShader ? m_shadowShader.get() : ctx.shadowShader;
        if (!shader) {
            return;
        }
        shader->bind();

        // Calculate light space matrix for directional light
        // For now, we support one directional light for shadows
        glm::mat4 lightSpaceMatrix = calculateLightSpaceMatrix(ctx);
        shader->setMat4("uLightSpaceMatrix", lightSpaceMatrix);

        // Store light space matrix for scene pass to use
        m_lightSpaceMatrix = lightSpaceMatrix;

        // Render scene from light's perspective
        // Note: This is simplified - a full implementation would iterate
        // through nodes and render their geometry with just depth
        renderSceneDepth(ctx, shader);

        // Unbind
        shadowFB->unbind();
    }

    void resize(int width, int height) override {
        // Shadow maps typically have fixed resolution
        // Don't resize based on viewport
        (void)width;
        (void)height;
    }

    // ========================================================================
    // Configuration
    // ========================================================================

    /// Size of the shadow map (width and height)
    int shadowMapSize = 2048;

    /// Name of the output render target
    std::string outputTarget = "shadowMap";

    /// Shadow bias to prevent shadow acne
    float shadowBias = 0.005f;

    /// Normal bias for shadow mapping
    float shadowNormalBias = 0.02f;

    /// Near plane for shadow projection
    float nearPlane = 0.1f;

    /// Far plane for shadow projection
    float farPlane = 100.0f;

    /// Orthographic projection size (for directional lights)
    float orthoSize = 20.0f;

    /// Get the computed light space matrix (for use in scene pass)
    const glm::mat4& getLightSpaceMatrix() const { return m_lightSpaceMatrix; }

private:
    glm::mat4 calculateLightSpaceMatrix(RenderContext& ctx) {
        // Get the first directional light
        // In a full implementation, you might want to handle multiple lights
        glm::vec3 lightDir = glm::vec3(-0.5f, -1.0f, -0.3f); // Default

        // Try to get direction from scene's light manager
        // This is simplified - would need proper light iteration
        if (ctx.lights && ctx.lights->getLightCount() > 0) {
            // Use first light's direction if available
            // Note: This assumes directional light at index 0
        }

        lightDir = glm::normalize(lightDir);

        // Light view matrix
        glm::vec3 lightPos = -lightDir * (farPlane * 0.5f); // Position light far away
        glm::mat4 lightView = glm::lookAt(lightPos, glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

        // Orthographic projection for directional light
        glm::mat4 lightProjection = glm::ortho(
            -orthoSize, orthoSize,
            -orthoSize, orthoSize,
            nearPlane, farPlane
        );

        return lightProjection * lightView;
    }

    void renderSceneDepth(RenderContext& ctx, Shader* shader) {
        // Simplified depth-only rendering
        // A full implementation would:
        // 1. Iterate through all nodes in the scene
        // 2. Check if node casts shadows
        // 3. Render geometry with only depth (no color)

        // For now, we rely on SceneRenderer or similar
        // This is a placeholder that would need integration with scene graph
        (void)ctx;
        (void)shader;
    }

    static const char* getShadowVertexShader() {
        return R"(
#version 410 core

layout (location = 0) in vec3 aPosition;

uniform mat4 uLightSpaceMatrix;
uniform mat4 uModel;

void main() {
    gl_Position = uLightSpaceMatrix * uModel * vec4(aPosition, 1.0);
}
)";
    }

    static const char* getShadowFragmentShader() {
        return R"(
#version 410 core

void main() {
    // Depth is written automatically
    // gl_FragDepth = gl_FragCoord.z;
}
)";
    }

    UNIQUE<Shader> m_shadowShader;
    glm::mat4 m_lightSpaceMatrix = glm::mat4(1.0f);
};

} // namespace Pina
