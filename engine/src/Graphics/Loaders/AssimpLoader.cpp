/// Pina Engine - Assimp Model Loader Implementation
/// Simplified approach - read vertex data directly like LearnOpenGL

#include "AssimpLoader.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/GltfMaterial.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <algorithm>
#include <cctype>
#include <map>

namespace Pina {

// ============================================================================
// Format Detection
// ============================================================================

enum class ModelFormat {
    Unknown,
    GLTF,   // .gltf, .glb
    OBJ,    // .obj
    FBX,    // .fbx
    COLLADA,// .dae
    ThreeDS,// .3ds
    PLY,    // .ply
    STL     // .stl
};

static std::string getFileExtension(const std::string& path) {
    size_t dotPos = path.find_last_of('.');
    if (dotPos == std::string::npos) return "";
    std::string ext = path.substr(dotPos + 1);
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
    return ext;
}

static ModelFormat detectFormat(const std::string& path) {
    std::string ext = getFileExtension(path);
    if (ext == "gltf" || ext == "glb") return ModelFormat::GLTF;
    if (ext == "obj") return ModelFormat::OBJ;
    if (ext == "fbx") return ModelFormat::FBX;
    if (ext == "dae") return ModelFormat::COLLADA;
    if (ext == "3ds") return ModelFormat::ThreeDS;
    if (ext == "ply") return ModelFormat::PLY;
    if (ext == "stl") return ModelFormat::STL;
    return ModelFormat::Unknown;
}

// Returns true if format needs UV flip for OpenGL
static bool needsUVFlip(ModelFormat format) {
    switch (format) {
        case ModelFormat::OBJ:
        case ModelFormat::FBX:
        case ModelFormat::COLLADA:
        case ModelFormat::ThreeDS:
        case ModelFormat::GLTF:  // glTF uses V=0 at top, OpenGL uses V=0 at bottom
            return true;
        case ModelFormat::PLY:
        case ModelFormat::STL:
        default:
            return false;
    }
}

// Helper to convert Assimp matrix to GLM
static glm::mat4 aiToGlm(const aiMatrix4x4& m) {
    return glm::mat4(
        m.a1, m.b1, m.c1, m.d1,
        m.a2, m.b2, m.c2, m.d2,
        m.a3, m.b3, m.c3, m.d3,
        m.a4, m.b4, m.c4, m.d4
    );
}

// Build mesh-to-transform map by traversing node hierarchy
static void buildMeshTransforms(
    aiNode* node,
    const glm::mat4& parentTransform,
    std::map<unsigned int, glm::mat4>& meshTransforms)
{
    glm::mat4 nodeTransform = aiToGlm(node->mTransformation);
    glm::mat4 worldTransform = parentTransform * nodeTransform;

    // Store transform for each mesh referenced by this node
    for (unsigned int i = 0; i < node->mNumMeshes; ++i) {
        unsigned int meshIndex = node->mMeshes[i];
        meshTransforms[meshIndex] = worldTransform;
    }

    // Process children
    for (unsigned int i = 0; i < node->mNumChildren; ++i) {
        buildMeshTransforms(node->mChildren[i], worldTransform, meshTransforms);
    }
}

// ============================================================================
// Main Load Function
// ============================================================================

UNIQUE<Model> AssimpLoader::load(GraphicsDevice* device, const std::string& path) {
    Assimp::Importer importer;

    // Detect format
    ModelFormat format = detectFormat(path);
    std::cout << "Loading model: " << path << " (format: " << static_cast<int>(format) << ")" << std::endl;

    // Minimal post-processing - just triangulate
    // Like LearnOpenGL: aiProcess_Triangulate | aiProcess_FlipUVs
    unsigned int flags = aiProcess_Triangulate;

    // Add UV flip for formats that need it
    if (needsUVFlip(format)) {
        flags |= aiProcess_FlipUVs;
        std::cout << "  UV flip enabled for this format" << std::endl;
    }

    const aiScene* scene = importer.ReadFile(path, flags);

    if (!scene) {
        std::cerr << "Assimp error loading " << path << ": " << importer.GetErrorString() << std::endl;
        return nullptr;
    }

    if (scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) {
        std::cerr << "Warning: Scene is incomplete: " << path << std::endl;
    }

    if (!scene->mRootNode) {
        std::cerr << "Error: No root node in scene: " << path << std::endl;
        return nullptr;
    }

    // Create model
    auto model = UNIQUE<Model>(new Model());
    model->m_path = path;

    // Extract directory from path
    size_t lastSlash = path.find_last_of("/\\");
    model->m_directory = (lastSlash != std::string::npos) ? path.substr(0, lastSlash) : ".";

    // Setup loading context
    LoadContext ctx;
    ctx.device = device;
    ctx.model = model.get();
    ctx.directory = model->m_directory;
    ctx.scene = scene;
    ctx.format = static_cast<int>(format);

    // Process materials first
    for (unsigned int i = 0; i < scene->mNumMaterials; ++i) {
        Material mat = processMaterial(scene->mMaterials[i], ctx);
        model->m_materials.push_back(std::move(mat));
    }

    // Add a default material if none exist
    if (model->m_materials.empty()) {
        model->m_materials.push_back(Material::createDefault());
    }

    // Build mesh-to-transform map from node hierarchy
    std::map<unsigned int, glm::mat4> meshTransforms;
    buildMeshTransforms(scene->mRootNode, glm::mat4(1.0f), meshTransforms);

    // Process all meshes with their transforms
    std::cout << "=== DEBUG: Processing " << scene->mNumMeshes << " meshes ===" << std::endl;
    for (unsigned int i = 0; i < scene->mNumMeshes; ++i) {
        aiMesh* mesh = scene->mMeshes[i];
        std::cout << "  Mesh " << i << ": " << (mesh->mName.length > 0 ? mesh->mName.C_Str() : "(unnamed)") << std::endl;

        // Get transform for this mesh (identity if not found)
        glm::mat4 transform = glm::mat4(1.0f);
        auto it = meshTransforms.find(i);
        if (it != meshTransforms.end()) {
            transform = it->second;
        }

        auto staticMesh = processMesh(mesh, ctx, transform);
        if (staticMesh) {
            model->m_meshes.push_back(std::move(staticMesh));
            if (mesh->mMaterialIndex < scene->mNumMaterials) {
                model->m_meshMaterialIndices.push_back(mesh->mMaterialIndex);
            } else {
                model->m_meshMaterialIndices.push_back(0);
            }
        }
    }

    std::cout << "Loaded model: " << path << std::endl;
    std::cout << "  Meshes: " << model->m_meshes.size() << std::endl;
    std::cout << "  Materials: " << model->m_materials.size() << std::endl;
    std::cout << "  Textures: " << model->m_textures.size() << std::endl;
    std::cout << "  Bounds: " << model->getSize().x << " x " << model->getSize().y << " x " << model->getSize().z << std::endl;

    return model;
}

// ============================================================================
// Node Processing (not used in simple mode, kept for compatibility)
// ============================================================================

void AssimpLoader::processNode(aiNode* node, const aiScene* scene, LoadContext& ctx, const glm::mat4& parentTransform) {
    // Not used in simple loading mode
    (void)node;
    (void)scene;
    (void)ctx;
    (void)parentTransform;
}

// ============================================================================
// Mesh Processing - Simple direct read like LearnOpenGL
// ============================================================================

UNIQUE<StaticMesh> AssimpLoader::processMesh(aiMesh* mesh, LoadContext& ctx, const glm::mat4& transform) {
    std::vector<float> vertices;
    std::vector<uint32_t> indices;

    if (mesh->mNumVertices == 0) {
        std::cerr << "Warning: Mesh has no vertices, skipping" << std::endl;
        return nullptr;
    }

    // Reserve space: pos(3) + normal(3) + uv(2) = 8 floats per vertex
    vertices.reserve(mesh->mNumVertices * 8);
    indices.reserve(mesh->mNumFaces * 3);

    // Calculate normal matrix for transforming normals
    glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(transform)));

    // Process vertices with transform applied
    for (unsigned int i = 0; i < mesh->mNumVertices; ++i) {
        // Position - apply transform
        glm::vec4 localPos(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z, 1.0f);
        glm::vec4 worldPos = transform * localPos;

        vertices.push_back(worldPos.x);
        vertices.push_back(worldPos.y);
        vertices.push_back(worldPos.z);

        // Expand bounding box with transformed position
        ctx.model->m_boundingBox.expand(glm::vec3(worldPos));

        // Normal - apply normal matrix
        if (mesh->HasNormals()) {
            glm::vec3 localNorm(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z);
            glm::vec3 worldNorm = glm::normalize(normalMatrix * localNorm);
            vertices.push_back(worldNorm.x);
            vertices.push_back(worldNorm.y);
            vertices.push_back(worldNorm.z);
        } else {
            vertices.push_back(0.0f);
            vertices.push_back(1.0f);
            vertices.push_back(0.0f);
        }

        // Texture coordinates (unchanged by transform)
        if (mesh->mTextureCoords[0]) {
            vertices.push_back(mesh->mTextureCoords[0][i].x);
            vertices.push_back(mesh->mTextureCoords[0][i].y);
        } else {
            vertices.push_back(0.0f);
            vertices.push_back(0.0f);
        }
    }

    // Process indices
    uint32_t maxIndex = 0;
    for (unsigned int i = 0; i < mesh->mNumFaces; ++i) {
        aiFace& face = mesh->mFaces[i];
        if (face.mNumIndices == 3) {
            indices.push_back(face.mIndices[0]);
            indices.push_back(face.mIndices[1]);
            indices.push_back(face.mIndices[2]);
            maxIndex = std::max(maxIndex, face.mIndices[0]);
            maxIndex = std::max(maxIndex, face.mIndices[1]);
            maxIndex = std::max(maxIndex, face.mIndices[2]);
        }
    }

    // Debug logging
    std::cout << "    Vertices: " << mesh->mNumVertices << ", Faces: " << mesh->mNumFaces
              << ", MaterialIdx: " << mesh->mMaterialIndex << std::endl;
    if (maxIndex >= mesh->mNumVertices) {
        std::cerr << "    ERROR: Max index " << maxIndex << " >= vertex count " << mesh->mNumVertices << std::endl;
    }

    if (indices.empty()) {
        std::cerr << "Warning: Mesh has no valid triangles, skipping" << std::endl;
        return nullptr;
    }

    return StaticMesh::create(ctx.device, vertices, indices);
}

// ============================================================================
// Material Processing
// ============================================================================

Material AssimpLoader::processMaterial(aiMaterial* mat, LoadContext& ctx) {
    Material material;

    // Get material name for debugging
    aiString matName;
    if (mat->Get(AI_MATKEY_NAME, matName) == AI_SUCCESS) {
        std::cout << "  Processing material: " << matName.C_Str() << std::endl;
    }

    // ========================================================================
    // Detect if this is a PBR material
    // ========================================================================
    bool isPBR = false;

    // Check for PBR textures
    if (mat->GetTextureCount(aiTextureType_BASE_COLOR) > 0 ||
        mat->GetTextureCount(aiTextureType_METALNESS) > 0 ||
        mat->GetTextureCount(aiTextureType_DIFFUSE_ROUGHNESS) > 0 ||
        mat->GetTextureCount(aiTextureType_UNKNOWN) > 0) {
        isPBR = true;
    }

    // Check for PBR properties
    float metallicFactor = 0.0f;
    float roughnessFactor = 0.5f;
    if (mat->Get(AI_MATKEY_METALLIC_FACTOR, metallicFactor) == AI_SUCCESS ||
        mat->Get(AI_MATKEY_ROUGHNESS_FACTOR, roughnessFactor) == AI_SUCCESS) {
        isPBR = true;
    }

    // ========================================================================
    // Common properties
    // ========================================================================
    aiColor3D color;

    // Emissive
    if (mat->Get(AI_MATKEY_COLOR_EMISSIVE, color) == AI_SUCCESS) {
        material.setEmissive(Color(color.r, color.g, color.b));
    }

    // Normal map
    Texture* normalMap = loadMaterialTexture(mat, aiTextureType_NORMALS, ctx);
    if (!normalMap) normalMap = loadMaterialTexture(mat, aiTextureType_HEIGHT, ctx);
    if (!normalMap) normalMap = loadMaterialTexture(mat, aiTextureType_NORMAL_CAMERA, ctx);
    if (normalMap) material.setNormalMap(normalMap);

    // Emission map
    Texture* emissionMap = loadMaterialTexture(mat, aiTextureType_EMISSIVE, ctx);
    if (emissionMap) material.setEmissionMap(emissionMap);

    // ========================================================================
    // PBR or Blinn-Phong
    // ========================================================================
    if (isPBR) {
        std::cout << "    -> PBR material detected" << std::endl;

        // Base color
        aiColor4D baseColor(1.0f, 1.0f, 1.0f, 1.0f);
        if (mat->Get(AI_MATKEY_BASE_COLOR, baseColor) == AI_SUCCESS) {
            material.setAlbedo(Color(baseColor.r, baseColor.g, baseColor.b, baseColor.a));
        } else if (mat->Get(AI_MATKEY_COLOR_DIFFUSE, color) == AI_SUCCESS) {
            material.setAlbedo(Color(color.r, color.g, color.b));
        }

        // Metallic and roughness
        mat->Get(AI_MATKEY_METALLIC_FACTOR, metallicFactor);
        mat->Get(AI_MATKEY_ROUGHNESS_FACTOR, roughnessFactor);
        material.setMetallic(metallicFactor);
        material.setRoughness(roughnessFactor);

        // Opacity - check both explicit opacity and glTF alphaMode
        float opacity = 1.0f;
        mat->Get(AI_MATKEY_OPACITY, opacity);

        // Check glTF alpha mode (BLEND mode means transparent)
        aiString alphaMode;
        if (mat->Get(AI_MATKEY_GLTF_ALPHAMODE, alphaMode) == AI_SUCCESS) {
            std::string alphaModeStr = alphaMode.C_Str();
            if (alphaModeStr == "BLEND") {
                // For BLEND mode, use alpha from base color or set to semi-transparent
                // so isTransparent() returns true
                if (opacity >= 1.0f) {
                    opacity = 0.99f;  // Mark as transparent for sorting
                }
                std::cout << "    -> Transparent material (alphaMode: BLEND)" << std::endl;
            }
        }
        material.setOpacity(opacity);

        // Albedo texture
        Texture* albedoMap = loadMaterialTexture(mat, aiTextureType_BASE_COLOR, ctx);
        if (!albedoMap) albedoMap = loadMaterialTexture(mat, aiTextureType_DIFFUSE, ctx);
        if (albedoMap) {
            material.setAlbedoMap(albedoMap);
            material.setDiffuseMap(albedoMap);
        }

        // Metallic-Roughness map
        Texture* mrMap = loadMaterialTexture(mat, aiTextureType_DIFFUSE_ROUGHNESS, ctx);
        if (!mrMap) mrMap = loadMaterialTexture(mat, aiTextureType_UNKNOWN, ctx);
        if (mrMap) {
            material.setMetallicRoughnessMap(mrMap);
        } else {
            Texture* metallicMap = loadMaterialTexture(mat, aiTextureType_METALNESS, ctx);
            if (metallicMap) material.setMetallicMap(metallicMap);
            Texture* roughnessMap = loadMaterialTexture(mat, aiTextureType_SHININESS, ctx);
            if (roughnessMap) material.setRoughnessMap(roughnessMap);
        }

        // AO map
        Texture* aoMap = loadMaterialTexture(mat, aiTextureType_AMBIENT_OCCLUSION, ctx);
        if (!aoMap) aoMap = loadMaterialTexture(mat, aiTextureType_LIGHTMAP, ctx);
        if (aoMap) material.setAOMap(aoMap);

        // Opacity map
        Texture* opacityMap = loadMaterialTexture(mat, aiTextureType_OPACITY, ctx);
        if (opacityMap) material.setOpacityMap(opacityMap);

    } else {
        std::cout << "    -> Blinn-Phong material" << std::endl;

        if (mat->Get(AI_MATKEY_COLOR_DIFFUSE, color) == AI_SUCCESS) {
            material.setDiffuse(Color(color.r, color.g, color.b));
        }
        if (mat->Get(AI_MATKEY_COLOR_SPECULAR, color) == AI_SUCCESS) {
            material.setSpecular(Color(color.r, color.g, color.b));
        }
        if (mat->Get(AI_MATKEY_COLOR_AMBIENT, color) == AI_SUCCESS) {
            material.setAmbient(Color(color.r, color.g, color.b));
        }

        float shininess = 32.0f;
        mat->Get(AI_MATKEY_SHININESS, shininess);
        material.setShininess(shininess > 0 ? shininess : 32.0f);

        Texture* diffuseMap = loadMaterialTexture(mat, aiTextureType_DIFFUSE, ctx);
        if (diffuseMap) material.setDiffuseMap(diffuseMap);

        Texture* specularMap = loadMaterialTexture(mat, aiTextureType_SPECULAR, ctx);
        if (specularMap) material.setSpecularMap(specularMap);
    }

    return material;
}

// ============================================================================
// Texture Loading
// ============================================================================

Texture* AssimpLoader::loadMaterialTexture(aiMaterial* mat, int type, LoadContext& ctx) {
    aiTextureType texType = static_cast<aiTextureType>(type);

    if (mat->GetTextureCount(texType) == 0) {
        return nullptr;
    }

    aiString texPath;
    if (mat->GetTexture(texType, 0, &texPath) != AI_SUCCESS) {
        return nullptr;
    }

    std::string texPathStr = texPath.C_Str();

    // Check if already loaded
    auto it = ctx.loadedTextures.find(texPathStr);
    if (it != ctx.loadedTextures.end()) {
        return ctx.model->m_textures[it->second].get();
    }

    UNIQUE<Texture> texture;

    // Embedded texture
    if (texPathStr.length() > 0 && texPathStr[0] == '*') {
        int texIndex = std::atoi(texPathStr.c_str() + 1);
        if (ctx.scene && texIndex >= 0 && static_cast<unsigned int>(texIndex) < ctx.scene->mNumTextures) {
            aiTexture* embeddedTex = ctx.scene->mTextures[texIndex];
            texture = loadEmbeddedTexture(embeddedTex, ctx);
        }
    } else {
        // External texture file
        std::vector<std::string> pathsToTry;
        pathsToTry.push_back(ctx.directory + "/" + texPathStr);

        size_t lastSlash = texPathStr.find_last_of("/\\");
        if (lastSlash != std::string::npos) {
            pathsToTry.push_back(ctx.directory + "/" + texPathStr.substr(lastSlash + 1));
        }
        pathsToTry.push_back(ctx.directory + "/textures/" + texPathStr);
        if (lastSlash != std::string::npos) {
            pathsToTry.push_back(ctx.directory + "/textures/" + texPathStr.substr(lastSlash + 1));
        }

        for (const auto& tryPath : pathsToTry) {
            texture = Texture::load(ctx.device, tryPath);
            if (texture) break;
        }
    }

    if (!texture) {
        std::cerr << "Failed to load texture: " << texPathStr << std::endl;
        return nullptr;
    }

    size_t index = ctx.model->m_textures.size();
    ctx.loadedTextures[texPathStr] = index;
    Texture* texPtr = texture.get();
    ctx.model->m_textures.push_back(std::move(texture));

    return texPtr;
}

UNIQUE<Texture> AssimpLoader::loadEmbeddedTexture(aiTexture* tex, LoadContext& ctx) {
    if (!tex) return nullptr;

    if (tex->mHeight == 0) {
        // Compressed format
        return Texture::loadFromMemory(ctx.device,
            reinterpret_cast<const unsigned char*>(tex->pcData),
            tex->mWidth);
    } else {
        // Raw ARGB8888
        std::vector<unsigned char> rgbaData(tex->mWidth * tex->mHeight * 4);
        for (unsigned int i = 0; i < tex->mWidth * tex->mHeight; ++i) {
            aiTexel& texel = tex->pcData[i];
            rgbaData[i * 4 + 0] = texel.r;
            rgbaData[i * 4 + 1] = texel.g;
            rgbaData[i * 4 + 2] = texel.b;
            rgbaData[i * 4 + 3] = texel.a;
        }
        return Texture::createFromRGBA(ctx.device, rgbaData.data(),
            tex->mWidth, tex->mHeight);
    }
}

} // namespace Pina
