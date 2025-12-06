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

// Shadow mapping
uniform mat4 uLightSpaceMatrix;

// Output to fragment shader
out vec3 vWorldPos;
out vec3 vNormal;
out vec2 vTexCoord;
out vec4 vLightSpacePos;

void main() {
    // Transform vertex to world space
    vec4 worldPos = uModel * vec4(aPosition, 1.0);
    vWorldPos = worldPos.xyz;

    // Transform normal to world space (handles non-uniform scaling)
    vNormal = uNormalMatrix * aNormal;

    // Pass through texture coordinates
    vTexCoord = aTexCoord;

    // Calculate position in light space for shadow mapping
    vLightSpacePos = uLightSpaceMatrix * worldPos;

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
uniform bool uWireframe;
uniform int uShadingMode;  // 0=smooth, 1=flat, 2=wireframe

// Shadow mapping
uniform sampler2D uShadowMap;
uniform bool uEnableShadows;
uniform float uShadowBias;
uniform float uShadowNormalBias;
uniform float uShadowSoftness;

// ============================================================================
// Inputs from vertex shader
// ============================================================================

in vec3 vWorldPos;
in vec3 vNormal;
in vec2 vTexCoord;
in vec4 vLightSpacePos;

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

// 16-sample Poisson disk for natural shadow sampling
const vec2 poissonDisk[16] = vec2[](
    vec2(-0.94201624, -0.39906216), vec2(0.94558609, -0.76890725),
    vec2(-0.094184101, -0.92938870), vec2(0.34495938, 0.29387760),
    vec2(-0.91588581, 0.45771432), vec2(-0.81544232, -0.87912464),
    vec2(-0.38277543, 0.27676845), vec2(0.97484398, 0.75648379),
    vec2(0.44323325, -0.97511554), vec2(0.53742981, -0.47373420),
    vec2(-0.26496911, -0.41893023), vec2(0.79197514, 0.19090188),
    vec2(-0.24188840, 0.99706507), vec2(-0.81409955, 0.91437590),
    vec2(0.19984126, 0.78641367), vec2(0.14383161, -0.14100790)
);

// Shadow calculation with Poisson disk sampling for soft shadows
float ShadowCalculation(vec4 fragPosLightSpace, vec3 normal, vec3 lightDir) {
    // Perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;

    // Transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;

    // Keep fragments outside the far plane lit
    if (projCoords.z > 1.0) {
        return 0.0;
    }

    float currentDepth = projCoords.z;

    // Dynamic bias based on surface angle to light
    float bias = max(uShadowBias * (1.0 - dot(normal, lightDir)), uShadowBias * 0.1);

    // Poisson disk sampling with softness control
    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(uShadowMap, 0);
    float radius = uShadowSoftness * 3.0;  // Base radius scaled by softness

    for (int i = 0; i < 16; i++) {
        float pcfDepth = texture(uShadowMap, projCoords.xy + poissonDisk[i] * texelSize * radius).r;
        shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;
    }
    return shadow / 16.0;
}

vec3 calculateLight(Light light, vec3 normal, vec3 viewDir, vec3 fragPos,
                    vec3 diffuseColor, vec3 specularColor, float shadow) {
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

    // Apply shadow to diffuse and specular (ambient is unaffected by shadows)
    return ambient + (diffuse + specular) * attenuation * spotIntensity * (1.0 - shadow);
}

// ============================================================================
// Main
// ============================================================================

void main() {
    // Wireframe mode: output solid white
    if (uWireframe || uShadingMode == 2) {
        FragColor = vec4(1.0, 1.0, 1.0, 1.0);
        return;
    }

    // Get normal based on shading mode
    vec3 normal;
    if (uShadingMode == 1) {
        // Flat shading: calculate from screen-space derivatives
        vec3 dFdxPos = dFdx(vWorldPos);
        vec3 dFdyPos = dFdy(vWorldPos);
        normal = normalize(cross(dFdxPos, dFdyPos));
    } else {
        // Smooth shading: interpolated normal
        normal = normalize(vNormal);
    }

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

    // Calculate shadow for first directional light
    float shadow = 0.0;
    if (uEnableShadows && uLightCount > 0) {
        // Check if first light is directional (position.w < 0.5)
        if (uLights[0].position.w < 0.5 && uLights[0].direction.w > 0.5) {
            vec3 lightDir = normalize(-uLights[0].direction.xyz);
            shadow = ShadowCalculation(vLightSpacePos, normal, lightDir);
        }
    }

    // Accumulate contribution from all lights
    for (int i = 0; i < uLightCount && i < MAX_LIGHTS; ++i) {
        // Apply shadow only to first directional light
        float lightShadow = (i == 0 && uLights[0].position.w < 0.5) ? shadow : 0.0;
        result += calculateLight(uLights[i], normal, viewDir, vWorldPos,
                                 diffuseColor, specularColor, lightShadow);
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

// ============================================================================
// PBR Vertex Shader (same layout as standard)
// ============================================================================

const char* ShaderLibrary::getPBRVertexShader() {
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
uniform mat3 uNormalMatrix;

// Shadow mapping
uniform mat4 uLightSpaceMatrix;

// Output to fragment shader
out vec3 vWorldPos;
out vec3 vNormal;
out vec2 vTexCoord;
out vec4 vLightSpacePos;

void main() {
    vec4 worldPos = uModel * vec4(aPosition, 1.0);
    vWorldPos = worldPos.xyz;
    vNormal = uNormalMatrix * aNormal;
    vTexCoord = aTexCoord;
    vLightSpacePos = uLightSpaceMatrix * worldPos;
    gl_Position = uProjection * uView * worldPos;
}
)";
}

// ============================================================================
// PBR Fragment Shader (Cook-Torrance BRDF)
// ============================================================================

const char* ShaderLibrary::getPBRFragmentShader() {
    return R"(
#version 410 core

// ============================================================================
// Constants
// ============================================================================

const float PI = 3.14159265359;
const int MAX_LIGHTS = 8;

// ============================================================================
// Light Structure (same as Blinn-Phong)
// ============================================================================

struct Light {
    vec4 position;     // xyz = position, w = type (0=dir, 1=point, 2=spot)
    vec4 direction;    // xyz = direction, w = enabled (0 or 1)
    vec4 color;        // rgb = color * intensity, a = intensity
    vec4 ambient;      // rgb = ambient contribution
    vec4 attenuation;  // x = constant, y = linear, z = quadratic, w = range
    vec4 cutoff;       // x = innerCos, y = outerCos (spotlight only)
};

// ============================================================================
// Uniforms
// ============================================================================

// Lighting
uniform int uLightCount;
uniform vec3 uViewPosition;
uniform vec3 uGlobalAmbient;
uniform Light uLights[MAX_LIGHTS];

// PBR Material properties
uniform vec3 uAlbedo;
uniform float uMetallic;
uniform float uRoughness;
uniform float uAO;
uniform vec3 uEmissive;
uniform float uOpacity;

// Texture maps
uniform sampler2D uAlbedoMap;
uniform sampler2D uMetallicRoughnessMap;
uniform sampler2D uMetallicMap;
uniform sampler2D uRoughnessMap;
uniform sampler2D uNormalMap;
uniform sampler2D uAOMap;
uniform sampler2D uEmissionMap;

// Texture flags
uniform bool uUseAlbedoMap;
uniform bool uUseMetallicRoughnessMap;
uniform bool uUseMetallicMap;
uniform bool uUseRoughnessMap;
uniform bool uUseNormalMap;
uniform bool uUseAOMap;
uniform bool uUseEmissionMap;

// Rendering flags
uniform bool uWireframe;
uniform int uShadingMode;  // 0=smooth, 1=flat, 2=wireframe

// Shadow mapping
uniform sampler2D uShadowMap;
uniform bool uEnableShadows;
uniform float uShadowBias;
uniform float uShadowNormalBias;
uniform float uShadowSoftness;

// ============================================================================
// Inputs from vertex shader
// ============================================================================

in vec3 vWorldPos;
in vec3 vNormal;
in vec2 vTexCoord;
in vec4 vLightSpacePos;

// ============================================================================
// Output
// ============================================================================

out vec4 FragColor;

// ============================================================================
// Shadow Calculation
// ============================================================================

// 16-sample Poisson disk for natural shadow sampling
const vec2 poissonDiskPBR[16] = vec2[](
    vec2(-0.94201624, -0.39906216), vec2(0.94558609, -0.76890725),
    vec2(-0.094184101, -0.92938870), vec2(0.34495938, 0.29387760),
    vec2(-0.91588581, 0.45771432), vec2(-0.81544232, -0.87912464),
    vec2(-0.38277543, 0.27676845), vec2(0.97484398, 0.75648379),
    vec2(0.44323325, -0.97511554), vec2(0.53742981, -0.47373420),
    vec2(-0.26496911, -0.41893023), vec2(0.79197514, 0.19090188),
    vec2(-0.24188840, 0.99706507), vec2(-0.81409955, 0.91437590),
    vec2(0.19984126, 0.78641367), vec2(0.14383161, -0.14100790)
);

float ShadowCalculation(vec4 fragPosLightSpace, vec3 normal, vec3 lightDir) {
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;

    if (projCoords.z > 1.0) {
        return 0.0;
    }

    float currentDepth = projCoords.z;
    float bias = max(uShadowBias * (1.0 - dot(normal, lightDir)), uShadowBias * 0.1);

    // Poisson disk sampling with softness control
    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(uShadowMap, 0);
    float radius = uShadowSoftness * 3.0;  // Base radius scaled by softness

    for (int i = 0; i < 16; i++) {
        float pcfDepth = texture(uShadowMap, projCoords.xy + poissonDiskPBR[i] * texelSize * radius).r;
        shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;
    }
    return shadow / 16.0;
}

// ============================================================================
// PBR Functions (Cook-Torrance BRDF)
// ============================================================================

// Normal Distribution Function (GGX/Trowbridge-Reitz)
float DistributionGGX(vec3 N, vec3 H, float roughness) {
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;

    float nom = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return nom / max(denom, 0.0001);
}

// Geometry Function (Schlick-GGX)
float GeometrySchlickGGX(float NdotV, float roughness) {
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;

    float nom = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / max(denom, 0.0001);
}

// Smith's method for geometry
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness) {
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}

// Fresnel (Schlick approximation)
vec3 fresnelSchlick(float cosTheta, vec3 F0) {
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

// Calculate attenuation
float calculateAttenuation(vec4 attenuation, float distance) {
    return 1.0 / (attenuation.x + attenuation.y * distance +
                  attenuation.z * distance * distance);
}

// Calculate spotlight intensity
float calculateSpotIntensity(vec3 lightDir, vec3 spotDir, vec4 cutoff) {
    float theta = dot(lightDir, normalize(-spotDir));
    float epsilon = cutoff.x - cutoff.y;
    return clamp((theta - cutoff.y) / epsilon, 0.0, 1.0);
}

// Calculate PBR lighting for a single light
vec3 calculatePBRLight(Light light, vec3 N, vec3 V, vec3 fragPos,
                       vec3 albedo, float metallic, float roughness, vec3 F0, float shadow) {
    if (light.direction.w < 0.5) {
        return vec3(0.0);
    }

    int lightType = int(light.position.w + 0.5);

    vec3 L;
    float attenuation = 1.0;
    float spotIntensity = 1.0;

    if (lightType == 0) {
        // Directional light
        L = normalize(-light.direction.xyz);
    } else {
        // Point or Spot light
        vec3 toLight = light.position.xyz - fragPos;
        float distance = length(toLight);
        L = toLight / distance;
        attenuation = calculateAttenuation(light.attenuation, distance);

        if (lightType == 2) {
            spotIntensity = calculateSpotIntensity(L, light.direction.xyz, light.cutoff);
        }
    }

    vec3 H = normalize(V + L);
    vec3 radiance = light.color.rgb * attenuation * spotIntensity;

    // Cook-Torrance BRDF
    float NDF = DistributionGGX(N, H, roughness);
    float G = GeometrySmith(N, V, L, roughness);
    vec3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);

    // Specular
    vec3 numerator = NDF * G * F;
    float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
    vec3 specular = numerator / denominator;

    // kS is equal to Fresnel
    vec3 kS = F;
    // For energy conservation, diffuse and specular can't be above 1.0
    vec3 kD = vec3(1.0) - kS;
    // Multiply kD by the inverse metalness - metals have no diffuse
    kD *= 1.0 - metallic;

    // Lambert diffuse
    float NdotL = max(dot(N, L), 0.0);

    // Apply shadow to entire light contribution (diffuse + specular)
    return (kD * albedo / PI + specular) * radiance * NdotL * (1.0 - shadow);
}

// ============================================================================
// Main
// ============================================================================

void main() {
    // Wireframe mode
    if (uWireframe || uShadingMode == 2) {
        FragColor = vec4(1.0, 1.0, 1.0, 1.0);
        return;
    }

    // Get normal based on shading mode
    vec3 N;
    if (uShadingMode == 1) {
        // Flat shading: calculate from screen-space derivatives
        vec3 dFdxPos = dFdx(vWorldPos);
        vec3 dFdyPos = dFdy(vWorldPos);
        N = normalize(cross(dFdxPos, dFdyPos));
    } else {
        // Smooth shading: interpolated normal
        N = normalize(vNormal);
    }

    vec3 V = normalize(uViewPosition - vWorldPos);

    // Sample material properties
    vec3 albedo = uAlbedo;
    float alpha = uOpacity;
    if (uUseAlbedoMap) {
        vec4 albedoSample = texture(uAlbedoMap, vTexCoord);
        albedo *= pow(albedoSample.rgb, vec3(2.2)); // sRGB to linear
        alpha *= albedoSample.a;  // Use texture alpha for transparency
    }

    float metallic = uMetallic;
    float roughness = uRoughness;

    if (uUseMetallicRoughnessMap) {
        // glTF format: G = roughness, B = metallic
        vec3 mr = texture(uMetallicRoughnessMap, vTexCoord).rgb;
        roughness *= mr.g;
        metallic *= mr.b;
    } else {
        if (uUseMetallicMap) {
            metallic *= texture(uMetallicMap, vTexCoord).r;
        }
        if (uUseRoughnessMap) {
            roughness *= texture(uRoughnessMap, vTexCoord).r;
        }
    }

    float ao = uAO;
    if (uUseAOMap) {
        ao *= texture(uAOMap, vTexCoord).r;
    }

    // Calculate F0 (reflectance at normal incidence)
    vec3 F0 = vec3(0.04); // Default for dielectrics
    F0 = mix(F0, albedo, metallic);

    // Calculate shadow for first directional light
    float shadow = 0.0;
    if (uEnableShadows && uLightCount > 0) {
        if (uLights[0].position.w < 0.5 && uLights[0].direction.w > 0.5) {
            vec3 lightDir = normalize(-uLights[0].direction.xyz);
            shadow = ShadowCalculation(vLightSpacePos, N, lightDir);
        }
    }

    // Accumulate lighting
    vec3 Lo = vec3(0.0);
    for (int i = 0; i < uLightCount && i < MAX_LIGHTS; ++i) {
        // Apply shadow only to first directional light
        float lightShadow = (i == 0 && uLights[0].position.w < 0.5) ? shadow : 0.0;
        Lo += calculatePBRLight(uLights[i], N, V, vWorldPos, albedo, metallic, roughness, F0, lightShadow);
    }

    // Ambient lighting (simplified IBL approximation) - unaffected by shadows
    vec3 ambient = uGlobalAmbient * albedo * ao;

    // Emission (glTF: emissive = texture * factor, or just factor if no texture)
    vec3 emission = uEmissive;
    if (uUseEmissionMap) {
        // When texture is present, use texture RGB (black areas = no emission)
        emission = texture(uEmissionMap, vTexCoord).rgb;
    }

    vec3 color = ambient + Lo + emission;

    // HDR tonemapping (Reinhard)
    color = color / (color + vec3(1.0));

    // Gamma correction
    color = pow(color, vec3(1.0 / 2.2));

    FragColor = vec4(color, alpha);
}
)";
}

} // namespace Pina
