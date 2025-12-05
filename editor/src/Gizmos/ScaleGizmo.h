#pragma once

#include "Gizmo.h"

namespace PinaEditor {

/// Scale gizmo with X/Y/Z handles (cubes at end of lines)
class ScaleGizmo : public Gizmo {
public:
    void render(GizmoRenderer* renderer, const glm::mat4& transform, Pina::Camera* camera) override;
    bool handleInput(Pina::Input* input, Pina::Camera* camera, Pina::Transform* target) override;

private:
    int checkAxisHover(Pina::Input* input, Pina::Camera* camera, const glm::vec3& center);

    glm::vec3 m_dragStartScale;
    glm::vec2 m_dragStartMouse;
};

} // namespace PinaEditor
