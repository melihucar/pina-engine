#pragma once

/// Pina Engine - FXAA Pass
/// Fast Approximate Anti-Aliasing post-processing effect

#include "../RenderPass.h"
#include "../RenderContext.h"
#include "../Shader.h"
#include "../GraphicsDevice.h"
#include "../../Core/Memory.h"
#include <iostream>

namespace Pina {

/// FXAA quality presets
enum class FXAAQuality {
    Low,      // Faster, lower quality
    Medium,   // Balanced
    High,     // Best quality
    Ultra     // Maximum quality, slower
};

/// Fast Approximate Anti-Aliasing pass
class PINA_API FXAAPass : public RenderPass {
public:
    FXAAPass() {
        name = "fxaa";
        needsSwap = true;
    }

    void initialize(RenderContext& ctx) override {
        if (!ctx.device) return;

        m_shader = ctx.device->createShader();
        if (!m_shader->load(getVertexShader(), getFragmentShader())) {
            std::cerr << "FXAAPass: Failed to create shader" << std::endl;
        }
    }

    void execute(RenderContext& ctx) override {
        if (!m_shader) return;

        bindOutput(ctx);

        m_shader->bind();
        m_shader->setInt("uInputTexture", 0);
        m_shader->setVec2("uTexelSize", glm::vec2(1.0f / ctx.viewportWidth, 1.0f / ctx.viewportHeight));

        // Quality settings
        float subpixel, edgeThreshold, edgeThresholdMin;
        getQualitySettings(quality, subpixel, edgeThreshold, edgeThresholdMin);

        m_shader->setFloat("uSubpixel", subpixel);
        m_shader->setFloat("uEdgeThreshold", edgeThreshold);
        m_shader->setFloat("uEdgeThresholdMin", edgeThresholdMin);

        if (ctx.drawFullscreenQuad) {
            ctx.drawFullscreenQuad();
        }
    }

    // ========================================================================
    // Configuration
    // ========================================================================

    /// FXAA quality preset
    FXAAQuality quality = FXAAQuality::Medium;

private:
    void getQualitySettings(FXAAQuality q, float& subpixel, float& edgeThreshold, float& edgeThresholdMin) {
        switch (q) {
            case FXAAQuality::Low:
                subpixel = 0.25f;
                edgeThreshold = 0.250f;
                edgeThresholdMin = 0.0833f;
                break;
            case FXAAQuality::Medium:
                subpixel = 0.50f;
                edgeThreshold = 0.166f;
                edgeThresholdMin = 0.0625f;
                break;
            case FXAAQuality::High:
                subpixel = 0.75f;
                edgeThreshold = 0.125f;
                edgeThresholdMin = 0.0312f;
                break;
            case FXAAQuality::Ultra:
                subpixel = 1.00f;
                edgeThreshold = 0.063f;
                edgeThresholdMin = 0.0156f;
                break;
        }
    }

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
uniform vec2 uTexelSize;
uniform float uSubpixel;
uniform float uEdgeThreshold;
uniform float uEdgeThresholdMin;

// FXAA 3.11 implementation
// Based on NVIDIA FXAA by Timothy Lottes

float luminance(vec3 color) {
    return dot(color, vec3(0.299, 0.587, 0.114));
}

void main() {
    // Sample center and neighbors
    vec3 colorCenter = texture(uInputTexture, vTexCoord).rgb;

    float lumaCenter = luminance(colorCenter);
    float lumaN = luminance(textureOffset(uInputTexture, vTexCoord, ivec2(0, 1)).rgb);
    float lumaS = luminance(textureOffset(uInputTexture, vTexCoord, ivec2(0, -1)).rgb);
    float lumaE = luminance(textureOffset(uInputTexture, vTexCoord, ivec2(1, 0)).rgb);
    float lumaW = luminance(textureOffset(uInputTexture, vTexCoord, ivec2(-1, 0)).rgb);

    // Find min/max luma
    float lumaMin = min(lumaCenter, min(min(lumaN, lumaS), min(lumaE, lumaW)));
    float lumaMax = max(lumaCenter, max(max(lumaN, lumaS), max(lumaE, lumaW)));
    float lumaRange = lumaMax - lumaMin;

    // Skip pixels with low contrast
    if (lumaRange < max(uEdgeThresholdMin, lumaMax * uEdgeThreshold)) {
        FragColor = vec4(colorCenter, 1.0);
        return;
    }

    // Sample corners
    float lumaNW = luminance(textureOffset(uInputTexture, vTexCoord, ivec2(-1, 1)).rgb);
    float lumaNE = luminance(textureOffset(uInputTexture, vTexCoord, ivec2(1, 1)).rgb);
    float lumaSW = luminance(textureOffset(uInputTexture, vTexCoord, ivec2(-1, -1)).rgb);
    float lumaSE = luminance(textureOffset(uInputTexture, vTexCoord, ivec2(1, -1)).rgb);

    // Compute edge direction
    float lumaWCorners = lumaNW + lumaSW;
    float lumaECorners = lumaNE + lumaSE;
    float lumaNCorners = lumaNW + lumaNE;
    float lumaSCorners = lumaSW + lumaSE;

    float edgeHorizontal = abs(-2.0 * lumaW + lumaWCorners) + abs(-2.0 * lumaCenter + lumaN + lumaS) * 2.0 + abs(-2.0 * lumaE + lumaECorners);
    float edgeVertical = abs(-2.0 * lumaN + lumaNCorners) + abs(-2.0 * lumaCenter + lumaW + lumaE) * 2.0 + abs(-2.0 * lumaS + lumaSCorners);

    bool isHorizontal = edgeHorizontal >= edgeVertical;

    // Select edge endpoints
    float luma1 = isHorizontal ? lumaS : lumaW;
    float luma2 = isHorizontal ? lumaN : lumaE;
    float gradient1 = luma1 - lumaCenter;
    float gradient2 = luma2 - lumaCenter;

    bool is1Steepest = abs(gradient1) >= abs(gradient2);
    float gradientScaled = 0.25 * max(abs(gradient1), abs(gradient2));

    // Choose step direction
    float stepLength = isHorizontal ? uTexelSize.y : uTexelSize.x;
    float lumaLocalAverage = 0.0;

    if (is1Steepest) {
        stepLength = -stepLength;
        lumaLocalAverage = 0.5 * (luma1 + lumaCenter);
    } else {
        lumaLocalAverage = 0.5 * (luma2 + lumaCenter);
    }

    // Shift UV in edge direction
    vec2 currentUV = vTexCoord;
    if (isHorizontal) {
        currentUV.y += stepLength * 0.5;
    } else {
        currentUV.x += stepLength * 0.5;
    }

    // Edge exploration
    vec2 offset = isHorizontal ? vec2(uTexelSize.x, 0.0) : vec2(0.0, uTexelSize.y);
    vec2 uv1 = currentUV - offset;
    vec2 uv2 = currentUV + offset;

    float lumaEnd1 = luminance(texture(uInputTexture, uv1).rgb) - lumaLocalAverage;
    float lumaEnd2 = luminance(texture(uInputTexture, uv2).rgb) - lumaLocalAverage;

    bool reached1 = abs(lumaEnd1) >= gradientScaled;
    bool reached2 = abs(lumaEnd2) >= gradientScaled;
    bool reachedBoth = reached1 && reached2;

    // Continue edge search
    const int ITERATIONS = 12;
    for (int i = 2; i < ITERATIONS && !reachedBoth; i++) {
        if (!reached1) {
            uv1 -= offset;
            lumaEnd1 = luminance(texture(uInputTexture, uv1).rgb) - lumaLocalAverage;
            reached1 = abs(lumaEnd1) >= gradientScaled;
        }
        if (!reached2) {
            uv2 += offset;
            lumaEnd2 = luminance(texture(uInputTexture, uv2).rgb) - lumaLocalAverage;
            reached2 = abs(lumaEnd2) >= gradientScaled;
        }
        reachedBoth = reached1 && reached2;
    }

    // Compute distances
    float distance1 = isHorizontal ? (vTexCoord.x - uv1.x) : (vTexCoord.y - uv1.y);
    float distance2 = isHorizontal ? (uv2.x - vTexCoord.x) : (uv2.y - vTexCoord.y);

    bool isDirection1 = distance1 < distance2;
    float distanceFinal = min(distance1, distance2);
    float edgeLength = distance1 + distance2;

    // Sub-pixel anti-aliasing
    float pixelOffset = -distanceFinal / edgeLength + 0.5;

    // Is the luma at center smaller than the local average?
    bool isLumaCenterSmaller = lumaCenter < lumaLocalAverage;
    bool correctVariation = ((isDirection1 ? lumaEnd1 : lumaEnd2) < 0.0) != isLumaCenterSmaller;
    float finalOffset = correctVariation ? pixelOffset : 0.0;

    // Subpixel aliasing test
    float lumaAverage = (1.0 / 12.0) * (2.0 * (lumaN + lumaS + lumaE + lumaW) + lumaNW + lumaNE + lumaSW + lumaSE);
    float subpixelOffset1 = clamp(abs(lumaAverage - lumaCenter) / lumaRange, 0.0, 1.0);
    float subpixelOffset2 = (-2.0 * subpixelOffset1 + 3.0) * subpixelOffset1 * subpixelOffset1;
    float subpixelOffsetFinal = subpixelOffset2 * subpixelOffset2 * uSubpixel;

    finalOffset = max(finalOffset, subpixelOffsetFinal);

    // Final UV offset
    vec2 finalUV = vTexCoord;
    if (isHorizontal) {
        finalUV.y += finalOffset * stepLength;
    } else {
        finalUV.x += finalOffset * stepLength;
    }

    FragColor = vec4(texture(uInputTexture, finalUV).rgb, 1.0);
}
)";
    }

    UNIQUE<Shader> m_shader;
};

} // namespace Pina
