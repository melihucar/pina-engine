/// Pina Engine - Assimp Model Loader Implementation

#include "AssimpLoader.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

namespace Pina {

// Convert assimp matrix to GLM matrix
// aiMatrix4x4 uses row-major with row vectors (v * M)
// GLM uses column-major with column vectors (M * v)
// Direct copy without transpose works for OpenGL
static glm::mat4 aiToGlm(const aiMatrix4x4& m) {
    return glm::mat4(
        m.a1, m.a2, m.a3, m.a4,
        m.b1, m.b2, m.b3, m.b4,
        m.c1, m.c2, m.c3, m.c4,
        m.d1, m.d2, m.d3, m.d4
    );
}

UNIQUE<Model> AssimpLoader::load(GraphicsDevice* device, const std::string& path) {
    Assimp::Importer importer;

    // Import with common post-processing flags
    unsigned int flags =
        aiProcess_Triangulate |           // Convert to triangles
        aiProcess_GenNormals |            // Generate normals if missing
        aiProcess_CalcTangentSpace |      // For normal mapping
        aiProcess_JoinIdenticalVertices | // Optimize mesh
        aiProcess_OptimizeMeshes;         // Reduce draw calls
        // Note: NOT using aiProcess_FlipUVs - glTF already has correct UV orientation
        // Note: NOT using aiProcess_PreTransformVertices - we apply node transforms manually

    const aiScene* scene = importer.ReadFile(path, flags);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        std::cerr << "Assimp error: " << importer.GetErrorString() << std::endl;
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

    // Process materials first
    for (unsigned int i = 0; i < scene->mNumMaterials; ++i) {
        Material mat = processMaterial(scene->mMaterials[i], scene, ctx);
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

    return model;
}

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

UNIQUE<StaticMesh> AssimpLoader::processMesh(aiMesh* mesh, LoadContext& ctx, const glm::mat4& transform) {
    std::vector<float> vertices;
    std::vector<uint32_t> indices;

    // Reserve space
    vertices.reserve(mesh->mNumVertices * 8);  // pos(3) + normal(3) + uv(2)
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
            vertices.push_back(0.0f);
            vertices.push_back(1.0f);
            vertices.push_back(0.0f);
        }

        // Texture coordinates (unchanged)
        if (mesh->mTextureCoords[0]) {
            vertices.push_back(mesh->mTextureCoords[0][i].x);
            vertices.push_back(mesh->mTextureCoords[0][i].y);
        } else {
            vertices.push_back(0.0f);
            vertices.push_back(0.0f);
        }
    }

    // Process indices
    for (unsigned int i = 0; i < mesh->mNumFaces; ++i) {
        aiFace& face = mesh->mFaces[i];
        for (unsigned int j = 0; j < face.mNumIndices; ++j) {
            indices.push_back(face.mIndices[j]);
        }
    }

    return StaticMesh::create(ctx.device, vertices, indices);
}

Material AssimpLoader::processMaterial(aiMaterial* mat, const aiScene* scene, LoadContext& ctx) {
    Material material;

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

    // Load diffuse texture
    Texture* diffuseMap = loadMaterialTexture(mat, aiTextureType_DIFFUSE, ctx);
    if (diffuseMap) {
        material.setDiffuseMap(diffuseMap);
    }

    // Load specular texture
    Texture* specularMap = loadMaterialTexture(mat, aiTextureType_SPECULAR, ctx);
    if (specularMap) {
        material.setSpecularMap(specularMap);
    }

    // Load normal texture
    Texture* normalMap = loadMaterialTexture(mat, aiTextureType_NORMALS, ctx);
    if (!normalMap) {
        // Some exporters use HEIGHT for normal maps
        normalMap = loadMaterialTexture(mat, aiTextureType_HEIGHT, ctx);
    }
    if (normalMap) {
        material.setNormalMap(normalMap);
    }

    return material;
}

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

    // Build full path
    std::string fullPath = ctx.directory + "/" + texPathStr;

    // Try to load texture
    auto texture = Texture::load(ctx.device, fullPath);
    if (!texture) {
        std::cerr << "Failed to load texture: " << fullPath << std::endl;
        return nullptr;
    }

    // Store in model and cache
    size_t index = ctx.model->m_textures.size();
    ctx.loadedTextures[texPathStr] = index;
    Texture* texPtr = texture.get();
    ctx.model->m_textures.push_back(std::move(texture));

    return texPtr;
}

} // namespace Pina
