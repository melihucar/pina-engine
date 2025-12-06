#pragma once

/// Pina Engine - Orbit Camera Controller
/// Provides orbit camera functionality that rotates around a target point

#include "../Core/Export.h"
#include <glm/glm.hpp>
#include <algorithm>

namespace Pina {

class Camera;
class Input;

/// Orbit camera controller that rotates around a target point
/// Supports pan, zoom, and rotation operations
class PINA_API OrbitCamera {
public:
    /// Create an orbit camera controller
    /// @param camera Pointer to the camera to control (non-owning)
    explicit OrbitCamera(Camera* camera);
    ~OrbitCamera() = default;

    // ========================================================================
    // Update
    // ========================================================================

    /// Update camera based on input
    /// @param input Pointer to input system
    /// @param deltaTime Time since last frame
    void update(Input* input, float deltaTime);

    // ========================================================================
    // Target Manipulation
    // ========================================================================

    /// Set the orbit target point
    void setTarget(const glm::vec3& target);

    /// Get the orbit target point
    const glm::vec3& getTarget() const { return m_target; }

    /// Focus on a point with given bounding size
    void focusOn(const glm::vec3& center, float size);

    // ========================================================================
    // Distance Control
    // ========================================================================

    /// Set distance from target
    void setDistance(float distance);

    /// Get distance from target
    float getDistance() const { return m_distance; }

    /// Zoom in/out (positive = zoom in, negative = zoom out)
    void zoom(float delta);

    // ========================================================================
    // Rotation Control
    // ========================================================================

    /// Rotate by delta angles (in degrees)
    void rotate(float yawDelta, float pitchDelta);

    /// Set rotation angles (in degrees)
    void setRotation(float yaw, float pitch);

    /// Get yaw angle (horizontal rotation in degrees)
    float getYaw() const { return m_yaw; }

    /// Get pitch angle (vertical rotation in degrees)
    float getPitch() const { return m_pitch; }

    // ========================================================================
    // Pan Control
    // ========================================================================

    /// Pan the camera (move target in screen space)
    void pan(const glm::vec2& delta);

    // ========================================================================
    // Speed Settings
    // ========================================================================

    void setRotateSpeed(float speed) { m_rotateSpeed = speed; }
    void setPanSpeed(float speed) { m_panSpeed = speed; }
    void setZoomSpeed(float speed) { m_zoomSpeed = speed; }

    float getRotateSpeed() const { return m_rotateSpeed; }
    float getPanSpeed() const { return m_panSpeed; }
    float getZoomSpeed() const { return m_zoomSpeed; }

    // ========================================================================
    // Reset
    // ========================================================================

    /// Reset to default position
    void reset();

    // ========================================================================
    // Mouse Button Configuration
    // ========================================================================

    enum class MouseButton { Left = 0, Middle = 1, Right = 2 };

    void setRotateButton(MouseButton button) { m_rotateButton = button; }
    void setPanButton(MouseButton button) { m_panButton = button; }

    MouseButton getRotateButton() const { return m_rotateButton; }
    MouseButton getPanButton() const { return m_panButton; }

    // ========================================================================
    // Camera Access
    // ========================================================================

    /// Get the underlying camera
    Camera* getCamera() const { return m_camera; }

private:
    void updateCameraFromOrbit();

    Camera* m_camera;  // Non-owning pointer

    glm::vec3 m_target = glm::vec3(0.0f);
    float m_distance = 5.0f;
    float m_yaw = -90.0f;    // Horizontal angle (degrees)
    float m_pitch = 20.0f;   // Vertical angle (degrees)

    float m_rotateSpeed = 0.3f;
    float m_panSpeed = 0.002f;
    float m_zoomSpeed = 1.0f;

    float m_minDistance = 0.1f;
    float m_maxDistance = 1000.0f;
    float m_minPitch = -89.0f;
    float m_maxPitch = 89.0f;

    MouseButton m_rotateButton = MouseButton::Right;
    MouseButton m_panButton = MouseButton::Middle;

    bool m_isRotating = false;
    bool m_isPanning = false;
    glm::vec2 m_lastMousePos = glm::vec2(0.0f);
};

} // namespace Pina
