#pragma once

/// Pina Engine - Camera System
/// Provides view and projection matrices for 3D rendering

#include "../Core/Export.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace Pina {

// Forward declaration
class Input;

/// Camera class for 3D rendering
/// Manages view and projection matrices
class PINA_API Camera {
public:
    Camera();
    virtual ~Camera() = default;

    // ========================================================================
    // Projection
    // ========================================================================

    /// Set perspective projection
    /// @param fov Field of view in degrees
    /// @param aspectRatio Width / Height
    /// @param nearPlane Near clipping plane
    /// @param farPlane Far clipping plane
    void setPerspective(float fov, float aspectRatio, float nearPlane, float farPlane);

    /// Set orthographic projection
    /// @param left Left boundary
    /// @param right Right boundary
    /// @param bottom Bottom boundary
    /// @param top Top boundary
    /// @param nearPlane Near clipping plane
    /// @param farPlane Far clipping plane
    void setOrthographic(float left, float right, float bottom, float top,
                         float nearPlane, float farPlane);

    /// Update aspect ratio (useful on window resize)
    void setAspectRatio(float aspectRatio);

    // ========================================================================
    // View / Transform
    // ========================================================================

    /// Set camera position
    void setPosition(const glm::vec3& position);

    /// Set camera target (look at point)
    void setTarget(const glm::vec3& target);

    /// Set up vector
    void setUp(const glm::vec3& up);

    /// Look at a specific point
    void lookAt(const glm::vec3& position, const glm::vec3& target, const glm::vec3& up);

    // ========================================================================
    // Getters
    // ========================================================================

    /// Get the view matrix
    const glm::mat4& getViewMatrix() const { return m_viewMatrix; }

    /// Get the projection matrix
    const glm::mat4& getProjectionMatrix() const { return m_projectionMatrix; }

    /// Get the combined view-projection matrix
    glm::mat4 getViewProjectionMatrix() const { return m_projectionMatrix * m_viewMatrix; }

    /// Get camera position
    const glm::vec3& getPosition() const { return m_position; }

    /// Get camera target
    const glm::vec3& getTarget() const { return m_target; }

    // ========================================================================
    // Input Handling (for controllable cameras)
    // ========================================================================

    /// Handle input for camera movement (override in derived classes)
    /// @param input Input system
    /// @param deltaTime Time since last frame
    virtual void handleInput(Input* input, float deltaTime) {
        (void)input; (void)deltaTime;
    }

    /// Whether this camera wants to receive input
    /// Override to return true for controllable cameras
    virtual bool wantsInput() const { return false; }

    // ========================================================================
    // Focus Helpers
    // ========================================================================

    /// Focus on a point at a given distance
    /// @param center Point to focus on
    /// @param distance Distance from the point
    void focusOn(const glm::vec3& center, float distance);

private:
    void updateViewMatrix();
    void updateProjectionMatrix();

    // View parameters
    glm::vec3 m_position = glm::vec3(0.0f, 0.0f, 3.0f);
    glm::vec3 m_target = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 m_up = glm::vec3(0.0f, 1.0f, 0.0f);

    // Projection parameters
    float m_fov = 45.0f;
    float m_aspectRatio = 16.0f / 9.0f;
    float m_nearPlane = 0.1f;
    float m_farPlane = 100.0f;
    bool m_isPerspective = true;

    // Orthographic parameters
    float m_orthoLeft = -1.0f;
    float m_orthoRight = 1.0f;
    float m_orthoBottom = -1.0f;
    float m_orthoTop = 1.0f;

    // Cached matrices
    glm::mat4 m_viewMatrix = glm::mat4(1.0f);
    glm::mat4 m_projectionMatrix = glm::mat4(1.0f);
};

} // namespace Pina
