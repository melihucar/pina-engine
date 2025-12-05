#pragma once

/// Pina Engine - Assimp Model Loader
/// Loads 3D models using the assimp library
/// Supports OBJ, glTF 2.0 (including GLB), FBX, COLLADA, 3DS, PLY, STL, and 50+ formats

#include "../Model.h"
#include "../GraphicsDevice.h"
#include "../Material.h"
#include "../Texture.h"
#include "../Primitives/StaticMesh.h"
#include "../../Core/Memory.h"
#include <string>
#include <vector>
#include <unordered_map>
#include <glm/glm.hpp>

// Forward declarations for assimp types
struct aiScene;
struct aiNode;
struct aiMesh;
struct aiMaterial;
struct aiTexture;

namespace Pina {

/// Assimp-based model loader
/// Supports OBJ, glTF, FBX, COLLADA, and 50+ other formats
class AssimpLoader {
public:
    /// Load a model from file
    /// @param device Graphics device for creating resources
    /// @param path Path to the model file
    /// @return Loaded model, or nullptr on failure
    static UNIQUE<Model> load(GraphicsDevice* device, const std::string& path);

private:
    /// Internal loading context
    struct LoadContext {
        GraphicsDevice* device;
        Model* model;
        std::string directory;
        std::unordered_map<std::string, size_t> loadedTextures;  // path -> texture index
        const aiScene* scene;  // For accessing embedded textures
        int format;            // ModelFormat enum value (internal)
    };

    static void processNode(aiNode* node, const aiScene* scene, LoadContext& ctx, const glm::mat4& parentTransform);
    static UNIQUE<StaticMesh> processMesh(aiMesh* mesh, LoadContext& ctx, const glm::mat4& transform);
    static Material processMaterial(aiMaterial* mat, LoadContext& ctx);
    static Texture* loadMaterialTexture(aiMaterial* mat, int type, LoadContext& ctx);
    static UNIQUE<Texture> loadEmbeddedTexture(aiTexture* tex, LoadContext& ctx);
};

} // namespace Pina
