#include "RotateGizmo.h"
#include "GizmoRenderer.h"
#include <Pina.h>

namespace PinaEditor {

void RotateGizmo::render(GizmoRenderer* renderer, const glm::mat4& transform, Pina::Camera* camera) {
    if (!renderer || !camera) return;

    glm::vec3 center = glm::vec3(transform[3]);
    float scale = calculateGizmoScale(center, camera);

    // Get axis directions based on space mode
    glm::vec3 xAxis, yAxis, zAxis;
    if (m_space == GizmoSpace::Local) {
        xAxis = glm::normalize(glm::vec3(transform[0]));
        yAxis = glm::normalize(glm::vec3(transform[1]));
        zAxis = glm::normalize(glm::vec3(transform[2]));
    } else {
        xAxis = glm::vec3(1, 0, 0);
        yAxis = glm::vec3(0, 1, 0);
        zAxis = glm::vec3(0, 0, 1);
    }

    // Draw circles with appropriate colors
    glm::vec4 xColor = (m_activeAxis == 0) ? HOVER_COLOR : X_AXIS_COLOR;
    glm::vec4 yColor = (m_activeAxis == 1) ? HOVER_COLOR : Y_AXIS_COLOR;
    glm::vec4 zColor = (m_activeAxis == 2) ? HOVER_COLOR : Z_AXIS_COLOR;

    renderer->drawCircle(center, xAxis, scale, xColor, 48);
    renderer->drawCircle(center, yAxis, scale, yColor, 48);
    renderer->drawCircle(center, zAxis, scale, zColor, 48);
}

bool RotateGizmo::handleInput(Pina::Input* input, Pina::Camera* camera, Pina::Transform* target) {
    if (!input || !camera || !target) return false;

    glm::vec3 center = target->getWorldPosition();

    bool mouseDown = input->isMouseButtonDown(Pina::MouseButton::Left);
    bool mousePressed = input->isMouseButtonPressed(Pina::MouseButton::Left);
    bool mouseReleased = input->isMouseButtonReleased(Pina::MouseButton::Left);

    if (!m_isDragging) {
        m_activeAxis = checkAxisHover(input, camera, center);
        m_isHovered = (m_activeAxis != -1);

        if (mousePressed && m_activeAxis != -1) {
            m_isDragging = true;
            m_dragStartRotation = target->getLocalRotation();
            m_dragStartMouse = input->getMousePosition();
        }
    }

    if (m_isDragging) {
        if (mouseReleased) {
            m_isDragging = false;
            m_activeAxis = -1;
            return true;
        }

        if (mouseDown) {
            glm::vec2 mousePos = input->getMousePosition();
            glm::vec2 delta = mousePos - m_dragStartMouse;

            // Calculate rotation based on mouse movement
            // TODO: This is a simplified rotation. Could improve with proper arc-ball rotation.
            float sensitivity = 0.5f;
            float angle = (delta.x + delta.y) * sensitivity;

            glm::vec3 axis;
            if (m_space == GizmoSpace::Local) {
                glm::mat4 worldMat = target->getWorldMatrix();
                axis = glm::normalize(glm::vec3(worldMat[m_activeAxis]));
            } else {
                axis = glm::vec3(m_activeAxis == 0, m_activeAxis == 1, m_activeAxis == 2);
            }

            glm::quat rotation = glm::angleAxis(glm::radians(angle), axis);
            target->setLocalRotation(rotation * m_dragStartRotation);

            return true;
        }
    }

    return false;
}

int RotateGizmo::checkAxisHover(Pina::Input* input, Pina::Camera* camera, const glm::vec3& center) {
    glm::vec2 windowMousePos = input->getMousePosition();
    // Convert window mouse position to viewport-relative coordinates
    glm::vec2 mousePos = windowMousePos - m_viewportPosition;
    float scale = calculateGizmoScale(center, camera);
    float threshold = 10.0f;

    // Check each circle - sample points along circle and find closest
    for (int axis = 0; axis < 3; ++axis) {
        glm::vec3 normal = glm::vec3(axis == 0, axis == 1, axis == 2);

        // Create orthonormal basis for circle
        glm::vec3 up = (glm::abs(normal.y) < 0.99f) ? glm::vec3(0, 1, 0) : glm::vec3(1, 0, 0);
        glm::vec3 right = glm::normalize(glm::cross(normal, up));
        glm::vec3 forward = glm::cross(right, normal);

        // Check several points on the circle
        float minDist = 1000.0f;
        int numSamples = 32;
        for (int i = 0; i < numSamples; ++i) {
            float angle = (float)i / numSamples * 2.0f * 3.14159265f;
            glm::vec3 point = center + (right * cos(angle) + forward * sin(angle)) * scale;
            glm::vec2 screenPoint = worldToScreen(point, camera);
            float dist = glm::length(mousePos - screenPoint);
            minDist = glm::min(minDist, dist);
        }

        if (minDist < threshold) {
            return axis;
        }
    }

    return -1;
}

} // namespace PinaEditor
