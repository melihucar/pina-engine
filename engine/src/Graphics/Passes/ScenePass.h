#pragma once

/// Pina Engine - Scene Pass
/// Renders the 3D scene (opaque + transparent objects)

#include "../RenderPass.h"
#include "../RenderContext.h"
#include "../GraphicsDevice.h"
#include "../Shader.h"
#include "../Camera.h"
#include "../Lighting/LightManager.h"
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

        // Upload shadow map if enabled
        if (enableShadows && !shadowMapInput.empty()) {
            uint32_t shadowMapID = ctx.getDepthTextureID(shadowMapInput);
            if (shadowMapID != 0) {
                // Bind shadow map to texture unit 8 (arbitrary choice)
                // The shader should sample from this
                shader->setInt("uShadowMap", 8);
                // Note: actual binding would need OpenGL calls
                // This is a simplified version
            }
        }

        // Set wireframe mode
        ctx.device->setWireframe(wireframe);

        // Render scene
        if (m_sceneRenderer) {
            // Two-pass rendering for transparency
            // Pass 1: Opaque objects
            ctx.device->setBlending(false);
            ctx.device->setDepthWrite(true);
            m_sceneRenderer->render(ctx.scene, shader);

            // Pass 2: Transparent objects (if enabled)
            if (enableTransparency) {
                ctx.device->setBlending(true);
                ctx.device->setDepthWrite(false);
                // Note: SceneRenderer would need to support separate
                // opaque/transparent rendering. For now, we just render all.
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
