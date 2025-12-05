/// Pina Engine - Shader Library Implementation

#include "ShaderLibrary.h"

namespace Pina {

// ============================================================================
// Light Struct Definitions
// ============================================================================

const char* ShaderLibrary::getLightStructs() {
    return R"(
// Maximum number of lights
const int MAX_LIGHTS = 8;

// Light data structure (GPU-aligned)
struct Light {
    vec4 position;     // xyz = position, w = type (0=dir, 1=point, 2=spot)
    vec4 direction;    // xyz = direction, w = enabled (0 or 1)
    vec4 color;        // rgb = color * intensity, a = intensity
    vec4 ambient;      // rgb = ambient contribution
    vec4 attenuation;  // x = constant, y = linear, z = quadratic, w = range
    vec4 cutoff;       // x = innerCos, y = outerCos (spotlight only)
};

// Material properties
struct Material {
    vec3 diffuse;
    vec3 specular;
    vec3 ambient;
    vec3 emissive;
    float shininess;
};
)";
}

// ============================================================================
// Lighting Functions
// ============================================================================

const char* ShaderLibrary::getLightingFunctions() {
    return R"(
// Calculate attenuation for point/spot lights
float calculateAttenuation(vec4 attenuation, float distance) {
    return 1.0 / (attenuation.x + attenuation.y * distance +
                  attenuation.z * distance * distance);
}

// Calculate spotlight intensity (cone falloff)
float calculateSpotIntensity(vec3 lightDir, vec3 spotDir, vec4 cutoff) {
    float theta = dot(lightDir, normalize(-spotDir));
    float epsilon = cutoff.x - cutoff.y;  // inner - outer
    return clamp((theta - cutoff.y) / epsilon, 0.0, 1.0);
}

// Calculate contribution from a single light (Blinn-Phong)
vec3 calculateLight(Light light, Material material, vec3 normal, vec3 viewDir, vec3 fragPos) {
    // Check if light is enabled
    if (light.direction.w < 0.5) {
        return vec3(0.0);
    }

    int lightType = int(light.position.w + 0.5);  // 0=dir, 1=point, 2=spot

    vec3 lightDir;
    float attenuation = 1.0;
    float spotIntensity = 1.0;

    if (lightType == 0) {
        // Directional light - no position, parallel rays
        lightDir = normalize(-light.direction.xyz);
    } else {
        // Point or Spot light - has position
        vec3 toLight = light.position.xyz - fragPos;
        float distance = length(toLight);
        lightDir = toLight / distance;
        attenuation = calculateAttenuation(light.attenuation, distance);

        if (lightType == 2) {
            // Spotlight - cone falloff
            spotIntensity = calculateSpotIntensity(lightDir, light.direction.xyz, light.cutoff);
        }
    }

    // Ambient component
    vec3 ambient = light.ambient.rgb * material.ambient;

    // Diffuse component (Lambertian)
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = diff * light.color.rgb * material.diffuse;

    // Specular component (Blinn-Phong)
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), material.shininess);
    vec3 specular = spec * light.color.rgb * material.specular;

    // Combine all components with attenuation and spot intensity
    return ambient + (diffuse + specular) * attenuation * spotIntensity;
}
)";
}

// ============================================================================
// Standard Vertex Shader
// ============================================================================

const char* ShaderLibrary::getStandardVertexShader() {
    return R"(
#version 410 core

// Vertex attributes
layout (location = 0) in vec3 aPosition;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;

// Transformation matrices
uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProjection;
uniform mat3 uNormalMatrix;  // transpose(inverse(mat3(uModel)))

// Output to fragment shader
out vec3 vWorldPos;
out vec3 vNormal;
out vec2 vTexCoord;

void main() {
    // Transform vertex to world space
    vec4 worldPos = uModel * vec4(aPosition, 1.0);
    vWorldPos = worldPos.xyz;

    // Transform normal to world space (handles non-uniform scaling)
    vNormal = uNormalMatrix * aNormal;

    // Pass through texture coordinates
    vTexCoord = aTexCoord;

    // Transform to clip space
    gl_Position = uProjection * uView * worldPos;
}
)";
}

// ============================================================================
// Standard Fragment Shader (Full Blinn-Phong Lighting)
// ============================================================================

const char* ShaderLibrary::getStandardFragmentShader() {
    return R"(
#version 410 core

// ============================================================================
// Light and Material Structures
// ============================================================================

const int MAX_LIGHTS = 8;

struct Light {
    vec4 position;
    vec4 direction;
    vec4 color;
    vec4 ambient;
    vec4 attenuation;
    vec4 cutoff;
};

struct Material {
    vec3 diffuse;
    vec3 specular;
    vec3 ambient;
    vec3 emissive;
    float shininess;
};

// ============================================================================
// Uniforms
// ============================================================================

uniform int uLightCount;
uniform vec3 uViewPosition;
uniform vec3 uGlobalAmbient;
uniform Light uLights[MAX_LIGHTS];
uniform Material uMaterial;

// Texture maps
uniform sampler2D uDiffuseMap;
uniform sampler2D uSpecularMap;
uniform sampler2D uNormalMap;
uniform bool uUseDiffuseMap;
uniform bool uUseSpecularMap;
uniform bool uUseNormalMap;

// ============================================================================
// Inputs from vertex shader
// ============================================================================

in vec3 vWorldPos;
in vec3 vNormal;
in vec2 vTexCoord;

// ============================================================================
// Output
// ============================================================================

out vec4 FragColor;

// ============================================================================
// Lighting Functions
// ============================================================================

float calculateAttenuation(vec4 attenuation, float distance) {
    return 1.0 / (attenuation.x + attenuation.y * distance +
                  attenuation.z * distance * distance);
}

float calculateSpotIntensity(vec3 lightDir, vec3 spotDir, vec4 cutoff) {
    float theta = dot(lightDir, normalize(-spotDir));
    float epsilon = cutoff.x - cutoff.y;
    return clamp((theta - cutoff.y) / epsilon, 0.0, 1.0);
}

vec3 calculateLight(Light light, vec3 normal, vec3 viewDir, vec3 fragPos,
                    vec3 diffuseColor, vec3 specularColor) {
    if (light.direction.w < 0.5) {
        return vec3(0.0);
    }

    int lightType = int(light.position.w + 0.5);

    vec3 lightDir;
    float attenuation = 1.0;
    float spotIntensity = 1.0;

    if (lightType == 0) {
        lightDir = normalize(-light.direction.xyz);
    } else {
        vec3 toLight = light.position.xyz - fragPos;
        float distance = length(toLight);
        lightDir = toLight / distance;
        attenuation = calculateAttenuation(light.attenuation, distance);

        if (lightType == 2) {
            spotIntensity = calculateSpotIntensity(lightDir, light.direction.xyz, light.cutoff);
        }
    }

    // Ambient
    vec3 ambient = light.ambient.rgb * uMaterial.ambient;

    // Diffuse (use texture-modulated color)
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = diff * light.color.rgb * diffuseColor;

    // Specular (Blinn-Phong, use texture-modulated specular)
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), uMaterial.shininess);
    vec3 specular = spec * light.color.rgb * specularColor;

    return ambient + (diffuse + specular) * attenuation * spotIntensity;
}

// ============================================================================
// Main
// ============================================================================

void main() {
    // Normalize interpolated normal
    vec3 normal = normalize(vNormal);

    // View direction (from fragment to camera)
    vec3 viewDir = normalize(uViewPosition - vWorldPos);

    // Sample diffuse color (texture or material)
    vec3 diffuseColor = uMaterial.diffuse;
    float alpha = 1.0;
    if (uUseDiffuseMap) {
        vec4 texColor = texture(uDiffuseMap, vTexCoord);
        diffuseColor *= texColor.rgb;
        alpha = texColor.a;
    }

    // Sample specular color (texture or material)
    vec3 specularColor = uMaterial.specular;
    if (uUseSpecularMap) {
        specularColor *= texture(uSpecularMap, vTexCoord).rgb;
    }

    // Start with global ambient
    vec3 result = uGlobalAmbient * uMaterial.ambient;

    // Add emissive (self-illumination)
    result += uMaterial.emissive;

    // Accumulate contribution from all lights
    for (int i = 0; i < uLightCount && i < MAX_LIGHTS; ++i) {
        result += calculateLight(uLights[i], normal, viewDir, vWorldPos,
                                 diffuseColor, specularColor);
    }

    // Final color output with alpha from diffuse texture
    FragColor = vec4(result, alpha);
}
)";
}

// ============================================================================
// Unlit Shaders (for debugging, UI, etc.)
// ============================================================================

const char* ShaderLibrary::getUnlitVertexShader() {
    return R"(
#version 410 core

layout (location = 0) in vec3 aPosition;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;

uniform mat4 uMVP;

out vec2 vTexCoord;

void main() {
    gl_Position = uMVP * vec4(aPosition, 1.0);
    vTexCoord = aTexCoord;
}
)";
}

const char* ShaderLibrary::getUnlitFragmentShader() {
    return R"(
#version 410 core

uniform vec3 uColor;

in vec2 vTexCoord;
out vec4 FragColor;

void main() {
    FragColor = vec4(uColor, 1.0);
}
)";
}

} // namespace Pina
