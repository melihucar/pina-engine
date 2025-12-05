#include "ScaleGizmo.h"
#include "GizmoRenderer.h"
#include <Pina.h>

namespace PinaEditor {

void ScaleGizmo::render(GizmoRenderer* renderer, const glm::mat4& transform, Pina::Camera* camera) {
    if (!renderer || !camera) return;

    glm::vec3 center = glm::vec3(transform[3]);
    float scale = calculateGizmoScale(center, camera);
    float cubeSize = scale * 0.1f;

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

    // Determine colors
    glm::vec4 xColor = (m_activeAxis == 0) ? HOVER_COLOR : X_AXIS_COLOR;
    glm::vec4 yColor = (m_activeAxis == 1) ? HOVER_COLOR : Y_AXIS_COLOR;
    glm::vec4 zColor = (m_activeAxis == 2) ? HOVER_COLOR : Z_AXIS_COLOR;

    // Draw lines
    renderer->drawLine(center, center + xAxis * scale, xColor);
    renderer->drawLine(center, center + yAxis * scale, yColor);
    renderer->drawLine(center, center + zAxis * scale, zColor);

    // Draw cubes at the end
    renderer->drawCube(center + xAxis * scale, cubeSize, xColor);
    renderer->drawCube(center + yAxis * scale, cubeSize, yColor);
    renderer->drawCube(center + zAxis * scale, cubeSize, zColor);

    // Draw center cube
    renderer->drawCube(center, cubeSize * 0.8f, glm::vec4(1.0f));
}

bool ScaleGizmo::handleInput(Pina::Input* input, Pina::Camera* camera, Pina::Transform* target) {
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
            m_dragStartScale = target->getLocalScale();
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

            // Calculate scale based on mouse movement
            float sensitivity = 0.01f;
            float scaleFactor = 1.0f + (delta.x - delta.y) * sensitivity;
            scaleFactor = glm::max(scaleFactor, 0.01f);

            glm::vec3 newScale = m_dragStartScale;
            if (m_activeAxis == 0) newScale.x *= scaleFactor;
            else if (m_activeAxis == 1) newScale.y *= scaleFactor;
            else if (m_activeAxis == 2) newScale.z *= scaleFactor;
            else newScale *= scaleFactor; // Uniform scale

            target->setLocalScale(newScale);
            return true;
        }
    }

    return false;
}

int ScaleGizmo::checkAxisHover(Pina::Input* input, Pina::Camera* camera, const glm::vec3& center) {
    glm::vec2 windowMousePos = input->getMousePosition();
    // Convert window mouse position to viewport-relative coordinates
    glm::vec2 mousePos = windowMousePos - m_viewportPosition;
    float scale = calculateGizmoScale(center, camera);
    float threshold = 20.0f;

    // Check cube at each axis end
    for (int axis = 0; axis < 3; ++axis) {
        glm::vec3 axisDir = glm::vec3(axis == 0, axis == 1, axis == 2);
        glm::vec3 cubePos = center + axisDir * scale;
        glm::vec2 screenPos = worldToScreen(cubePos, camera);

        float dist = glm::length(mousePos - screenPos);
        if (dist < threshold) {
            return axis;
        }
    }

    // Check center cube for uniform scale
    glm::vec2 centerScreen = worldToScreen(center, camera);
    if (glm::length(mousePos - centerScreen) < threshold * 0.8f) {
        return 3; // Special value for uniform scale
    }

    return -1;
}

} // namespace PinaEditor
