#pragma once

/// Pina Engine - Render Pipeline
/// High-level rendering coordinator with default pass configuration

#include "RenderCompositor.h"
#include "Shader.h"
#include "GraphicsDevice.h"
#include "../Core/Memory.h"
#include "../Math/Color.h"
#include <string>

namespace Pina {

// Forward declarations
class Scene;
class Camera;
class ClearPass;
class ScenePass;
class ShadowPass;
class BloomPass;
class ToneMappingPass;
class FXAAPass;

/// High-level rendering pipeline with sensible defaults
/// Provides simple API for common rendering tasks
class PINA_API RenderPipeline {
public:
    explicit RenderPipeline(GraphicsDevice* device);
    ~RenderPipeline();

    // ========================================================================
    // Rendering
    // ========================================================================

    /// Render scene with camera using the configured pass chain
    void render(Scene* scene, Camera* camera, float deltaTime);

    /// Handle viewport resize
    void resize(int width, int height);

    // ========================================================================
    // Quick Configuration
    // ========================================================================

    /// Set the clear color for the scene
    void setClearColor(const Color& color);

    /// Get current clear color
    Color getClearColor() const;

    /// Enable/disable shadow rendering
    void setShadowsEnabled(bool enabled);
    bool getShadowsEnabled() const;

    /// Enable/disable bloom post-processing
    void setBloomEnabled(bool enabled);
    bool getBloomEnabled() const;

    /// Configure bloom parameters
    void setBloomThreshold(float threshold);
    void setBloomIntensity(float intensity);

    /// Enable/disable tone mapping
    void setToneMappingEnabled(bool enabled);
    bool getToneMappingEnabled() const;

    /// Set exposure for tone mapping
    void setExposure(float exposure);
    float getExposure() const;

    /// Enable/disable FXAA anti-aliasing
    void setFXAAEnabled(bool enabled);
    bool getFXAAEnabled() const;

    /// Enable/disable wireframe mode
    void setWireframe(bool enabled);
    bool getWireframe() const;

    /// Enable/disable PBR shading (vs standard Blinn-Phong)
    void setPBREnabled(bool enabled);
    bool getPBREnabled() const;

    // ========================================================================
    // Advanced Access
    // ========================================================================

    /// Get the underlying compositor for advanced pass manipulation
    RenderCompositor* getCompositor() { return m_compositor.get(); }
    const RenderCompositor* getCompositor() const { return m_compositor.get(); }

    /// Get the graphics device
    GraphicsDevice* getDevice() { return m_device; }

    /// Get built-in shaders
    Shader* getStandardShader() { return m_standardShader.get(); }
    Shader* getPBRShader() { return m_pbrShader.get(); }
    Shader* getShadowShader() { return m_shadowShader.get(); }

    // ========================================================================
    // Pass Access
    // ========================================================================

    /// Get specific passes for direct configuration
    ClearPass* getClearPass();
    ScenePass* getScenePass();
    ShadowPass* getShadowPass();
    BloomPass* getBloomPass();
    ToneMappingPass* getToneMappingPass();
    FXAAPass* getFXAAPass();

private:
    void createDefaultPasses();
    void createDefaultShaders();

    GraphicsDevice* m_device = nullptr;
    UNIQUE<RenderCompositor> m_compositor;

    // Default shaders
    UNIQUE<Shader> m_standardShader;
    UNIQUE<Shader> m_pbrShader;
    UNIQUE<Shader> m_shadowShader;

    // Cached pass pointers (owned by compositor)
    ClearPass* m_clearPass = nullptr;
    ScenePass* m_scenePass = nullptr;
    ShadowPass* m_shadowPass = nullptr;
    BloomPass* m_bloomPass = nullptr;
    ToneMappingPass* m_toneMappingPass = nullptr;
    FXAAPass* m_fxaaPass = nullptr;
};

} // namespace Pina
