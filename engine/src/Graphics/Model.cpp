/// Pina Engine - Model Implementation

#include "Model.h"
#include "Loaders/AssimpLoader.h"
#include "Loaders/TinyGLTFLoader.h"
#include <iostream>
#include <algorithm>
#include <cctype>

namespace Pina {

UNIQUE<Model> Model::load(GraphicsDevice* device, const std::string& path) {
    // Use Assimp for all formats (testing)
    return AssimpLoader::load(device, path);
}

void Model::draw(Shader* shader, LightManager* lightManager) {
    for (size_t i = 0; i < m_meshes.size(); ++i) {
        // Get material for this mesh
        size_t materialIndex = i < m_meshMaterialIndices.size() ? m_meshMaterialIndices[i] : 0;
        if (materialIndex < m_materials.size()) {
            const Material& mat = m_materials[materialIndex];
            // Use PBR upload for PBR materials, Blinn-Phong for others
            if (mat.isPBR()) {
                lightManager->uploadPBRMaterial(shader, mat);
            } else {
                lightManager->uploadMaterial(shader, mat);
            }
        }

        // Draw mesh
        m_meshes[i]->draw();
    }
}

StaticMesh* Model::getMesh(size_t index) {
    if (index >= m_meshes.size()) return nullptr;
    return m_meshes[index].get();
}

const StaticMesh* Model::getMesh(size_t index) const {
    if (index >= m_meshes.size()) return nullptr;
    return m_meshes[index].get();
}

Material* Model::getMaterial(size_t index) {
    if (index >= m_materials.size()) return nullptr;
    return &m_materials[index];
}

const Material* Model::getMaterial(size_t index) const {
    if (index >= m_materials.size()) return nullptr;
    return &m_materials[index];
}

bool Model::hasPBRMaterials() const {
    for (const auto& material : m_materials) {
        if (material.isPBR()) {
            return true;
        }
    }
    return false;
}

bool Model::hasTransparentMaterials() const {
    for (const auto& material : m_materials) {
        if (material.isTransparent()) {
            return true;
        }
    }
    return false;
}

void Model::drawOpaque(Shader* shader, LightManager* lightManager) {
    for (size_t i = 0; i < m_meshes.size(); ++i) {
        // Get material for this mesh
        size_t materialIndex = i < m_meshMaterialIndices.size() ? m_meshMaterialIndices[i] : 0;
        if (materialIndex < m_materials.size()) {
            const Material& mat = m_materials[materialIndex];

            // Skip transparent materials
            if (mat.isTransparent()) continue;

            // Upload material
            if (mat.isPBR()) {
                lightManager->uploadPBRMaterial(shader, mat);
            } else {
                lightManager->uploadMaterial(shader, mat);
            }
        }

        // Draw mesh
        size_t matIdx = i < m_meshMaterialIndices.size() ? m_meshMaterialIndices[i] : 0;
        if (matIdx < m_materials.size() && m_materials[matIdx].isTransparent()) continue;
        m_meshes[i]->draw();
    }
}

void Model::drawTransparent(Shader* shader, LightManager* lightManager) {
    for (size_t i = 0; i < m_meshes.size(); ++i) {
        // Get material for this mesh
        size_t materialIndex = i < m_meshMaterialIndices.size() ? m_meshMaterialIndices[i] : 0;
        if (materialIndex < m_materials.size()) {
            const Material& mat = m_materials[materialIndex];

            // Skip opaque materials
            if (!mat.isTransparent()) continue;

            // Upload material
            if (mat.isPBR()) {
                lightManager->uploadPBRMaterial(shader, mat);
            } else {
                lightManager->uploadMaterial(shader, mat);
            }

            // Draw mesh
            m_meshes[i]->draw();
        }
    }
}

} // namespace Pina
