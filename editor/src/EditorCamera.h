#pragma once

#include <glm/glm.hpp>
#include <memory>

namespace Pina {
    class Camera;
    class Input;
}

namespace PinaEditor {

/// Editor camera with orbit and fly modes
class EditorCamera {
public:
    enum class Mode { Orbit, Fly };

    EditorCamera();
    ~EditorCamera();

    // Update camera based on input
    void update(Pina::Input* input, float deltaTime);

    // Focus on a point with given bounding size
    void focusOn(const glm::vec3& center, float size);

    // Reset to default position
    void reset();

    // Get the underlying camera
    Pina::Camera* getCamera() { return m_camera.get(); }

    // Camera mode
    Mode getMode() const { return m_mode; }
    void setMode(Mode mode) { m_mode = mode; }

    // Orbit settings
    void setOrbitTarget(const glm::vec3& target) { m_orbitTarget = target; }
    const glm::vec3& getOrbitTarget() const { return m_orbitTarget; }

    // Speed settings
    void setMoveSpeed(float speed) { m_moveSpeed = speed; }
    void setRotateSpeed(float speed) { m_rotateSpeed = speed; }
    void setZoomSpeed(float speed) { m_zoomSpeed = speed; }

private:
    void updateOrbitMode(Pina::Input* input, float deltaTime);
    void updateFlyMode(Pina::Input* input, float deltaTime);
    void updateCameraFromOrbit();

    std::unique_ptr<Pina::Camera> m_camera;

    Mode m_mode = Mode::Orbit;

    // Orbit mode parameters
    glm::vec3 m_orbitTarget = glm::vec3(0.0f);
    float m_orbitDistance = 5.0f;
    float m_yaw = -90.0f;    // Horizontal angle (degrees)
    float m_pitch = 20.0f;   // Vertical angle (degrees)

    // Speed settings
    float m_moveSpeed = 5.0f;
    float m_rotateSpeed = 0.3f;
    float m_zoomSpeed = 1.0f;

    // Input state tracking
    bool m_isPanning = false;
    bool m_isOrbiting = false;
    glm::vec2 m_lastMousePos = glm::vec2(0.0f);
};

} // namespace PinaEditor
