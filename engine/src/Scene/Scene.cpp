#include "Scene.h"
#include "../Graphics/Camera.h"

namespace Pina {

Scene::Scene() {
    // Create root node
    m_root = MAKE_UNIQUE<Node>("Root");
    m_root->setScene(this);
}

Scene::~Scene() = default;

// ============================================================================
// Node Creation
// ============================================================================

Node* Scene::createNode(const std::string& name) {
    return createNode(name, m_root.get());
}

Node* Scene::createNode(const std::string& name, Node* parent) {
    Node* parentNode = parent ? parent : m_root.get();
    return parentNode->addChild(name);
}

// ============================================================================
// Node Lookup
// ============================================================================

Node* Scene::findNode(uint64_t id) const {
    auto it = m_nodesByID.find(id);
    if (it != m_nodesByID.end()) {
        return it->second;
    }
    return nullptr;
}

Node* Scene::findNode(const std::string& name) const {
    return m_root->findDescendant(name);
}

// ============================================================================
// Traversal
// ============================================================================

void Scene::traverse(const std::function<void(Node*)>& callback) {
    m_root->traverse(callback);
}

void Scene::traverse(const std::function<void(const Node*)>& callback) const {
    const Node* root = m_root.get();
    root->traverse(callback);
}

void Scene::traverseEnabled(const std::function<void(Node*)>& callback) {
    m_root->traverseEnabled(callback);
}

// ============================================================================
// Update
// ============================================================================

void Scene::update(float deltaTime) {
    (void)deltaTime;

    // Update light manager with camera position for specular calculations
    if (m_activeCamera) {
        m_lightManager.setViewPosition(m_activeCamera->getPosition());
    }

    // Update light data
    m_lightManager.update();
}

// ============================================================================
// Node Registry
// ============================================================================

void Scene::registerNode(Node* node) {
    if (node) {
        m_nodesByID[node->getID()] = node;
    }
}

void Scene::unregisterNode(Node* node) {
    if (node) {
        m_nodesByID.erase(node->getID());
    }
}

} // namespace Pina
