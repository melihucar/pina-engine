/// Pina Engine - Assimp Model Loader Implementation

#include "AssimpLoader.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

namespace Pina {

UNIQUE<Model> AssimpLoader::load(GraphicsDevice* device, const std::string& path) {
    Assimp::Importer importer;

    // Import with common post-processing flags
    unsigned int flags =
        aiProcess_Triangulate |           // Convert to triangles
        aiProcess_GenNormals |            // Generate normals if missing
        aiProcess_FlipUVs |               // Flip UVs for OpenGL
        aiProcess_CalcTangentSpace |      // For normal mapping
        aiProcess_JoinIdenticalVertices | // Optimize mesh
        aiProcess_OptimizeMeshes |        // Reduce draw calls
        aiProcess_PreTransformVertices;   // Apply node transforms to vertices

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

void AssimpLoader::processNode(aiNode* node, const aiScene* scene, LoadContext& ctx, const glm::mat4& /*parentTransform*/) {
    // With aiProcess_PreTransformVertices, all transforms are already applied to vertices
    // We just need to traverse and collect meshes

    // Process meshes in this node
    for (unsigned int i = 0; i < node->mNumMeshes; ++i) {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        auto staticMesh = processMesh(mesh, ctx, glm::mat4(1.0f));
        if (staticMesh) {
            ctx.model->m_meshes.push_back(std::move(staticMesh));
            ctx.model->m_meshMaterialIndices.push_back(mesh->mMaterialIndex);
        }
    }

    // Recursively process child nodes
    for (unsigned int i = 0; i < node->mNumChildren; ++i) {
        processNode(node->mChildren[i], scene, ctx, glm::mat4(1.0f));
    }
}

UNIQUE<StaticMesh> AssimpLoader::processMesh(aiMesh* mesh, LoadContext& ctx, const glm::mat4& /*transform*/) {
    std::vector<float> vertices;
    std::vector<uint32_t> indices;

    // Reserve space
    vertices.reserve(mesh->mNumVertices * 8);  // pos(3) + normal(3) + uv(2)
    indices.reserve(mesh->mNumFaces * 3);

    // With aiProcess_PreTransformVertices, vertex positions and normals are already in world space
    // Process vertices
    for (unsigned int i = 0; i < mesh->mNumVertices; ++i) {
        // Position (already transformed by assimp)
        glm::vec3 pos(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z);
        vertices.push_back(pos.x);
        vertices.push_back(pos.y);
        vertices.push_back(pos.z);

        // Expand model's bounding box
        ctx.model->m_boundingBox.expand(pos);

        // Normal (already transformed by assimp)
        if (mesh->HasNormals()) {
            vertices.push_back(mesh->mNormals[i].x);
            vertices.push_back(mesh->mNormals[i].y);
            vertices.push_back(mesh->mNormals[i].z);
        } else {
            vertices.push_back(0.0f);
            vertices.push_back(1.0f);
            vertices.push_back(0.0f);
        }

        // Texture coordinates
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
