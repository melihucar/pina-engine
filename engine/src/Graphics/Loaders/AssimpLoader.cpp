/// Pina Engine - Assimp Model Loader Implementation

#include "AssimpLoader.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/GltfMaterial.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <algorithm>
#include <cctype>

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
            return true;  // These formats typically need UV flip
        case ModelFormat::GLTF:
        case ModelFormat::PLY:
        case ModelFormat::STL:
        default:
            return false; // glTF already has correct UVs
    }
}

// ============================================================================
// Matrix Conversion
// ============================================================================

// Convert assimp matrix to GLM matrix
// aiMatrix4x4 uses row-major storage
// GLM uses column-major storage
static glm::mat4 aiToGlm(const aiMatrix4x4& m) {
    return glm::mat4(
        m.a1, m.a2, m.a3, m.a4,
        m.b1, m.b2, m.b3, m.b4,
        m.c1, m.c2, m.c3, m.c4,
        m.d1, m.d2, m.d3, m.d4
    );
}

// ============================================================================
// Main Load Function
// ============================================================================

UNIQUE<Model> AssimpLoader::load(GraphicsDevice* device, const std::string& path) {
    Assimp::Importer importer;

    // Detect format
    ModelFormat format = detectFormat(path);
    std::cout << "Loading model: " << path << " (format: " << static_cast<int>(format) << ")" << std::endl;

    // Base post-processing flags
    unsigned int flags =
        aiProcess_Triangulate |           // Convert to triangles
        aiProcess_GenSmoothNormals |      // Generate smooth normals if missing
        aiProcess_CalcTangentSpace |      // For normal mapping
        aiProcess_JoinIdenticalVertices | // Optimize mesh
        aiProcess_OptimizeMeshes |        // Reduce draw calls
        aiProcess_SortByPType |           // Split meshes by primitive type
        aiProcess_ValidateDataStructure;  // Check data integrity

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

    // Process scene graph with identity transform as root
    processNode(scene->mRootNode, scene, ctx, glm::mat4(1.0f));

    std::cout << "Loaded model: " << path << std::endl;
    std::cout << "  Meshes: " << model->m_meshes.size() << std::endl;
    std::cout << "  Materials: " << model->m_materials.size() << std::endl;
    std::cout << "  Textures: " << model->m_textures.size() << std::endl;
    std::cout << "  Bounds: " << model->getSize().x << " x " << model->getSize().y << " x " << model->getSize().z << std::endl;

    return model;
}

// ============================================================================
// Node Processing
// ============================================================================

void AssimpLoader::processNode(aiNode* node, const aiScene* scene, LoadContext& ctx, const glm::mat4& parentTransform) {
    // Calculate this node's world transform by accumulating parent transform
    glm::mat4 nodeTransform = aiToGlm(node->mTransformation);
    glm::mat4 worldTransform = parentTransform * nodeTransform;

    // Process meshes in this node with the accumulated transform
    for (unsigned int i = 0; i < node->mNumMeshes; ++i) {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        auto staticMesh = processMesh(mesh, ctx, worldTransform);
        if (staticMesh) {
            ctx.model->m_meshes.push_back(std::move(staticMesh));
            ctx.model->m_meshMaterialIndices.push_back(mesh->mMaterialIndex);
        }
    }

    // Recursively process child nodes with this node's world transform
    for (unsigned int i = 0; i < node->mNumChildren; ++i) {
        processNode(node->mChildren[i], scene, ctx, worldTransform);
    }
}

// ============================================================================
// Mesh Processing
// ============================================================================

UNIQUE<StaticMesh> AssimpLoader::processMesh(aiMesh* mesh, LoadContext& ctx, const glm::mat4& transform) {
    std::vector<float> vertices;
    std::vector<uint32_t> indices;

    // Check if mesh has required data
    if (mesh->mNumVertices == 0) {
        std::cerr << "Warning: Mesh has no vertices, skipping" << std::endl;
        return nullptr;
    }

    // Reserve space: pos(3) + normal(3) + uv(2) = 8 floats per vertex
    vertices.reserve(mesh->mNumVertices * 8);
    indices.reserve(mesh->mNumFaces * 3);

    // Calculate normal matrix for transforming normals
    glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(transform)));

    // Process vertices, applying the node hierarchy transform
    for (unsigned int i = 0; i < mesh->mNumVertices; ++i) {
        // Position - transform by node hierarchy
        glm::vec4 rawPos(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z, 1.0f);
        glm::vec3 pos = glm::vec3(transform * rawPos);
        vertices.push_back(pos.x);
        vertices.push_back(pos.y);
        vertices.push_back(pos.z);

        // Expand model's bounding box with transformed position
        ctx.model->m_boundingBox.expand(pos);

        // Normal - transform by normal matrix
        if (mesh->HasNormals()) {
            glm::vec3 rawNormal(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z);
            glm::vec3 normal = glm::normalize(normalMatrix * rawNormal);
            vertices.push_back(normal.x);
            vertices.push_back(normal.y);
            vertices.push_back(normal.z);
        } else {
            // Default up normal
            vertices.push_back(0.0f);
            vertices.push_back(1.0f);
            vertices.push_back(0.0f);
        }

        // Texture coordinates - use first UV channel
        if (mesh->mTextureCoords[0]) {
            vertices.push_back(mesh->mTextureCoords[0][i].x);
            vertices.push_back(mesh->mTextureCoords[0][i].y);
        } else {
            // Default UVs
            vertices.push_back(0.0f);
            vertices.push_back(0.0f);
        }
    }

    // Process indices
    for (unsigned int i = 0; i < mesh->mNumFaces; ++i) {
        aiFace& face = mesh->mFaces[i];
        // Only process triangles (aiProcess_Triangulate should ensure this)
        if (face.mNumIndices == 3) {
            indices.push_back(face.mIndices[0]);
            indices.push_back(face.mIndices[1]);
            indices.push_back(face.mIndices[2]);
        }
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

    // Get colors
    aiColor3D color;

    if (mat->Get(AI_MATKEY_COLOR_DIFFUSE, color) == AI_SUCCESS) {
        material.setDiffuse(Color(color.r, color.g, color.b));
    }

    if (mat->Get(AI_MATKEY_COLOR_SPECULAR, color) == AI_SUCCESS) {
        material.setSpecular(Color(color.r, color.g, color.b));
    }

    if (mat->Get(AI_MATKEY_COLOR_AMBIENT, color) == AI_SUCCESS) {
        material.setAmbient(Color(color.r, color.g, color.b));
    }

    if (mat->Get(AI_MATKEY_COLOR_EMISSIVE, color) == AI_SUCCESS) {
        material.setEmissive(Color(color.r, color.g, color.b));
    }

    // Get shininess
    float shininess = 32.0f;
    mat->Get(AI_MATKEY_SHININESS, shininess);
    material.setShininess(shininess > 0 ? shininess : 32.0f);

    // Load textures - try multiple texture types for compatibility

    // Diffuse/Base Color texture
    Texture* diffuseMap = loadMaterialTexture(mat, aiTextureType_DIFFUSE, ctx);
    if (!diffuseMap) {
        // Try PBR base color
        diffuseMap = loadMaterialTexture(mat, aiTextureType_BASE_COLOR, ctx);
    }
    if (diffuseMap) {
        material.setDiffuseMap(diffuseMap);
    }

    // Specular texture
    Texture* specularMap = loadMaterialTexture(mat, aiTextureType_SPECULAR, ctx);
    if (!specularMap) {
        // Try PBR metallic-roughness (for now just use as specular)
        specularMap = loadMaterialTexture(mat, aiTextureType_METALNESS, ctx);
    }
    if (specularMap) {
        material.setSpecularMap(specularMap);
    }

    // Normal texture - try multiple types
    Texture* normalMap = loadMaterialTexture(mat, aiTextureType_NORMALS, ctx);
    if (!normalMap) {
        normalMap = loadMaterialTexture(mat, aiTextureType_HEIGHT, ctx);
    }
    if (!normalMap) {
        normalMap = loadMaterialTexture(mat, aiTextureType_NORMAL_CAMERA, ctx);
    }
    if (normalMap) {
        material.setNormalMap(normalMap);
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

    // Check for embedded texture (starts with '*')
    if (texPathStr.length() > 0 && texPathStr[0] == '*') {
        // Embedded texture - extract index
        int texIndex = std::atoi(texPathStr.c_str() + 1);
        if (ctx.scene && texIndex >= 0 && static_cast<unsigned int>(texIndex) < ctx.scene->mNumTextures) {
            aiTexture* embeddedTex = ctx.scene->mTextures[texIndex];
            texture = loadEmbeddedTexture(embeddedTex, ctx);
        }
    } else {
        // External texture file - try multiple paths
        std::vector<std::string> pathsToTry;

        // Original path as-is
        pathsToTry.push_back(ctx.directory + "/" + texPathStr);

        // Just the filename (in case path is absolute or has wrong directory)
        size_t lastSlash = texPathStr.find_last_of("/\\");
        if (lastSlash != std::string::npos) {
            pathsToTry.push_back(ctx.directory + "/" + texPathStr.substr(lastSlash + 1));
        }

        // Try in textures subdirectory
        pathsToTry.push_back(ctx.directory + "/textures/" + texPathStr);
        if (lastSlash != std::string::npos) {
            pathsToTry.push_back(ctx.directory + "/textures/" + texPathStr.substr(lastSlash + 1));
        }

        for (const auto& tryPath : pathsToTry) {
            texture = Texture::load(ctx.device, tryPath);
            if (texture) {
                break;
            }
        }
    }

    if (!texture) {
        std::cerr << "Failed to load texture: " << texPathStr << std::endl;
        return nullptr;
    }

    // Store in model and cache
    size_t index = ctx.model->m_textures.size();
    ctx.loadedTextures[texPathStr] = index;
    Texture* texPtr = texture.get();
    ctx.model->m_textures.push_back(std::move(texture));

    return texPtr;
}

UNIQUE<Texture> AssimpLoader::loadEmbeddedTexture(aiTexture* tex, LoadContext& ctx) {
    if (!tex) return nullptr;

    // Check if texture is compressed (has height of 0)
    if (tex->mHeight == 0) {
        // Compressed format (PNG, JPG, etc.) - data is in pcData, size is mWidth bytes
        return Texture::loadFromMemory(ctx.device,
            reinterpret_cast<const unsigned char*>(tex->pcData),
            tex->mWidth);
    } else {
        // Raw ARGB8888 format
        // Convert to RGBA and create texture
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
