#pragma once

/// Pina Engine - Scene
/// Container for a scene hierarchy with root node, camera, and lights

#include "../Core/Export.h"
#include "../Core/Memory.h"
#include "../Graphics/Lighting/LightManager.h"
#include "Node.h"
#include <string>
#include <unordered_map>
#include <functional>

namespace Pina {

class Camera;

/// Scene container for 3D objects, camera, and lights
class PINA_API Scene {
public:
    Scene();
    ~Scene();

    // Non-copyable
    Scene(const Scene&) = delete;
    Scene& operator=(const Scene&) = delete;

    // ========================================================================
    // Root Node
    // ========================================================================

    /// Get the root node of the scene
    Node* getRoot() { return m_root.get(); }
    const Node* getRoot() const { return m_root.get(); }

    // ========================================================================
    // Node Creation
    // ========================================================================

    /// Create a new node as child of root
    /// @param name Node name
    /// @return Pointer to newly created node
    Node* createNode(const std::string& name = "Node");

    /// Create a new node as child of specified parent
    /// @param name Node name
    /// @param parent Parent node (nullptr = root)
    /// @return Pointer to newly created node
    Node* createNode(const std::string& name, Node* parent);

    // ========================================================================
    // Node Lookup
    // ========================================================================

    /// Find a node by ID
    /// @param id Node ID
    /// @return Node pointer or nullptr if not found
    Node* findNode(uint64_t id) const;

    /// Find a node by name (first match)
    /// @param name Node name
    /// @return Node pointer or nullptr if not found
    Node* findNode(const std::string& name) const;

    /// Get total number of nodes in scene
    size_t getNodeCount() const { return m_nodesByID.size(); }

    // ========================================================================
    // Traversal
    // ========================================================================

    /// Traverse all nodes in the scene
    void traverse(const std::function<void(Node*)>& callback);

    /// Traverse all nodes (const version)
    void traverse(const std::function<void(const Node*)>& callback) const;

    /// Traverse only enabled nodes
    void traverseEnabled(const std::function<void(Node*)>& callback);

    // ========================================================================
    // Camera
    // ========================================================================

    /// Set the active camera for rendering
    void setActiveCamera(Camera* camera) { m_activeCamera = camera; }

    /// Get the active camera
    Camera* getActiveCamera() const { return m_activeCamera; }

    // ========================================================================
    // Lighting
    // ========================================================================

    /// Get the light manager
    LightManager& getLightManager() { return m_lightManager; }
    const LightManager& getLightManager() const { return m_lightManager; }

    // ========================================================================
    // Update
    // ========================================================================

    /// Update the scene (called each frame)
    /// Updates light manager and other per-frame state
    void update(float deltaTime);

    // ========================================================================
    // Node Registry (Internal)
    // ========================================================================

    /// Register a node with the scene (called by Node)
    void registerNode(Node* node);

    /// Unregister a node from the scene (called by Node)
    void unregisterNode(Node* node);

private:
    UNIQUE<Node> m_root;
    Camera* m_activeCamera = nullptr;
    LightManager m_lightManager;

    // Node lookup by ID
    std::unordered_map<uint64_t, Node*> m_nodesByID;
};

} // namespace Pina
