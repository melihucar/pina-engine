#pragma once

/// Pina Engine - Model
/// Container for 3D models with multiple meshes and materials

#include "../Core/Export.h"
#include "../Core/Memory.h"
#include "GraphicsDevice.h"
#include "Material.h"
#include "Texture.h"
#include "Shader.h"
#include "Primitives/StaticMesh.h"
#include "Lighting/LightManager.h"
#include <vector>
#include <string>
#include <glm/glm.hpp>

namespace Pina {

/// Axis-aligned bounding box
struct PINA_API BoundingBox {
    glm::vec3 min{std::numeric_limits<float>::max()};
    glm::vec3 max{std::numeric_limits<float>::lowest()};

    /// Expand the bounding box to include a point
    void expand(const glm::vec3& point) {
        min = glm::min(min, point);
        max = glm::max(max, point);
    }

    /// Get center of the bounding box
    glm::vec3 getCenter() const {
        return (min + max) * 0.5f;
    }

    /// Get size (extent) of the bounding box
    glm::vec3 getSize() const {
        return max - min;
    }

    /// Get the maximum dimension (largest of width, height, depth)
    float getMaxDimension() const {
        glm::vec3 size = getSize();
        return glm::max(glm::max(size.x, size.y), size.z);
    }

    /// Check if bounding box is valid (has been expanded at least once)
    bool isValid() const {
        return min.x <= max.x && min.y <= max.y && min.z <= max.z;
    }
};

/// 3D model container
/// Holds multiple meshes with their associated materials and textures
class PINA_API Model {
public:
    ~Model() = default;

    /// Load a 3D model from file
    /// Supports OBJ, glTF, FBX, COLLADA, and 50+ other formats via assimp
    /// @param device Graphics device for creating GPU resources
    /// @param path Path to the model file
    /// @return Loaded model, or nullptr on failure
    static UNIQUE<Model> load(GraphicsDevice* device, const std::string& path);

    /// Draw the model
    /// Binds each mesh's material and draws it
    /// @param shader Bound shader to upload material uniforms to
    /// @param lightManager Light manager for material uploads
    void draw(Shader* shader, LightManager* lightManager);

    // ========================================================================
    // Mesh Access
    // ========================================================================

    /// Get number of meshes in the model
    size_t getMeshCount() const { return m_meshes.size(); }

    /// Get mesh by index
    StaticMesh* getMesh(size_t index);
    const StaticMesh* getMesh(size_t index) const;

    // ========================================================================
    // Material Access
    // ========================================================================

    /// Get number of materials
    size_t getMaterialCount() const { return m_materials.size(); }

    /// Get material by index
    Material* getMaterial(size_t index);
    const Material* getMaterial(size_t index) const;

    /// Check if any material uses PBR workflow
    bool hasPBRMaterials() const;

    // ========================================================================
    // Info
    // ========================================================================

    /// Get the directory containing the model file
    const std::string& getDirectory() const { return m_directory; }

    /// Get the original file path
    const std::string& getPath() const { return m_path; }

    // ========================================================================
    // Bounding Box
    // ========================================================================

    /// Get the model's bounding box (in model space)
    const BoundingBox& getBoundingBox() const { return m_boundingBox; }

    /// Get the center of the model
    glm::vec3 getCenter() const { return m_boundingBox.getCenter(); }

    /// Get the size of the model
    glm::vec3 getSize() const { return m_boundingBox.getSize(); }

    /// Calculate scale factor to fit model within a target size
    /// @param targetSize Maximum dimension the model should fit within
    /// @return Scale factor to apply
    float calculateFitScale(float targetSize = 2.0f) const {
        float maxDim = m_boundingBox.getMaxDimension();
        return maxDim > 0.0f ? targetSize / maxDim : 1.0f;
    }

private:
    friend class AssimpLoader;
    friend class TinyGLTFLoader;

    Model() = default;

    std::vector<UNIQUE<StaticMesh>> m_meshes;
    std::vector<Material> m_materials;
    std::vector<size_t> m_meshMaterialIndices;  // Material index for each mesh
    std::vector<UNIQUE<Texture>> m_textures;    // Owned textures

    std::string m_path;
    std::string m_directory;
    BoundingBox m_boundingBox;
};

} // namespace Pina
