#pragma once

/// Pina Engine - Transform Component
/// Position, rotation, and scale with dirty flag caching for efficient matrix updates

#include "../Core/Export.h"
#include "../Core/Memory.h"
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace Pina {

class Node;

/// Transform component for scene nodes
/// Stores local transform (relative to parent) and caches world transform
class PINA_API Transform {
public:
    Transform();
    ~Transform() = default;

    // ========================================================================
    // Local Transform (relative to parent)
    // ========================================================================

    /// Set local position
    void setLocalPosition(const glm::vec3& position);
    void setLocalPosition(float x, float y, float z);

    /// Get local position
    const glm::vec3& getLocalPosition() const { return m_localPosition; }

    /// Set local rotation using Euler angles (in degrees)
    void setLocalRotationEuler(const glm::vec3& eulerDegrees);
    void setLocalRotationEuler(float pitch, float yaw, float roll);

    /// Get local rotation as Euler angles (in degrees)
    glm::vec3 getLocalRotationEuler() const;

    /// Set local rotation using quaternion
    void setLocalRotation(const glm::quat& rotation);

    /// Get local rotation as quaternion
    const glm::quat& getLocalRotation() const { return m_localRotation; }

    /// Set local scale
    void setLocalScale(const glm::vec3& scale);
    void setLocalScale(float uniformScale);
    void setLocalScale(float x, float y, float z);

    /// Get local scale
    const glm::vec3& getLocalScale() const { return m_localScale; }

    // ========================================================================
    // Transform Operations
    // ========================================================================

    /// Translate in local space
    void translate(const glm::vec3& delta);
    void translate(float x, float y, float z);

    /// Rotate by Euler angles (in degrees)
    void rotate(const glm::vec3& eulerDegrees);
    void rotate(float pitch, float yaw, float roll);

    /// Rotate around an axis (in degrees)
    void rotateAround(const glm::vec3& axis, float angleDegrees);

    /// Scale uniformly
    void scale(float factor);

    /// Scale non-uniformly
    void scale(const glm::vec3& factors);

    // ========================================================================
    // Matrix Access
    // ========================================================================

    /// Get local transform matrix (lazy evaluation)
    const glm::mat4& getLocalMatrix() const;

    /// Get world transform matrix (lazy evaluation, requires parent)
    const glm::mat4& getWorldMatrix() const;

    /// Get the normal matrix for transforming normals (inverse transpose of 3x3 world matrix)
    glm::mat3 getNormalMatrix() const;

    // ========================================================================
    // World Space Getters
    // ========================================================================

    /// Get world position
    glm::vec3 getWorldPosition() const;

    /// Get world rotation
    glm::quat getWorldRotation() const;

    /// Get world scale
    glm::vec3 getWorldScale() const;

    // ========================================================================
    // Direction Vectors
    // ========================================================================

    /// Get local forward direction (-Z in local space)
    glm::vec3 getForward() const;

    /// Get local right direction (+X in local space)
    glm::vec3 getRight() const;

    /// Get local up direction (+Y in local space)
    glm::vec3 getUp() const;

    // ========================================================================
    // Parent/Dirty Management
    // ========================================================================

    /// Mark transform as dirty (needs matrix recalculation)
    void markDirty();

    /// Check if transform is dirty
    bool isDirty() const { return m_dirty; }

    /// Set the owning node (for parent transform access)
    void setOwner(Node* node) { m_owner = node; }

private:
    void updateLocalMatrix() const;
    void updateWorldMatrix() const;

    // Local transform components
    glm::vec3 m_localPosition{0.0f, 0.0f, 0.0f};
    glm::quat m_localRotation{1.0f, 0.0f, 0.0f, 0.0f};  // Identity quaternion (w, x, y, z)
    glm::vec3 m_localScale{1.0f, 1.0f, 1.0f};

    // Cached matrices
    mutable glm::mat4 m_localMatrix{1.0f};
    mutable glm::mat4 m_worldMatrix{1.0f};

    // Dirty flags
    mutable bool m_dirty = true;
    mutable bool m_worldDirty = true;

    // Owner node (for parent access)
    Node* m_owner = nullptr;
};

} // namespace Pina
