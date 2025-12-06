#pragma once

/// Pina Engine - Shadow Pass
/// Renders shadow depth maps from light perspective

#include "../RenderPass.h"
#include "../RenderContext.h"
#include "../Framebuffer.h"
#include "../Shader.h"
#include "../../Core/Memory.h"
#include "../../Scene/Scene.h"
#include "../../Scene/Node.h"
#include "../Model.h"
#include "../Lighting/DirectionalLight.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <cmath>

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
        glm::mat4 lightSpaceMatrix = calculateLightSpaceMatrix(ctx);
        shader->setMat4("uLightSpaceMatrix", lightSpaceMatrix);

        // Store light space matrix for scene pass to use
        m_lightSpaceMatrix = lightSpaceMatrix;

        // Also store in LightManager so ScenePass can access it
        if (ctx.lights) {
            ctx.lights->setLightSpaceMatrix(m_lightSpaceMatrix);
        }

        // Render scene from light's perspective
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
        // Get the first shadow-casting directional light
        glm::vec3 lightDir = glm::vec3(-0.5f, -1.0f, -0.3f); // Default
        float currentOrthoSize = orthoSize;
        float currentNearPlane = nearPlane;
        float currentFarPlane = farPlane;

        // Try to get direction and settings from shadow-casting directional light
        if (ctx.lights) {
            DirectionalLight* shadowLight = ctx.lights->getShadowCastingLight();
            if (shadowLight) {
                Vector3 dir = shadowLight->getDirection();
                lightDir = glm::vec3(dir.x, dir.y, dir.z);

                // Use light's shadow configuration
                currentOrthoSize = shadowLight->getShadowOrthoSize();
                currentNearPlane = shadowLight->getShadowNearPlane();
                currentFarPlane = shadowLight->getShadowFarPlane();
            }
        }

        lightDir = glm::normalize(lightDir);

        // Position light far away in opposite direction
        glm::vec3 lightPos = -lightDir * (currentFarPlane * 0.5f);

        // Determine up vector (avoid parallel to lightDir)
        glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
        if (std::abs(glm::dot(lightDir, up)) > 0.99f) {
            up = glm::vec3(1.0f, 0.0f, 0.0f);
        }

        // Light view matrix
        glm::mat4 lightView = glm::lookAt(lightPos, glm::vec3(0.0f), up);

        // Orthographic projection for directional light
        glm::mat4 lightProjection = glm::ortho(
            -currentOrthoSize, currentOrthoSize,
            -currentOrthoSize, currentOrthoSize,
            currentNearPlane, currentFarPlane
        );

        return lightProjection * lightView;
    }

    void renderSceneDepth(RenderContext& ctx, Shader* shader) {
        if (!ctx.scene) return;

        Node* root = ctx.scene->getRoot();
        if (!root) return;

        renderNodeDepthRecursive(root, shader);
    }

    void renderNodeDepthRecursive(Node* node, Shader* shader) {
        if (!node || !node->isEnabled()) return;

        // Only render nodes that cast shadows
        if (node->getCastsShadow()) {
            // Render model if present
            if (node->hasModel()) {
                Model* model = node->getModel();
                const glm::mat4& worldMatrix = node->getTransform().getWorldMatrix();
                shader->setMat4("uModel", worldMatrix);

                // Draw all meshes (depth only, no material needed)
                for (size_t i = 0; i < model->getMeshCount(); ++i) {
                    StaticMesh* mesh = model->getMesh(i);
                    if (mesh) {
                        mesh->draw();
                    }
                }
            }

            // Render static mesh if present
            if (node->hasMesh()) {
                const glm::mat4& worldMatrix = node->getTransform().getWorldMatrix();
                shader->setMat4("uModel", worldMatrix);
                node->getMesh()->draw();
            }
        }

        // Recursively process children
        for (size_t i = 0; i < node->getChildCount(); ++i) {
            renderNodeDepthRecursive(node->getChild(i), shader);
        }
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
