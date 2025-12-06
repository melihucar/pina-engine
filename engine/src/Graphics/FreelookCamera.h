#pragma once

/// Pina Engine - Freelook Camera Controller
/// Provides FPS-style camera with WASD movement and mouse look

#include "../Core/Export.h"
#include "../Input/KeyCodes.h"
#include <glm/glm.hpp>
#include <algorithm>

namespace Pina {

class Camera;
class Input;

/// Freelook camera controller with FPS-style movement
/// Supports WASD movement and mouse look
class PINA_API FreelookCamera {
public:
    /// Create a freelook camera controller
    /// @param camera Pointer to the camera to control (non-owning)
    explicit FreelookCamera(Camera* camera);
    ~FreelookCamera() = default;

    // ========================================================================
    // Update
    // ========================================================================

    /// Update camera based on input
    /// @param input Pointer to input system
    /// @param deltaTime Time since last frame
    void update(Input* input, float deltaTime);

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
    // Speed Settings
    // ========================================================================

    void setMoveSpeed(float speed) { m_moveSpeed = speed; }
    void setRotateSpeed(float speed) { m_rotateSpeed = speed; }
    void setBoostMultiplier(float multiplier) { m_boostMultiplier = multiplier; }

    float getMoveSpeed() const { return m_moveSpeed; }
    float getRotateSpeed() const { return m_rotateSpeed; }
    float getBoostMultiplier() const { return m_boostMultiplier; }

    // ========================================================================
    // Reset
    // ========================================================================

    /// Reset to a specific position
    void reset(const glm::vec3& position = glm::vec3(0.0f, 2.0f, 5.0f));

    // ========================================================================
    // Mouse Button Configuration
    // ========================================================================

    enum class MouseButton { Left = 0, Middle = 1, Right = 2 };

    void setLookButton(MouseButton button) { m_lookButton = button; }
    MouseButton getLookButton() const { return m_lookButton; }

    // ========================================================================
    // Movement Keys Configuration
    // ========================================================================

    struct MovementKeys {
        Key forward = Key::W;
        Key backward = Key::S;
        Key left = Key::A;
        Key right = Key::D;
        Key up = Key::E;
        Key down = Key::Q;
        Key boost = Key::LeftShift;
    };

    void setMovementKeys(const MovementKeys& keys) { m_keys = keys; }
    const MovementKeys& getMovementKeys() const { return m_keys; }

    // ========================================================================
    // Camera Access
    // ========================================================================

    /// Get the underlying camera
    Camera* getCamera() const { return m_camera; }

private:
    void updateCameraFromRotation();
    glm::vec3 calculateFrontVector() const;

    Camera* m_camera;  // Non-owning pointer

    float m_yaw = -90.0f;    // Horizontal angle (degrees)
    float m_pitch = 0.0f;    // Vertical angle (degrees)

    float m_moveSpeed = 5.0f;
    float m_rotateSpeed = 0.3f;
    float m_boostMultiplier = 2.0f;

    float m_minPitch = -89.0f;
    float m_maxPitch = 89.0f;

    MouseButton m_lookButton = MouseButton::Right;
    MovementKeys m_keys;

    bool m_isLooking = false;
    glm::vec2 m_lastMousePos = glm::vec2(0.0f);
};

} // namespace Pina
