#include "EditorCamera.h"
#include <Pina.h>

namespace PinaEditor {

EditorCamera::EditorCamera() {
    m_camera = std::make_unique<Pina::Camera>();
    m_camera->setPerspective(45.0f, 16.0f / 9.0f, 0.1f, 1000.0f);

    // Create controllers that share the same camera
    m_orbitController = std::make_unique<Pina::OrbitCamera>(m_camera.get());
    m_freelookController = std::make_unique<Pina::FreelookCamera>(m_camera.get());

    // Set default orbit state
    m_orbitController->setRotation(-90.0f, 20.0f);
    m_orbitController->setDistance(5.0f);
}

EditorCamera::~EditorCamera() = default;

void EditorCamera::update(Pina::Input* input, float deltaTime) {
    if (!input) return;

    switch (m_mode) {
        case Mode::Orbit:
            m_orbitController->update(input, deltaTime);
            break;
        case Mode::Fly:
            m_freelookController->update(input, deltaTime);
            break;
    }
}

void EditorCamera::setMode(Mode mode) {
    if (mode == m_mode) return;

    // Transfer state when switching modes
    if (mode == Mode::Orbit) {
        transferStateToOrbit();
    } else {
        transferStateToFreelook();
    }

    m_mode = mode;
}

void EditorCamera::transferStateToOrbit() {
    // When switching to orbit, preserve camera position and look toward origin
    // Set yaw/pitch based on current camera orientation
    glm::vec3 pos = m_camera->getPosition();
    glm::vec3 target = glm::vec3(0.0f);

    // Calculate distance and angles from position
    glm::vec3 toCamera = pos - target;
    float distance = glm::length(toCamera);

    if (distance > 0.01f) {
        toCamera = glm::normalize(toCamera);
        float pitch = glm::degrees(asin(toCamera.y));
        float yaw = glm::degrees(atan2(toCamera.z, toCamera.x));

        m_orbitController->setTarget(target);
        m_orbitController->setDistance(distance);
        m_orbitController->setRotation(yaw, pitch);
    }
}

void EditorCamera::transferStateToFreelook() {
    // When switching to freelook, preserve current view direction
    float yaw = m_orbitController->getYaw();
    float pitch = m_orbitController->getPitch();
    m_freelookController->setRotation(yaw, pitch);
}

void EditorCamera::focusOn(const glm::vec3& center, float size) {
    m_orbitController->focusOn(center, size);
}

void EditorCamera::reset() {
    m_orbitController->reset();
    m_freelookController->reset();
}

void EditorCamera::setOrbitTarget(const glm::vec3& target) {
    m_orbitController->setTarget(target);
}

glm::vec3 EditorCamera::getOrbitTarget() const {
    return m_orbitController->getTarget();
}

void EditorCamera::setMoveSpeed(float speed) {
    m_freelookController->setMoveSpeed(speed);
}

void EditorCamera::setRotateSpeed(float speed) {
    m_orbitController->setRotateSpeed(speed);
    m_freelookController->setRotateSpeed(speed);
}

void EditorCamera::setZoomSpeed(float speed) {
    m_orbitController->setZoomSpeed(speed);
}

} // namespace PinaEditor
