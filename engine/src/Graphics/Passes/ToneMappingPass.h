#pragma once

/// Pina Engine - Tone Mapping Pass
/// Converts HDR colors to LDR with various tone mapping operators

#include "ShaderPass.h"
#include <iostream>

namespace Pina {

/// Tone mapping algorithms
enum class ToneMapOperator {
    Linear,         // Simple exposure adjustment
    Reinhard,       // Basic Reinhard
    ReinhardExt,    // Extended Reinhard with white point
    ACES,           // Academy Color Encoding System
    Uncharted2,     // Filmic curve from Uncharted 2
    Exposure        // Simple exposure with gamma
};

/// Tone mapping pass for HDR to LDR conversion
class PINA_API ToneMappingPass : public RenderPass {
public:
    ToneMappingPass() {
        name = "tonemapping";
        needsSwap = true;
    }

    void initialize(RenderContext& ctx) override {
        if (!ctx.device) return;

        m_shader = ctx.device->createShader();
        if (!m_shader->load(getVertexShader(), getFragmentShader())) {
            std::cerr << "ToneMappingPass: Failed to create shader" << std::endl;
        }
    }

    void execute(RenderContext& ctx) override {
        if (!m_shader) return;

        bindOutput(ctx);

        m_shader->bind();
        m_shader->setInt("uInputTexture", 0);
        m_shader->setInt("uOperator", static_cast<int>(toneMapOperator));
        m_shader->setFloat("uExposure", exposure);
        m_shader->setFloat("uGamma", gamma);
        m_shader->setFloat("uWhitePoint", whitePoint);

        if (ctx.drawFullscreenQuad) {
            ctx.drawFullscreenQuad();
        }
    }

    // ========================================================================
    // Configuration
    // ========================================================================

    /// Tone mapping algorithm to use
    ToneMapOperator toneMapOperator = ToneMapOperator::ACES;

    /// Exposure adjustment (applied before tone mapping)
    float exposure = 1.0f;

    /// Gamma correction value
    float gamma = 2.2f;

    /// White point for extended Reinhard
    float whitePoint = 4.0f;

private:
    static const char* getVertexShader() {
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

    static const char* getFragmentShader() {
        return R"(
#version 410 core

in vec2 vTexCoord;
out vec4 FragColor;

uniform sampler2D uInputTexture;
uniform int uOperator;
uniform float uExposure;
uniform float uGamma;
uniform float uWhitePoint;

// Simple linear exposure
vec3 linearToneMap(vec3 color) {
    return clamp(color * uExposure, 0.0, 1.0);
}

// Basic Reinhard
vec3 reinhardToneMap(vec3 color) {
    color *= uExposure;
    return color / (color + vec3(1.0));
}

// Extended Reinhard with white point
vec3 reinhardExtToneMap(vec3 color) {
    color *= uExposure;
    float white2 = uWhitePoint * uWhitePoint;
    return (color * (1.0 + color / white2)) / (1.0 + color);
}

// ACES filmic curve
vec3 acesToneMap(vec3 color) {
    color *= uExposure;
    const float a = 2.51;
    const float b = 0.03;
    const float c = 2.43;
    const float d = 0.59;
    const float e = 0.14;
    return clamp((color * (a * color + b)) / (color * (c * color + d) + e), 0.0, 1.0);
}

// Uncharted 2 filmic curve
vec3 uncharted2Partial(vec3 x) {
    const float A = 0.15;
    const float B = 0.50;
    const float C = 0.10;
    const float D = 0.20;
    const float E = 0.02;
    const float F = 0.30;
    return ((x * (A * x + C * B) + D * E) / (x * (A * x + B) + D * F)) - E / F;
}

vec3 uncharted2ToneMap(vec3 color) {
    color *= uExposure;
    const float W = 11.2;
    vec3 curr = uncharted2Partial(color * 2.0);
    vec3 whiteScale = 1.0 / uncharted2Partial(vec3(W));
    return curr * whiteScale;
}

// Simple exposure with gamma
vec3 exposureToneMap(vec3 color) {
    return vec3(1.0) - exp(-color * uExposure);
}

void main() {
    vec3 hdrColor = texture(uInputTexture, vTexCoord).rgb;
    vec3 mapped;

    // Apply selected tone mapping operator
    if (uOperator == 0) {
        mapped = linearToneMap(hdrColor);
    } else if (uOperator == 1) {
        mapped = reinhardToneMap(hdrColor);
    } else if (uOperator == 2) {
        mapped = reinhardExtToneMap(hdrColor);
    } else if (uOperator == 3) {
        mapped = acesToneMap(hdrColor);
    } else if (uOperator == 4) {
        mapped = uncharted2ToneMap(hdrColor);
    } else {
        mapped = exposureToneMap(hdrColor);
    }

    // Gamma correction
    mapped = pow(mapped, vec3(1.0 / uGamma));

    FragColor = vec4(mapped, 1.0);
}
)";
    }

    UNIQUE<Shader> m_shader;
};

} // namespace Pina
