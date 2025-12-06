#include "FreelookCamera.h"
#include "Camera.h"
#include "../Input/Input.h"
#include <glm/gtc/matrix_transform.hpp>
#include <cmath>

namespace Pina {

FreelookCamera::FreelookCamera(Camera* camera)
    : m_camera(camera)
{
    updateCameraFromRotation();
}

void FreelookCamera::update(Input* input, float deltaTime) {
    if (!input || !m_camera) return;

    glm::vec2 mousePos = input->getMousePosition();
    glm::vec2 mouseDelta = mousePos - m_lastMousePos;

    // Check look button
    bool lookButtonDown = false;
    switch (m_lookButton) {
        case MouseButton::Left:   lookButtonDown = input->isMouseButtonDown(Pina::MouseButton::Left); break;
        case MouseButton::Middle: lookButtonDown = input->isMouseButtonDown(Pina::MouseButton::Middle); break;
        case MouseButton::Right:  lookButtonDown = input->isMouseButtonDown(Pina::MouseButton::Right); break;
    }

    // Look around when button is held
    if (lookButtonDown) {
        if (!m_isLooking) {
            m_isLooking = true;
        }

        if (m_isLooking) {
            rotate(mouseDelta.x * m_rotateSpeed, mouseDelta.y * m_rotateSpeed);
        }

        // Movement only when looking
        glm::vec3 front = calculateFrontVector();
        glm::vec3 right = glm::normalize(glm::cross(front, glm::vec3(0.0f, 1.0f, 0.0f)));
        glm::vec3 up = glm::normalize(glm::cross(right, front));

        glm::vec3 position = m_camera->getPosition();

        // Speed with optional boost
        float speed = m_moveSpeed * deltaTime;
        if (input->isKeyDown(m_keys.boost)) {
            speed *= m_boostMultiplier;
        }

        // WASD movement
        if (input->isKeyDown(m_keys.forward))  position += front * speed;
        if (input->isKeyDown(m_keys.backward)) position -= front * speed;
        if (input->isKeyDown(m_keys.left))     position -= right * speed;
        if (input->isKeyDown(m_keys.right))    position += right * speed;
        if (input->isKeyDown(m_keys.up))       position += up * speed;
        if (input->isKeyDown(m_keys.down))     position -= up * speed;

        m_camera->setPosition(position);
        m_camera->lookAt(position, position + front, glm::vec3(0.0f, 1.0f, 0.0f));
    } else {
        m_isLooking = false;
    }

    m_lastMousePos = mousePos;
}

void FreelookCamera::rotate(float yawDelta, float pitchDelta) {
    m_yaw += yawDelta;
    m_pitch += pitchDelta;  // Drag up = look up
    m_pitch = std::clamp(m_pitch, m_minPitch, m_maxPitch);
    updateCameraFromRotation();
}

void FreelookCamera::setRotation(float yaw, float pitch) {
    m_yaw = yaw;
    m_pitch = std::clamp(pitch, m_minPitch, m_maxPitch);
    updateCameraFromRotation();
}

void FreelookCamera::reset(const glm::vec3& position) {
    if (m_camera) {
        m_camera->setPosition(position);
    }
    m_yaw = -90.0f;
    m_pitch = 0.0f;
    updateCameraFromRotation();
}

glm::vec3 FreelookCamera::calculateFrontVector() const {
    float yawRad = glm::radians(m_yaw);
    float pitchRad = glm::radians(m_pitch);

    glm::vec3 front;
    front.x = cos(yawRad) * cos(pitchRad);
    front.y = sin(pitchRad);
    front.z = sin(yawRad) * cos(pitchRad);
    return glm::normalize(front);
}

void FreelookCamera::updateCameraFromRotation() {
    if (!m_camera) return;

    glm::vec3 front = calculateFrontVector();
    glm::vec3 position = m_camera->getPosition();
    m_camera->lookAt(position, position + front, glm::vec3(0.0f, 1.0f, 0.0f));
}

} // namespace Pina
