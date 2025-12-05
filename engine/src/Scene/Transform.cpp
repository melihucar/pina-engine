#include "Transform.h"
#include "Node.h"
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/matrix_decompose.hpp>

namespace Pina {

Transform::Transform() = default;

// ============================================================================
// Local Position
// ============================================================================

void Transform::setLocalPosition(const glm::vec3& position) {
    m_localPosition = position;
    markDirty();
}

void Transform::setLocalPosition(float x, float y, float z) {
    setLocalPosition(glm::vec3(x, y, z));
}

// ============================================================================
// Local Rotation
// ============================================================================

void Transform::setLocalRotationEuler(const glm::vec3& eulerDegrees) {
    glm::vec3 radians = glm::radians(eulerDegrees);
    m_localRotation = glm::quat(radians);
    markDirty();
}

void Transform::setLocalRotationEuler(float pitch, float yaw, float roll) {
    setLocalRotationEuler(glm::vec3(pitch, yaw, roll));
}

glm::vec3 Transform::getLocalRotationEuler() const {
    return glm::degrees(glm::eulerAngles(m_localRotation));
}

void Transform::setLocalRotation(const glm::quat& rotation) {
    m_localRotation = rotation;
    markDirty();
}

// ============================================================================
// Local Scale
// ============================================================================

void Transform::setLocalScale(const glm::vec3& scale) {
    m_localScale = scale;
    markDirty();
}

void Transform::setLocalScale(float uniformScale) {
    setLocalScale(glm::vec3(uniformScale));
}

void Transform::setLocalScale(float x, float y, float z) {
    setLocalScale(glm::vec3(x, y, z));
}

// ============================================================================
// Transform Operations
// ============================================================================

void Transform::translate(const glm::vec3& delta) {
    m_localPosition += delta;
    markDirty();
}

void Transform::translate(float x, float y, float z) {
    translate(glm::vec3(x, y, z));
}

void Transform::rotate(const glm::vec3& eulerDegrees) {
    glm::vec3 radians = glm::radians(eulerDegrees);
    glm::quat rotation(radians);
    m_localRotation = rotation * m_localRotation;
    markDirty();
}

void Transform::rotate(float pitch, float yaw, float roll) {
    rotate(glm::vec3(pitch, yaw, roll));
}

void Transform::rotateAround(const glm::vec3& axis, float angleDegrees) {
    float radians = glm::radians(angleDegrees);
    glm::quat rotation = glm::angleAxis(radians, glm::normalize(axis));
    m_localRotation = rotation * m_localRotation;
    markDirty();
}

void Transform::scale(float factor) {
    m_localScale *= factor;
    markDirty();
}

void Transform::scale(const glm::vec3& factors) {
    m_localScale *= factors;
    markDirty();
}

// ============================================================================
// Matrix Access
// ============================================================================

const glm::mat4& Transform::getLocalMatrix() const {
    if (m_dirty) {
        updateLocalMatrix();
        m_dirty = false;
    }
    return m_localMatrix;
}

const glm::mat4& Transform::getWorldMatrix() const {
    if (m_worldDirty || m_dirty) {
        updateWorldMatrix();
        m_worldDirty = false;
    }
    return m_worldMatrix;
}

glm::mat3 Transform::getNormalMatrix() const {
    return glm::transpose(glm::inverse(glm::mat3(getWorldMatrix())));
}

// ============================================================================
// World Space Getters
// ============================================================================

glm::vec3 Transform::getWorldPosition() const {
    return glm::vec3(getWorldMatrix()[3]);
}

glm::quat Transform::getWorldRotation() const {
    const glm::mat4& world = getWorldMatrix();
    glm::vec3 scale;
    glm::quat rotation;
    glm::vec3 translation;
    glm::vec3 skew;
    glm::vec4 perspective;
    glm::decompose(world, scale, rotation, translation, skew, perspective);
    return rotation;
}

glm::vec3 Transform::getWorldScale() const {
    const glm::mat4& world = getWorldMatrix();
    return glm::vec3(
        glm::length(glm::vec3(world[0])),
        glm::length(glm::vec3(world[1])),
        glm::length(glm::vec3(world[2]))
    );
}

// ============================================================================
// Direction Vectors
// ============================================================================

glm::vec3 Transform::getForward() const {
    return glm::normalize(getWorldRotation() * glm::vec3(0.0f, 0.0f, -1.0f));
}

glm::vec3 Transform::getRight() const {
    return glm::normalize(getWorldRotation() * glm::vec3(1.0f, 0.0f, 0.0f));
}

glm::vec3 Transform::getUp() const {
    return glm::normalize(getWorldRotation() * glm::vec3(0.0f, 1.0f, 0.0f));
}

// ============================================================================
// Dirty Management
// ============================================================================

void Transform::markDirty() {
    m_dirty = true;
    m_worldDirty = true;

    // Propagate dirty flag to children
    if (m_owner) {
        m_owner->markChildrenWorldDirty();
    }
}

// ============================================================================
// Private Matrix Updates
// ============================================================================

void Transform::updateLocalMatrix() const {
    // Build matrix: Translation * Rotation * Scale
    glm::mat4 translation = glm::translate(glm::mat4(1.0f), m_localPosition);
    glm::mat4 rotation = glm::mat4_cast(m_localRotation);
    glm::mat4 scale = glm::scale(glm::mat4(1.0f), m_localScale);

    m_localMatrix = translation * rotation * scale;
}

void Transform::updateWorldMatrix() const {
    // Ensure local matrix is up to date
    if (m_dirty) {
        updateLocalMatrix();
        m_dirty = false;
    }

    // Combine with parent world matrix if we have a parent
    if (m_owner && m_owner->getParent()) {
        const Transform& parentTransform = m_owner->getParent()->getTransform();
        m_worldMatrix = parentTransform.getWorldMatrix() * m_localMatrix;
    } else {
        // No parent, world = local
        m_worldMatrix = m_localMatrix;
    }
}

} // namespace Pina
