/// Pina Engine - Camera Implementation

#include "Camera.h"

namespace Pina {

Camera::Camera() {
    updateViewMatrix();
    updateProjectionMatrix();
}

void Camera::setPerspective(float fov, float aspectRatio, float nearPlane, float farPlane) {
    m_isPerspective = true;
    m_fov = fov;
    m_aspectRatio = aspectRatio;
    m_nearPlane = nearPlane;
    m_farPlane = farPlane;
    updateProjectionMatrix();
}

void Camera::setOrthographic(float left, float right, float bottom, float top,
                              float nearPlane, float farPlane) {
    m_isPerspective = false;
    m_orthoLeft = left;
    m_orthoRight = right;
    m_orthoBottom = bottom;
    m_orthoTop = top;
    m_nearPlane = nearPlane;
    m_farPlane = farPlane;
    updateProjectionMatrix();
}

void Camera::setAspectRatio(float aspectRatio) {
    m_aspectRatio = aspectRatio;
    if (m_isPerspective) {
        updateProjectionMatrix();
    }
}

void Camera::setPosition(const glm::vec3& position) {
    m_position = position;
    updateViewMatrix();
}

void Camera::setTarget(const glm::vec3& target) {
    m_target = target;
    updateViewMatrix();
}

void Camera::setUp(const glm::vec3& up) {
    m_up = up;
    updateViewMatrix();
}

void Camera::lookAt(const glm::vec3& position, const glm::vec3& target, const glm::vec3& up) {
    m_position = position;
    m_target = target;
    m_up = up;
    updateViewMatrix();
}

void Camera::updateViewMatrix() {
    m_viewMatrix = glm::lookAt(m_position, m_target, m_up);
}

void Camera::updateProjectionMatrix() {
    if (m_isPerspective) {
        m_projectionMatrix = glm::perspective(glm::radians(m_fov), m_aspectRatio,
                                               m_nearPlane, m_farPlane);
    } else {
        m_projectionMatrix = glm::ortho(m_orthoLeft, m_orthoRight,
                                         m_orthoBottom, m_orthoTop,
                                         m_nearPlane, m_farPlane);
    }
}

} // namespace Pina
