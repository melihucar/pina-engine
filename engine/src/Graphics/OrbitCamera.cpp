#include "OrbitCamera.h"
#include "Camera.h"
#include "../Input/Input.h"
#include <glm/gtc/matrix_transform.hpp>
#include <cmath>

namespace Pina {

OrbitCamera::OrbitCamera(Camera* camera)
    : m_camera(camera)
{
    updateCameraFromOrbit();
}

void OrbitCamera::update(Input* input, float deltaTime) {
    if (!input || !m_camera) return;
    (void)deltaTime;

    glm::vec2 mousePos = input->getMousePosition();
    glm::vec2 mouseDelta = mousePos - m_lastMousePos;

    // Check rotate button
    bool rotateButtonDown = false;
    switch (m_rotateButton) {
        case MouseButton::Left:   rotateButtonDown = input->isMouseButtonDown(Pina::MouseButton::Left); break;
        case MouseButton::Middle: rotateButtonDown = input->isMouseButtonDown(Pina::MouseButton::Middle); break;
        case MouseButton::Right:  rotateButtonDown = input->isMouseButtonDown(Pina::MouseButton::Right); break;
    }

    // Check pan button
    bool panButtonDown = false;
    switch (m_panButton) {
        case MouseButton::Left:   panButtonDown = input->isMouseButtonDown(Pina::MouseButton::Left); break;
        case MouseButton::Middle: panButtonDown = input->isMouseButtonDown(Pina::MouseButton::Middle); break;
        case MouseButton::Right:  panButtonDown = input->isMouseButtonDown(Pina::MouseButton::Right); break;
    }

    // Pan takes priority if both buttons are the same
    if (panButtonDown && !m_isRotating) {
        if (!m_isPanning) {
            m_isPanning = true;
        }

        if (m_isPanning) {
            pan(mouseDelta);
        }
    } else {
        m_isPanning = false;
    }

    // Rotation
    if (rotateButtonDown && !m_isPanning) {
        if (!m_isRotating) {
            m_isRotating = true;
        }

        if (m_isRotating) {
            rotate(mouseDelta.x * m_rotateSpeed, mouseDelta.y * m_rotateSpeed);
        }
    } else {
        m_isRotating = false;
    }

    // Mouse wheel zoom
    float scroll = input->getScrollDelta().y;
    if (scroll != 0.0f) {
        zoom(scroll * m_zoomSpeed);
    }

    m_lastMousePos = mousePos;
}

void OrbitCamera::setTarget(const glm::vec3& target) {
    m_target = target;
    updateCameraFromOrbit();
}

void OrbitCamera::focusOn(const glm::vec3& center, float size) {
    m_target = center;
    m_distance = size * 2.0f;
    updateCameraFromOrbit();
}

void OrbitCamera::setDistance(float distance) {
    m_distance = std::clamp(distance, m_minDistance, m_maxDistance);
    updateCameraFromOrbit();
}

void OrbitCamera::zoom(float delta) {
    m_distance -= delta;
    m_distance = std::clamp(m_distance, m_minDistance, m_maxDistance);
    updateCameraFromOrbit();
}

void OrbitCamera::rotate(float yawDelta, float pitchDelta) {
    m_yaw += yawDelta;
    m_pitch += pitchDelta;  // Drag up = look up
    m_pitch = std::clamp(m_pitch, m_minPitch, m_maxPitch);
    updateCameraFromOrbit();
}

void OrbitCamera::setRotation(float yaw, float pitch) {
    m_yaw = yaw;
    m_pitch = std::clamp(pitch, m_minPitch, m_maxPitch);
    updateCameraFromOrbit();
}

void OrbitCamera::pan(const glm::vec2& delta) {
    if (!m_camera) return;

    // Calculate right and up vectors from camera orientation
    glm::vec3 forward = glm::normalize(m_target - m_camera->getPosition());
    glm::vec3 right = glm::normalize(glm::cross(forward, glm::vec3(0.0f, 1.0f, 0.0f)));
    glm::vec3 up = glm::normalize(glm::cross(right, forward));

    // Pan speed scales with distance
    float panSpeed = m_distance * m_panSpeed;

    // Move target in camera's local XY plane
    m_target -= right * delta.x * panSpeed;
    m_target += up * delta.y * panSpeed;

    updateCameraFromOrbit();
}

void OrbitCamera::reset() {
    m_target = glm::vec3(0.0f);
    m_distance = 5.0f;
    m_yaw = -90.0f;
    m_pitch = 20.0f;
    updateCameraFromOrbit();
}

void OrbitCamera::updateCameraFromOrbit() {
    if (!m_camera) return;

    // Convert spherical coordinates to Cartesian
    float yawRad = glm::radians(m_yaw);
    float pitchRad = glm::radians(m_pitch);

    glm::vec3 position;
    position.x = m_target.x + m_distance * cos(pitchRad) * cos(yawRad);
    position.y = m_target.y + m_distance * sin(pitchRad);
    position.z = m_target.z + m_distance * cos(pitchRad) * sin(yawRad);

    m_camera->setPosition(position);
    m_camera->lookAt(position, m_target, glm::vec3(0.0f, 1.0f, 0.0f));
}

} // namespace Pina
