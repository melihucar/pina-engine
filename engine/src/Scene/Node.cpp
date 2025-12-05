#include "Node.h"
#include "Scene.h"
#include <algorithm>

namespace Pina {

uint64_t Node::s_nextID = 1;

Node::Node(const std::string& name)
    : m_id(s_nextID++)
    , m_name(name)
{
    m_transform.setOwner(this);
}

Node::~Node() = default;

// ============================================================================
// Enable State
// ============================================================================

bool Node::isEnabledInHierarchy() const {
    if (!m_enabled) return false;
    if (m_parent) return m_parent->isEnabledInHierarchy();
    return true;
}

// ============================================================================
// Hierarchy
// ============================================================================

void Node::setParent(Node* newParent) {
    if (m_parent == newParent) return;

    // Prevent circular reference
    if (newParent) {
        Node* check = newParent;
        while (check) {
            if (check == this) return;  // Would create cycle
            check = check->m_parent;
        }
    }

    // Remove from old parent
    if (m_parent) {
        auto& siblings = m_parent->m_children;
        for (auto it = siblings.begin(); it != siblings.end(); ++it) {
            if (it->get() == this) {
                it->release();  // Release ownership without deleting
                siblings.erase(it);
                break;
            }
        }
    }

    m_parent = newParent;

    // Add to new parent
    if (newParent) {
        newParent->m_children.push_back(UNIQUE<Node>(this));
        setScene(newParent->m_scene);
    }

    // Mark transform dirty since parent changed
    m_transform.markDirty();
}

Node* Node::getChild(size_t index) {
    if (index >= m_children.size()) return nullptr;
    return m_children[index].get();
}

const Node* Node::getChild(size_t index) const {
    if (index >= m_children.size()) return nullptr;
    return m_children[index].get();
}

Node* Node::addChild(const std::string& name) {
    auto child = MAKE_UNIQUE<Node>(name);
    child->m_parent = this;
    child->setScene(m_scene);
    child->m_transform.setOwner(child.get());
    Node* ptr = child.get();
    m_children.push_back(std::move(child));
    return ptr;
}

Node* Node::addChild(UNIQUE<Node> child) {
    if (!child) return nullptr;

    // Remove from old parent if any
    if (child->m_parent) {
        auto& siblings = child->m_parent->m_children;
        for (auto it = siblings.begin(); it != siblings.end(); ++it) {
            if (it->get() == child.get()) {
                it->release();  // Release without deleting
                siblings.erase(it);
                break;
            }
        }
    }

    child->m_parent = this;
    child->setScene(m_scene);
    child->m_transform.markDirty();
    Node* ptr = child.get();
    m_children.push_back(std::move(child));
    return ptr;
}

UNIQUE<Node> Node::removeChild(Node* child) {
    for (auto it = m_children.begin(); it != m_children.end(); ++it) {
        if (it->get() == child) {
            UNIQUE<Node> removed = std::move(*it);
            m_children.erase(it);
            removed->m_parent = nullptr;
            removed->setScene(nullptr);
            return removed;
        }
    }
    return nullptr;
}

UNIQUE<Node> Node::removeChild(size_t index) {
    if (index >= m_children.size()) return nullptr;

    auto it = m_children.begin() + static_cast<ptrdiff_t>(index);
    UNIQUE<Node> removed = std::move(*it);
    m_children.erase(it);
    removed->m_parent = nullptr;
    removed->setScene(nullptr);
    return removed;
}

void Node::removeAllChildren() {
    for (auto& child : m_children) {
        child->m_parent = nullptr;
        child->setScene(nullptr);
    }
    m_children.clear();
}

Node* Node::findChild(const std::string& name) const {
    for (const auto& child : m_children) {
        if (child->m_name == name) {
            return child.get();
        }
    }
    return nullptr;
}

Node* Node::findDescendant(const std::string& name) const {
    // Check direct children first
    for (const auto& child : m_children) {
        if (child->m_name == name) {
            return child.get();
        }
    }

    // Recurse into children
    for (const auto& child : m_children) {
        Node* found = child->findDescendant(name);
        if (found) return found;
    }

    return nullptr;
}

// ============================================================================
// Traversal
// ============================================================================

void Node::traverse(const std::function<void(Node*)>& callback) {
    callback(this);
    for (auto& child : m_children) {
        child->traverse(callback);
    }
}

void Node::traverse(const std::function<void(const Node*)>& callback) const {
    callback(this);
    for (const auto& child : m_children) {
        const Node* childPtr = child.get();
        childPtr->traverse(callback);
    }
}

void Node::traverseEnabled(const std::function<void(Node*)>& callback) {
    if (!m_enabled) return;
    callback(this);
    for (auto& child : m_children) {
        child->traverseEnabled(callback);
    }
}

// ============================================================================
// Internal
// ============================================================================

void Node::markChildrenWorldDirty() {
    for (auto& child : m_children) {
        child->m_transform.markDirty();
    }
}

void Node::setScene(Scene* scene) {
    if (m_scene == scene) return;

    // Unregister from old scene
    if (m_scene) {
        m_scene->unregisterNode(this);
    }

    m_scene = scene;

    // Register with new scene
    if (m_scene) {
        m_scene->registerNode(this);
    }

    // Propagate to children
    for (auto& child : m_children) {
        child->setScene(scene);
    }
}

} // namespace Pina
