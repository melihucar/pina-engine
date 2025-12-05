#pragma once

/// Pina Engine - Shader Library
/// Standard GLSL shader code for common rendering scenarios

#include "../../Core/Export.h"

namespace Pina {

/// Provides standard GLSL shader code as strings
/// Use these for quick setup or as templates for custom shaders
class PINA_API ShaderLibrary {
public:
    // ========================================================================
    // Complete Shaders
    // ========================================================================

    /// Standard lit vertex shader
    /// Requires: aPosition (vec3), aNormal (vec3), aTexCoord (vec2)
    /// Uniforms: uModel, uView, uProjection, uNormalMatrix
    static const char* getStandardVertexShader();

    /// Standard lit fragment shader with Blinn-Phong lighting
    /// Supports all light types (directional, point, spot)
    /// Uniforms: uLights[], uMaterial, uViewPosition, uGlobalAmbient, uLightCount
    static const char* getStandardFragmentShader();

    /// Simple unlit vertex shader (for debug/UI rendering)
    static const char* getUnlitVertexShader();

    /// Simple unlit fragment shader (solid color)
    static const char* getUnlitFragmentShader();

    // ========================================================================
    // Shader Components (for custom shaders)
    // ========================================================================

    /// Light and Material struct definitions
    static const char* getLightStructs();

    /// Lighting calculation functions (Blinn-Phong)
    static const char* getLightingFunctions();
};

} // namespace Pina
