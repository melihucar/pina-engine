/// Pina Engine - Render Pipeline Implementation

#include "RenderPipeline.h"
#include "Passes/ClearPass.h"
#include "Passes/ScenePass.h"
#include "Passes/ShadowPass.h"
#include "Passes/BloomPass.h"
#include "Passes/ToneMappingPass.h"
#include "Passes/FXAAPass.h"
#include "Shaders/ShaderLibrary.h"
#include <iostream>

namespace Pina {

// ========================================================================
// Lifecycle
// ========================================================================

RenderPipeline::RenderPipeline(GraphicsDevice* device)
    : m_device(device)
{
    if (!device) {
        std::cerr << "RenderPipeline: null device" << std::endl;
        return;
    }

    m_compositor = MAKE_UNIQUE<RenderCompositor>(device);

    createDefaultShaders();
    createDefaultPasses();
}

RenderPipeline::~RenderPipeline() = default;

void RenderPipeline::createDefaultShaders() {
    if (!m_device) return;

    // Create standard Blinn-Phong shader
    m_standardShader = m_device->createShader();
    if (m_standardShader) {
        if (!m_standardShader->load(
            ShaderLibrary::getStandardVertexShader(),
            ShaderLibrary::getStandardFragmentShader())) {
            std::cerr << "RenderPipeline: Failed to create standard shader" << std::endl;
        }
    }

    // Create PBR shader
    m_pbrShader = m_device->createShader();
    if (m_pbrShader) {
        if (!m_pbrShader->load(
            ShaderLibrary::getPBRVertexShader(),
            ShaderLibrary::getPBRFragmentShader())) {
            std::cerr << "RenderPipeline: Failed to create PBR shader" << std::endl;
        }
    }

    // Shadow shader is created by ShadowPass itself
}

void RenderPipeline::createDefaultPasses() {
    if (!m_compositor) return;

    // Create shadow map render target
    FramebufferSpec shadowSpec;
    shadowSpec.width = 2048;
    shadowSpec.height = 2048;
    shadowSpec.colorAttachments = {};  // Depth only
    shadowSpec.depthAttachment = TextureFormat::Depth32F;
    m_compositor->createRenderTarget("shadowMap", shadowSpec);

    // Pass 1: Shadow map (disabled by default)
    auto shadowPass = MAKE_UNIQUE<ShadowPass>();
    shadowPass->enabled = false;  // Disabled by default
    m_shadowPass = shadowPass.get();
    m_compositor->addPass(std::move(shadowPass));

    // Pass 2: Clear
    auto clearPass = MAKE_UNIQUE<ClearPass>();
    m_clearPass = clearPass.get();
    m_compositor->addPass(std::move(clearPass));

    // Pass 3: Scene rendering
    auto scenePass = MAKE_UNIQUE<ScenePass>();
    m_scenePass = scenePass.get();
    m_compositor->addPass(std::move(scenePass));

    // Pass 4: Bloom (disabled by default)
    auto bloomPass = MAKE_UNIQUE<BloomPass>();
    bloomPass->enabled = false;
    m_bloomPass = bloomPass.get();
    m_compositor->addPass(std::move(bloomPass));

    // Pass 5: Tone mapping (disabled by default, enable for HDR)
    auto toneMappingPass = MAKE_UNIQUE<ToneMappingPass>();
    toneMappingPass->enabled = false;
    m_toneMappingPass = toneMappingPass.get();
    m_compositor->addPass(std::move(toneMappingPass));

    // Pass 6: FXAA (disabled by default)
    auto fxaaPass = MAKE_UNIQUE<FXAAPass>();
    fxaaPass->enabled = false;
    m_fxaaPass = fxaaPass.get();
    m_compositor->addPass(std::move(fxaaPass));
}

// ========================================================================
// Rendering
// ========================================================================

void RenderPipeline::render(Scene* scene, Camera* camera, float deltaTime) {
    if (!m_compositor) return;

    // Set up shaders in render context
    // The compositor will pass these to the scene pass
    m_compositor->render(scene, camera, deltaTime,
                         m_standardShader.get(),
                         m_pbrShader.get(),
                         m_shadowShader.get());
}

void RenderPipeline::resize(int width, int height) {
    if (m_compositor) {
        m_compositor->resize(width, height);
    }
}

// ========================================================================
// Quick Configuration
// ========================================================================

void RenderPipeline::setClearColor(const Color& color) {
    if (m_clearPass) {
        m_clearPass->clearColor = color;
    }
    if (m_scenePass) {
        m_scenePass->clearColor = color;
    }
}

Color RenderPipeline::getClearColor() const {
    if (m_clearPass) {
        return m_clearPass->clearColor;
    }
    return Color(0.1f, 0.1f, 0.12f);
}

void RenderPipeline::setShadowsEnabled(bool enabled) {
    if (m_shadowPass) {
        m_shadowPass->enabled = enabled;
    }
    if (m_scenePass) {
        m_scenePass->enableShadows = enabled;
    }
}

bool RenderPipeline::getShadowsEnabled() const {
    return m_shadowPass ? m_shadowPass->enabled : false;
}

void RenderPipeline::setBloomEnabled(bool enabled) {
    if (m_bloomPass) {
        m_bloomPass->enabled = enabled;
    }
    // Auto-enable tone mapping when bloom is enabled (HDR workflow)
    if (enabled && m_toneMappingPass && !m_toneMappingPass->enabled) {
        m_toneMappingPass->enabled = true;
    }
}

bool RenderPipeline::getBloomEnabled() const {
    return m_bloomPass ? m_bloomPass->enabled : false;
}

void RenderPipeline::setBloomThreshold(float threshold) {
    if (m_bloomPass) {
        m_bloomPass->threshold = threshold;
    }
}

void RenderPipeline::setBloomIntensity(float intensity) {
    if (m_bloomPass) {
        m_bloomPass->intensity = intensity;
    }
}

void RenderPipeline::setToneMappingEnabled(bool enabled) {
    if (m_toneMappingPass) {
        m_toneMappingPass->enabled = enabled;
    }
}

bool RenderPipeline::getToneMappingEnabled() const {
    return m_toneMappingPass ? m_toneMappingPass->enabled : false;
}

void RenderPipeline::setExposure(float exposure) {
    if (m_toneMappingPass) {
        m_toneMappingPass->exposure = exposure;
    }
}

float RenderPipeline::getExposure() const {
    return m_toneMappingPass ? m_toneMappingPass->exposure : 1.0f;
}

void RenderPipeline::setFXAAEnabled(bool enabled) {
    if (m_fxaaPass) {
        m_fxaaPass->enabled = enabled;
    }
}

bool RenderPipeline::getFXAAEnabled() const {
    return m_fxaaPass ? m_fxaaPass->enabled : false;
}

void RenderPipeline::setWireframe(bool enabled) {
    if (m_scenePass) {
        m_scenePass->wireframe = enabled;
    }
}

bool RenderPipeline::getWireframe() const {
    return m_scenePass ? m_scenePass->wireframe : false;
}

void RenderPipeline::setPBREnabled(bool enabled) {
    if (m_scenePass) {
        m_scenePass->usePBR = enabled;
    }
}

bool RenderPipeline::getPBREnabled() const {
    return m_scenePass ? m_scenePass->usePBR : false;
}

// ========================================================================
// Pass Access
// ========================================================================

ClearPass* RenderPipeline::getClearPass() {
    return m_clearPass;
}

ScenePass* RenderPipeline::getScenePass() {
    return m_scenePass;
}

ShadowPass* RenderPipeline::getShadowPass() {
    return m_shadowPass;
}

BloomPass* RenderPipeline::getBloomPass() {
    return m_bloomPass;
}

ToneMappingPass* RenderPipeline::getToneMappingPass() {
    return m_toneMappingPass;
}

FXAAPass* RenderPipeline::getFXAAPass() {
    return m_fxaaPass;
}

} // namespace Pina
