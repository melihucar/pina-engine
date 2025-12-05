#include "TranslateGizmo.h"
#include "GizmoRenderer.h"
#include <Pina.h>

namespace PinaEditor {

void TranslateGizmo::render(GizmoRenderer* renderer, const glm::mat4& transform, Pina::Camera* camera) {
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

    // Draw arrows with appropriate colors
    glm::vec4 xColor = (m_activeAxis == 0 || m_isHovered && m_activeAxis == -1) ? HOVER_COLOR : X_AXIS_COLOR;
    glm::vec4 yColor = (m_activeAxis == 1 || m_isHovered && m_activeAxis == -1) ? HOVER_COLOR : Y_AXIS_COLOR;
    glm::vec4 zColor = (m_activeAxis == 2 || m_isHovered && m_activeAxis == -1) ? HOVER_COLOR : Z_AXIS_COLOR;

    // Adjust colors if this axis is being dragged
    if (m_isDragging) {
        xColor = (m_activeAxis == 0) ? HOVER_COLOR : X_AXIS_COLOR;
        yColor = (m_activeAxis == 1) ? HOVER_COLOR : Y_AXIS_COLOR;
        zColor = (m_activeAxis == 2) ? HOVER_COLOR : Z_AXIS_COLOR;
    }

    renderer->drawArrow(center, center + xAxis * scale, xColor, scale * 0.15f);
    renderer->drawArrow(center, center + yAxis * scale, yColor, scale * 0.15f);
    renderer->drawArrow(center, center + zAxis * scale, zColor, scale * 0.15f);
}

bool TranslateGizmo::handleInput(Pina::Input* input, Pina::Camera* camera, Pina::Transform* target) {
    if (!input || !camera || !target) return false;

    glm::vec3 center = target->getWorldPosition();

    // Check for mouse button state
    bool mouseDown = input->isMouseButtonDown(Pina::MouseButton::Left);
    bool mousePressed = input->isMouseButtonPressed(Pina::MouseButton::Left);
    bool mouseReleased = input->isMouseButtonReleased(Pina::MouseButton::Left);

    if (!m_isDragging) {
        // Check hover
        m_activeAxis = checkAxisHover(input, camera, center);
        m_isHovered = (m_activeAxis != -1);

        // Start dragging on mouse press
        if (mousePressed && m_activeAxis != -1) {
            m_isDragging = true;
            m_dragStartObjectPos = target->getLocalPosition();
            m_dragStartCenter = center;  // Store original center

            glm::vec3 rayOrigin = camera->getPosition();
            glm::vec3 rayDir = getMouseRay(input, camera);

            if (m_space == GizmoSpace::Local) {
                glm::mat4 worldMat = target->getWorldMatrix();
                m_dragAxis = glm::normalize(glm::vec3(worldMat[m_activeAxis]));
            } else {
                m_dragAxis = glm::vec3(m_activeAxis == 0, m_activeAxis == 1, m_activeAxis == 2);
            }

            m_dragStartPos = projectOntoAxis(m_dragAxis, rayOrigin, rayDir, m_dragStartCenter);
        }
    }

    if (m_isDragging) {
        if (mouseReleased) {
            m_isDragging = false;
            m_activeAxis = -1;
            return true;
        }

        if (mouseDown) {
            glm::vec3 rayOrigin = camera->getPosition();
            glm::vec3 rayDir = getMouseRay(input, camera);

            // Use stored axis and center for consistent projection
            glm::vec3 currentPos = projectOntoAxis(m_dragAxis, rayOrigin, rayDir, m_dragStartCenter);
            glm::vec3 delta = currentPos - m_dragStartPos;

            // Apply translation
            target->setLocalPosition(m_dragStartObjectPos + delta);

            return true;
        }
    }

    return false;
}

int TranslateGizmo::checkAxisHover(Pina::Input* input, Pina::Camera* camera, const glm::vec3& center) {
    glm::vec2 windowMousePos = input->getMousePosition();
    // Convert window mouse position to viewport-relative coordinates
    glm::vec2 mousePos = windowMousePos - m_viewportPosition;
    float scale = calculateGizmoScale(center, camera);
    float threshold = 15.0f; // Pixel threshold for hovering

    // Check each axis
    for (int axis = 0; axis < 3; ++axis) {
        glm::vec3 axisDir = glm::vec3(axis == 0, axis == 1, axis == 2);
        glm::vec3 axisEnd = center + axisDir * scale;

        glm::vec2 screenStart = worldToScreen(center, camera);
        glm::vec2 screenEnd = worldToScreen(axisEnd, camera);

        // Calculate distance from mouse to line segment
        glm::vec2 line = screenEnd - screenStart;
        float lineLen = glm::length(line);
        if (lineLen < 1.0f) continue;

        glm::vec2 toMouse = mousePos - screenStart;
        float t = glm::clamp(glm::dot(toMouse, line) / (lineLen * lineLen), 0.0f, 1.0f);
        glm::vec2 closest = screenStart + line * t;
        float dist = glm::length(mousePos - closest);

        if (dist < threshold) {
            return axis;
        }
    }

    return -1;
}

glm::vec3 TranslateGizmo::projectOntoAxis(const glm::vec3& axis, const glm::vec3& rayOrigin,
                                           const glm::vec3& rayDir, const glm::vec3& center) {
    // Find closest point on axis to the ray using line-line closest point formula
    // Line 1 (ray): P(s) = rayOrigin + s * rayDir
    // Line 2 (axis): Q(t) = center + t * axis
    // We want to find t for the closest point on the axis

    glm::vec3 w0 = rayOrigin - center;
    float a = glm::dot(rayDir, rayDir);      // |rayDir|^2, should be 1 if normalized
    float b = glm::dot(rayDir, axis);         // ray · axis
    float c = glm::dot(axis, axis);           // |axis|^2, should be 1 if normalized
    float d = glm::dot(rayDir, w0);           // ray · (rayOrigin - center)
    float e = glm::dot(axis, w0);             // axis · (rayOrigin - center)

    float denom = a * c - b * b;
    if (glm::abs(denom) < 0.0001f) {
        // Lines are parallel
        return center;
    }

    // Correct formula: t = (a*e - d*b) / (a*c - b*b)
    float t = (a * e - d * b) / denom;
    return center + axis * t;
}

} // namespace PinaEditor
