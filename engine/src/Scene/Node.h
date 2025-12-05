#pragma once

/// Pina Engine - Scene Node
/// Represents a node in the scene hierarchy with transform, children, and optional model

#include "../Core/Export.h"
#include "../Core/Memory.h"
#include "Transform.h"
#include <string>
#include <vector>
#include <functional>
#include <cstdint>

namespace Pina {

class Model;
class Scene;

/// Scene node representing an object in the scene hierarchy
class PINA_API Node {
public:
    /// Create a node with a name
    explicit Node(const std::string& name = "Node");
    ~Node();

    // Non-copyable, moveable
    Node(const Node&) = delete;
    Node& operator=(const Node&) = delete;
    Node(Node&&) = default;
    Node& operator=(Node&&) = default;

    // ========================================================================
    // Identity
    // ========================================================================

    /// Get unique node ID
    uint64_t getID() const { return m_id; }

    /// Get node name
    const std::string& getName() const { return m_name; }

    /// Set node name
    void setName(const std::string& name) { m_name = name; }

    // ========================================================================
    // Enable State
    // ========================================================================

    /// Check if node is enabled
    bool isEnabled() const { return m_enabled; }

    /// Set enabled state
    void setEnabled(bool enabled) { m_enabled = enabled; }

    /// Check if node is enabled in hierarchy (self and all ancestors enabled)
    bool isEnabledInHierarchy() const;

    // ========================================================================
    // Transform
    // ========================================================================

    /// Get transform component
    Transform& getTransform() { return m_transform; }
    const Transform& getTransform() const { return m_transform; }

    // ========================================================================
    // Hierarchy
    // ========================================================================

    /// Get parent node (nullptr if root)
    Node* getParent() const { return m_parent; }

    /// Set parent node (removes from old parent, adds to new)
    void setParent(Node* newParent);

    /// Get child count
    size_t getChildCount() const { return m_children.size(); }

    /// Get child by index
    Node* getChild(size_t index);
    const Node* getChild(size_t index) const;

    /// Get all children
    const std::vector<UNIQUE<Node>>& getChildren() const { return m_children; }

    /// Add a new child with name
    /// @return Pointer to the newly created child
    Node* addChild(const std::string& name = "Node");

    /// Add an existing node as child (takes ownership)
    /// @return Pointer to the added child
    Node* addChild(UNIQUE<Node> child);

    /// Remove a child by pointer
    /// @return The removed child (transfers ownership)
    UNIQUE<Node> removeChild(Node* child);

    /// Remove a child by index
    /// @return The removed child (transfers ownership)
    UNIQUE<Node> removeChild(size_t index);

    /// Remove all children
    void removeAllChildren();

    /// Find a child by name (direct children only)
    Node* findChild(const std::string& name) const;

    /// Find a descendant by name (recursive)
    Node* findDescendant(const std::string& name) const;

    // ========================================================================
    // Traversal
    // ========================================================================

    /// Traverse this node and all descendants
    /// @param callback Function called for each node (return false to stop)
    void traverse(const std::function<void(Node*)>& callback);

    /// Traverse this node and all descendants (const version)
    void traverse(const std::function<void(const Node*)>& callback) const;

    /// Traverse only enabled nodes
    void traverseEnabled(const std::function<void(Node*)>& callback);

    // ========================================================================
    // Model Attachment
    // ========================================================================

    /// Attach a model to this node (does NOT take ownership)
    void setModel(Model* model) { m_model = model; }

    /// Get attached model (may be nullptr)
    Model* getModel() const { return m_model; }

    /// Check if node has a model attached
    bool hasModel() const { return m_model != nullptr; }

    // ========================================================================
    // Scene
    // ========================================================================

    /// Get the owning scene (may be nullptr if not added to scene)
    Scene* getScene() const { return m_scene; }

    // ========================================================================
    // Internal
    // ========================================================================

    /// Mark world transforms of all children as dirty
    void markChildrenWorldDirty();

private:
    friend class Scene;

    void setScene(Scene* scene);

    static uint64_t s_nextID;

    uint64_t m_id;
    std::string m_name;
    bool m_enabled = true;

    Transform m_transform;
    Node* m_parent = nullptr;
    std::vector<UNIQUE<Node>> m_children;

    Model* m_model = nullptr;  // Non-owning pointer
    Scene* m_scene = nullptr;  // Owning scene
};

} // namespace Pina
