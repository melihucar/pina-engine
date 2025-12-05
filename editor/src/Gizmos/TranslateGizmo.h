#pragma once

#include "Gizmo.h"

namespace PinaEditor {

/// Translation gizmo with X/Y/Z arrows
class TranslateGizmo : public Gizmo {
public:
    void render(GizmoRenderer* renderer, const glm::mat4& transform, Pina::Camera* camera) override;
    bool handleInput(Pina::Input* input, Pina::Camera* camera, Pina::Transform* target) override;

private:
    // Check if mouse is over an axis arrow
    int checkAxisHover(Pina::Input* input, Pina::Camera* camera, const glm::vec3& center);

    // Project mouse movement onto axis for dragging
    glm::vec3 projectOntoAxis(const glm::vec3& axis, const glm::vec3& rayOrigin,
                              const glm::vec3& rayDir, const glm::vec3& center);

    glm::vec3 m_dragStartPos;
    glm::vec3 m_dragStartObjectPos;
    glm::vec3 m_dragStartCenter;  // Original center at drag start
    glm::vec3 m_dragAxis;         // Axis direction at drag start
};

} // namespace PinaEditor
