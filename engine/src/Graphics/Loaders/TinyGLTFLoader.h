#pragma once

/// Pina Engine - TinyGLTF Model Loader
/// Direct glTF loading without Assimp for debugging

#include "../Model.h"
#include "../GraphicsDevice.h"
#include "../../Core/Memory.h"
#include <string>
#include <glm/glm.hpp>

// Forward declarations for TinyGLTF types
namespace tinygltf {
    class Model;
    class Node;
    struct Primitive;
}

namespace Pina {

/// Loading context passed to helper functions
struct TinyGLTFLoadContext {
    GraphicsDevice* device;
    Model* model;
    const tinygltf::Model* gltfModel;
};

/// TinyGLTF-based model loader for glTF/GLB files
class TinyGLTFLoader {
public:
    /// Load a glTF/GLB model from file
    static UNIQUE<Model> load(GraphicsDevice* device, const std::string& path);

private:
    /// Get the local transform matrix for a node
    static glm::mat4 getNodeLocalTransform(const tinygltf::Node& node);

    /// Process a single primitive with transform applied
    static UNIQUE<StaticMesh> processPrimitive(
        const tinygltf::Model& gltfModel,
        const tinygltf::Primitive& primitive,
        const glm::mat4& transform,
        TinyGLTFLoadContext& ctx,
        size_t meshIdx,
        size_t primIdx);

    /// Recursively process a node and its children
    static void processNode(
        const tinygltf::Model& gltfModel,
        int nodeIdx,
        const glm::mat4& parentTransform,
        TinyGLTFLoadContext& ctx);
};

} // namespace Pina
