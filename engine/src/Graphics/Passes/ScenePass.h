#pragma once

/// Pina Engine - Scene Pass
/// Renders the 3D scene (opaque + transparent objects)

#include "../RenderPass.h"
#include "../RenderContext.h"
#include "../GraphicsDevice.h"
#include "../Shader.h"
#include "../Camera.h"
#include "../Lighting/LightManager.h"
#include "../Lighting/DirectionalLight.h"
#include "../../Scene/Scene.h"
#include "../../Scene/SceneRenderer.h"

namespace Pina {

/// Pass that renders the 3D scene
class PINA_API ScenePass : public RenderPass {
public:
    ScenePass() {
        name = "scene";
        needsSwap = false;  // First pass typically writes to ping buffer
        clear = true;
        clearColor = Color(0.1f, 0.1f, 0.12f);
        clearDepth = true;
    }

    void initialize(RenderContext& ctx) override {
        m_sceneRenderer = MAKE_UNIQUE<SceneRenderer>(ctx.device);
    }

    void execute(RenderContext& ctx) override {
        if (!ctx.scene || !ctx.camera) {
            return;
        }

        // Bind output target
        bindOutput(ctx);

        // Select shader based on material type
        Shader* shader = usePBR ? ctx.pbrShader : ctx.standardShader;
        if (!shader) {
            return;
        }

        shader->bind();

        // Upload camera matrices
        shader->setMat4("uView", ctx.camera->getViewMatrix());
        shader->setMat4("uProjection", ctx.camera->getProjectionMatrix());

        // Upload lighting
        if (ctx.lights) {
            ctx.lights->setViewPosition(ctx.camera->getPosition());
            ctx.lights->uploadToShader(shader);
        }

        // Upload shadow map and uniforms if enabled
        if (enableShadows && !shadowMapInput.empty() && ctx.lights) {
            uint32_t shadowMapID = ctx.getDepthTextureID(shadowMapInput);
            if (shadowMapID != 0) {
                // Upload light space matrix and bind shadow map
                ctx.lights->uploadShadowUniforms(shader, shadowMapID);
                shader->setInt("uEnableShadows", 1);

                // Upload shadow parameters from light
                DirectionalLight* shadowLight = ctx.lights->getShadowCastingLight();
                if (shadowLight) {
                    shader->setFloat("uShadowBias", shadowLight->getShadowBias());
                    shader->setFloat("uShadowNormalBias", shadowLight->getShadowNormalBias());
                    shader->setFloat("uShadowSoftness", shadowLight->getShadowSoftness());
                } else {
                    shader->setFloat("uShadowBias", 0.005f);
                    shader->setFloat("uShadowNormalBias", 0.02f);
                    shader->setFloat("uShadowSoftness", 1.5f);
                }
            } else {
                shader->setInt("uEnableShadows", 0);
            }
        } else {
            shader->setInt("uEnableShadows", 0);
        }

        // Set wireframe mode
        ctx.device->setWireframe(wireframe);

        // Render scene with two-pass rendering for proper transparency
        if (m_sceneRenderer) {
            // Pass 1: Opaque objects
            ctx.device->setBlending(false);
            ctx.device->setDepthWrite(true);
            m_sceneRenderer->renderOpaque(ctx.scene, shader);

            // Pass 2: Transparent objects (if enabled)
            if (enableTransparency) {
                ctx.device->setBlending(true);
                ctx.device->setDepthWrite(false);
                m_sceneRenderer->renderTransparent(ctx.scene, shader);
                ctx.device->setDepthWrite(true);
                ctx.device->setBlending(false);
            }
        }

        // Reset wireframe
        ctx.device->setWireframe(false);
    }

    // ========================================================================
    // Configuration
    // ========================================================================

    /// Whether shadows are enabled
    bool enableShadows = false;

    /// Name of the shadow map render target
    std::string shadowMapInput = "shadowMap";

    /// Whether to enable transparency rendering
    bool enableTransparency = true;

    /// Whether to use PBR shader (vs standard Blinn-Phong)
    bool usePBR = false;

    /// Wireframe rendering mode
    bool wireframe = false;

private:
    UNIQUE<SceneRenderer> m_sceneRenderer;
};

} // namespace Pina
