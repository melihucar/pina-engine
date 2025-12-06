#pragma once

/// Pina Engine - Bloom Pass
/// Multi-pass bloom effect with threshold, blur, and composite

#include "../RenderPass.h"
#include "../RenderContext.h"
#include "../Framebuffer.h"
#include "../Shader.h"
#include "../GraphicsDevice.h"
#include "../../Core/Memory.h"
#include <glm/glm.hpp>
#include <iostream>

namespace Pina {

/// Bloom post-processing effect
/// Uses threshold extraction, Gaussian blur, and additive blending
class PINA_API BloomPass : public RenderPass {
public:
    BloomPass() {
        name = "bloom";
        needsSwap = true;
    }

    void initialize(RenderContext& ctx) override {
        if (!ctx.device) return;

        // Create shaders
        m_thresholdShader = ctx.device->createShader();
        if (!m_thresholdShader->load(getFullscreenVertexShader(), getThresholdFragmentShader())) {
            std::cerr << "BloomPass: Failed to create threshold shader" << std::endl;
        }

        m_blurShader = ctx.device->createShader();
        if (!m_blurShader->load(getFullscreenVertexShader(), getBlurFragmentShader())) {
            std::cerr << "BloomPass: Failed to create blur shader" << std::endl;
        }

        m_compositeShader = ctx.device->createShader();
        if (!m_compositeShader->load(getFullscreenVertexShader(), getCompositeFragmentShader())) {
            std::cerr << "BloomPass: Failed to create composite shader" << std::endl;
        }

        // Create blur framebuffers (half resolution for performance)
        createBlurBuffers(ctx, ctx.viewportWidth / 2, ctx.viewportHeight / 2);
    }

    void execute(RenderContext& ctx) override {
        if (!m_thresholdShader || !m_blurShader || !m_compositeShader) {
            return;
        }
        if (!m_blurFB1 || !m_blurFB2) {
            return;
        }

        // Step 1: Extract bright areas (threshold)
        m_blurFB1->bind();
        m_thresholdShader->bind();
        m_thresholdShader->setInt("uInputTexture", 0);
        m_thresholdShader->setFloat("uThreshold", threshold);
        m_thresholdShader->setFloat("uSoftThreshold", softThreshold);
        // Bind input texture from read buffer
        if (ctx.drawFullscreenQuad) {
            ctx.drawFullscreenQuad();
        }

        // Step 2: Gaussian blur (ping-pong between blur buffers)
        for (int i = 0; i < blurIterations; ++i) {
            // Horizontal blur
            m_blurFB2->bind();
            m_blurShader->bind();
            m_blurShader->setInt("uInputTexture", 0);
            m_blurShader->setVec2("uDirection", glm::vec2(1.0f, 0.0f));
            m_blurShader->setFloat("uBlurSize", blurSize);
            if (ctx.drawFullscreenQuad) {
                ctx.drawFullscreenQuad();
            }

            // Vertical blur
            m_blurFB1->bind();
            m_blurShader->bind();
            m_blurShader->setInt("uInputTexture", 0);
            m_blurShader->setVec2("uDirection", glm::vec2(0.0f, 1.0f));
            m_blurShader->setFloat("uBlurSize", blurSize);
            if (ctx.drawFullscreenQuad) {
                ctx.drawFullscreenQuad();
            }
        }

        // Step 3: Composite bloom with original scene
        bindOutput(ctx);
        m_compositeShader->bind();
        m_compositeShader->setInt("uSceneTexture", 0);
        m_compositeShader->setInt("uBloomTexture", 1);
        m_compositeShader->setFloat("uBloomIntensity", intensity);
        if (ctx.drawFullscreenQuad) {
            ctx.drawFullscreenQuad();
        }
    }

    void resize(int width, int height) override {
        // Blur buffers are half resolution
        if (m_blurFB1) m_blurFB1->resize(width / 2, height / 2);
        if (m_blurFB2) m_blurFB2->resize(width / 2, height / 2);
    }

    // ========================================================================
    // Configuration
    // ========================================================================

    /// Brightness threshold for bloom extraction (default: 1.0 for HDR)
    float threshold = 1.0f;

    /// Soft threshold knee (smoother transition)
    float softThreshold = 0.5f;

    /// Number of blur passes (more = softer bloom)
    int blurIterations = 4;

    /// Blur kernel size
    float blurSize = 1.0f;

    /// Final bloom intensity multiplier
    float intensity = 1.0f;

private:
    void createBlurBuffers(RenderContext& ctx, int width, int height) {
        if (!ctx.device) return;

        FramebufferSpec spec;
        spec.width = width;
        spec.height = height;
        spec.colorAttachments = { TextureFormat::RGBA16F };  // HDR
        spec.depthAttachment = TextureFormat::None;

        m_blurFB1 = ctx.device->createFramebuffer(spec);
        m_blurFB2 = ctx.device->createFramebuffer(spec);
    }

    static const char* getFullscreenVertexShader() {
        return R"(
#version 410 core

layout (location = 0) in vec2 aPosition;
layout (location = 1) in vec2 aTexCoord;

out vec2 vTexCoord;

void main() {
    vTexCoord = aTexCoord;
    gl_Position = vec4(aPosition, 0.0, 1.0);
}
)";
    }

    static const char* getThresholdFragmentShader() {
        return R"(
#version 410 core

in vec2 vTexCoord;
out vec4 FragColor;

uniform sampler2D uInputTexture;
uniform float uThreshold;
uniform float uSoftThreshold;

void main() {
    vec4 color = texture(uInputTexture, vTexCoord);

    // Calculate luminance
    float brightness = dot(color.rgb, vec3(0.2126, 0.7152, 0.0722));

    // Soft threshold
    float soft = brightness - uThreshold + uSoftThreshold;
    soft = clamp(soft / (2.0 * uSoftThreshold + 0.00001), 0.0, 1.0);
    soft = soft * soft;

    // Hard threshold
    float contribution = max(soft, step(uThreshold, brightness));

    FragColor = color * contribution;
}
)";
    }

    static const char* getBlurFragmentShader() {
        return R"(
#version 410 core

in vec2 vTexCoord;
out vec4 FragColor;

uniform sampler2D uInputTexture;
uniform vec2 uDirection;
uniform float uBlurSize;

// 9-tap Gaussian weights
const float weights[5] = float[](0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216);

void main() {
    vec2 texelSize = 1.0 / textureSize(uInputTexture, 0);
    vec3 result = texture(uInputTexture, vTexCoord).rgb * weights[0];

    for (int i = 1; i < 5; ++i) {
        vec2 offset = uDirection * texelSize * float(i) * uBlurSize;
        result += texture(uInputTexture, vTexCoord + offset).rgb * weights[i];
        result += texture(uInputTexture, vTexCoord - offset).rgb * weights[i];
    }

    FragColor = vec4(result, 1.0);
}
)";
    }

    static const char* getCompositeFragmentShader() {
        return R"(
#version 410 core

in vec2 vTexCoord;
out vec4 FragColor;

uniform sampler2D uSceneTexture;
uniform sampler2D uBloomTexture;
uniform float uBloomIntensity;

void main() {
    vec3 scene = texture(uSceneTexture, vTexCoord).rgb;
    vec3 bloom = texture(uBloomTexture, vTexCoord).rgb;

    FragColor = vec4(scene + bloom * uBloomIntensity, 1.0);
}
)";
    }

    UNIQUE<Shader> m_thresholdShader;
    UNIQUE<Shader> m_blurShader;
    UNIQUE<Shader> m_compositeShader;
    UNIQUE<Framebuffer> m_blurFB1;
    UNIQUE<Framebuffer> m_blurFB2;
};

} // namespace Pina
