/// Pina Engine - Model Implementation

#include "Model.h"
#include "Loaders/AssimpLoader.h"
#include <iostream>

namespace Pina {

UNIQUE<Model> Model::load(GraphicsDevice* device, const std::string& path) {
    return AssimpLoader::load(device, path);
}

void Model::draw(Shader* shader, LightManager* lightManager) {
    for (size_t i = 0; i < m_meshes.size(); ++i) {
        // Get material for this mesh
        size_t materialIndex = i < m_meshMaterialIndices.size() ? m_meshMaterialIndices[i] : 0;
        if (materialIndex < m_materials.size()) {
            lightManager->uploadMaterial(shader, m_materials[materialIndex]);
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

} // namespace Pina
