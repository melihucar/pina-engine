#include "EditorCamera.h"
#include <Pina.h>
#include <algorithm>

namespace PinaEditor {

EditorCamera::EditorCamera() {
    m_camera = std::make_unique<Pina::Camera>();
    m_camera->setPerspective(45.0f, 16.0f / 9.0f, 0.1f, 1000.0f);
    updateCameraFromOrbit();
}

EditorCamera::~EditorCamera() = default;

void EditorCamera::update(Pina::Input* input, float deltaTime) {
    if (!input) return;

    switch (m_mode) {
        case Mode::Orbit:
            updateOrbitMode(input, deltaTime);
            break;
        case Mode::Fly:
            updateFlyMode(input, deltaTime);
            break;
    }
}

void EditorCamera::updateOrbitMode(Pina::Input* input, float deltaTime) {
    (void)deltaTime;

    glm::vec2 mousePos = input->getMousePosition();
    glm::vec2 mouseDelta = mousePos - m_lastMousePos;

    // Middle mouse button: pan
    if (input->isMouseButtonDown(Pina::MouseButton::Middle)) {
        if (!m_isPanning && !m_isOrbiting) {
            m_isPanning = true;
        }

        if (m_isPanning) {
            // Pan in camera space - calculate right/up from camera orientation
            glm::vec3 forward = glm::normalize(m_orbitTarget - m_camera->getPosition());
            glm::vec3 right = glm::normalize(glm::cross(forward, glm::vec3(0.0f, 1.0f, 0.0f)));
            glm::vec3 up = glm::normalize(glm::cross(right, forward));
            float panSpeed = m_orbitDistance * 0.002f;
            m_orbitTarget -= right * mouseDelta.x * panSpeed;
            m_orbitTarget += up * mouseDelta.y * panSpeed;
            updateCameraFromOrbit();
        }
    } else {
        m_isPanning = false;
    }

    // Right mouse button: orbit
    if (input->isMouseButtonDown(Pina::MouseButton::Right)) {
        if (!m_isOrbiting && !m_isPanning) {
            m_isOrbiting = true;
        }

        if (m_isOrbiting) {
            m_yaw += mouseDelta.x * m_rotateSpeed;
            m_pitch += mouseDelta.y * m_rotateSpeed;  // Drag up = look up
            m_pitch = std::clamp(m_pitch, -89.0f, 89.0f);
            updateCameraFromOrbit();
        }
    } else {
        m_isOrbiting = false;
    }

    // Mouse wheel: zoom
    float scroll = input->getScrollDelta().y;
    if (scroll != 0.0f) {
        m_orbitDistance -= scroll * m_zoomSpeed;
        m_orbitDistance = std::max(m_orbitDistance, 0.1f);
        updateCameraFromOrbit();
    }

    m_lastMousePos = mousePos;
}

void EditorCamera::updateFlyMode(Pina::Input* input, float deltaTime) {
    glm::vec2 mousePos = input->getMousePosition();
    glm::vec2 mouseDelta = mousePos - m_lastMousePos;

    // Right mouse: look around
    if (input->isMouseButtonDown(Pina::MouseButton::Right)) {
        m_yaw += mouseDelta.x * m_rotateSpeed;
        m_pitch += mouseDelta.y * m_rotateSpeed;  // Drag up = look up
        m_pitch = std::clamp(m_pitch, -89.0f, 89.0f);

        // Calculate direction from yaw/pitch
        glm::vec3 front;
        front.x = cos(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
        front.y = sin(glm::radians(m_pitch));
        front.z = sin(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
        front = glm::normalize(front);

        glm::vec3 right = glm::normalize(glm::cross(front, glm::vec3(0.0f, 1.0f, 0.0f)));
        glm::vec3 up = glm::normalize(glm::cross(right, front));

        // WASD movement
        glm::vec3 position = m_camera->getPosition();
        float speed = m_moveSpeed * deltaTime;

        if (input->isKeyDown(Pina::Key::W)) position += front * speed;
        if (input->isKeyDown(Pina::Key::S)) position -= front * speed;
        if (input->isKeyDown(Pina::Key::A)) position -= right * speed;
        if (input->isKeyDown(Pina::Key::D)) position += right * speed;
        if (input->isKeyDown(Pina::Key::Q)) position -= up * speed;
        if (input->isKeyDown(Pina::Key::E)) position += up * speed;

        m_camera->setPosition(position);
        m_camera->lookAt(position, position + front, glm::vec3(0.0f, 1.0f, 0.0f));
    }

    m_lastMousePos = mousePos;
}

void EditorCamera::updateCameraFromOrbit() {
    // Calculate camera position from spherical coordinates
    float yawRad = glm::radians(m_yaw);
    float pitchRad = glm::radians(m_pitch);

    glm::vec3 position;
    position.x = m_orbitTarget.x + m_orbitDistance * cos(pitchRad) * cos(yawRad);
    position.y = m_orbitTarget.y + m_orbitDistance * sin(pitchRad);
    position.z = m_orbitTarget.z + m_orbitDistance * cos(pitchRad) * sin(yawRad);

    m_camera->setPosition(position);
    m_camera->lookAt(position, m_orbitTarget, glm::vec3(0.0f, 1.0f, 0.0f));
}

void EditorCamera::focusOn(const glm::vec3& center, float size) {
    m_orbitTarget = center;
    m_orbitDistance = size * 2.0f;
    updateCameraFromOrbit();
}

void EditorCamera::reset() {
    m_orbitTarget = glm::vec3(0.0f);
    m_orbitDistance = 5.0f;
    m_yaw = -90.0f;
    m_pitch = 20.0f;
    updateCameraFromOrbit();
}

} // namespace PinaEditor
