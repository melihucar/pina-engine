#pragma once

#include "Gizmo.h"
#include <glm/gtc/quaternion.hpp>

namespace PinaEditor {

/// Rotation gizmo with X/Y/Z circles
class RotateGizmo : public Gizmo {
public:
    void render(GizmoRenderer* renderer, const glm::mat4& transform, Pina::Camera* camera) override;
    bool handleInput(Pina::Input* input, Pina::Camera* camera, Pina::Transform* target) override;

private:
    // Check if mouse is over an axis circle
    int checkAxisHover(Pina::Input* input, Pina::Camera* camera, const glm::vec3& center);

    glm::quat m_dragStartRotation;
    glm::vec2 m_dragStartMouse;
    float m_dragStartAngle = 0.0f;
};

} // namespace PinaEditor
