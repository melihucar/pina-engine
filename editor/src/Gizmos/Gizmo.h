#pragma once

#include <glm/glm.hpp>

namespace Pina {
    class Camera;
    class Input;
    class Transform;
}

namespace PinaEditor {

class GizmoRenderer;

enum class GizmoMode {
    Translate,
    Rotate,
    Scale
};

enum class GizmoSpace {
    Local,
    World
};

/// Base class for transform gizmos
class Gizmo {
public:
    virtual ~Gizmo() = default;

    // Render the gizmo at the given transform position
    virtual void render(GizmoRenderer* renderer, const glm::mat4& transform, Pina::Camera* camera) = 0;

    // Handle input and return true if transform was modified
    virtual bool handleInput(Pina::Input* input, Pina::Camera* camera, Pina::Transform* target) = 0;

    // Check if gizmo is being dragged
    bool isDragging() const { return m_isDragging; }

    // Active axis during drag (-1 = none, 0 = X, 1 = Y, 2 = Z)
    int getActiveAxis() const { return m_activeAxis; }

    // Gizmo space (local or world)
    GizmoSpace getSpace() const { return m_space; }
    void setSpace(GizmoSpace space) { m_space = space; }

    // Size multiplier for the gizmo
    float getSize() const { return m_size; }
    void setSize(float size) { m_size = size; }

    // Set viewport bounds for coordinate conversion (call before handleInput)
    void setViewportBounds(const glm::vec2& position, const glm::vec2& size) {
        m_viewportPosition = position;
        m_viewportSize = size;
    }

protected:
    // Helper to get screen-space ray from mouse position
    glm::vec3 getMouseRay(Pina::Input* input, Pina::Camera* camera) const;

    // Helper to project world point to screen
    glm::vec2 worldToScreen(const glm::vec3& worldPos, Pina::Camera* camera) const;

    // Helper to calculate gizmo scale based on camera distance
    float calculateGizmoScale(const glm::vec3& position, Pina::Camera* camera) const;

    bool m_isDragging = false;
    bool m_isHovered = false;
    int m_activeAxis = -1;  // -1 = none, 0 = X, 1 = Y, 2 = Z
    GizmoSpace m_space = GizmoSpace::World;
    float m_size = 1.0f;

    // Viewport bounds for coordinate conversion
    glm::vec2 m_viewportPosition = glm::vec2(0.0f);
    glm::vec2 m_viewportSize = glm::vec2(1600.0f, 900.0f);

    // Axis colors
    static constexpr glm::vec4 X_AXIS_COLOR = glm::vec4(1.0f, 0.2f, 0.2f, 1.0f);
    static constexpr glm::vec4 Y_AXIS_COLOR = glm::vec4(0.2f, 1.0f, 0.2f, 1.0f);
    static constexpr glm::vec4 Z_AXIS_COLOR = glm::vec4(0.2f, 0.4f, 1.0f, 1.0f);
    static constexpr glm::vec4 HOVER_COLOR = glm::vec4(1.0f, 1.0f, 0.0f, 1.0f);
};

} // namespace PinaEditor
