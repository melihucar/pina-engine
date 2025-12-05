#include "Gizmo.h"
#include <Pina.h>

namespace PinaEditor {

glm::vec3 Gizmo::getMouseRay(Pina::Input* input, Pina::Camera* camera) const {
    if (!input || !camera) return glm::vec3(0.0f, 0.0f, -1.0f);

    glm::vec2 mousePos = input->getMousePosition();

    // Convert window mouse position to viewport-relative coordinates
    float viewportX = mousePos.x - m_viewportPosition.x;
    float viewportY = mousePos.y - m_viewportPosition.y;

    // Use actual viewport dimensions
    float width = m_viewportSize.x;
    float height = m_viewportSize.y;

    // Convert to NDC
    float x = (2.0f * viewportX) / width - 1.0f;
    float y = 1.0f - (2.0f * viewportY) / height;

    // Get inverse matrices
    glm::mat4 invProj = glm::inverse(camera->getProjectionMatrix());
    glm::mat4 invView = glm::inverse(camera->getViewMatrix());

    // Ray in clip space
    glm::vec4 rayClip(x, y, -1.0f, 1.0f);

    // Ray in view space
    glm::vec4 rayEye = invProj * rayClip;
    rayEye = glm::vec4(rayEye.x, rayEye.y, -1.0f, 0.0f);

    // Ray in world space
    glm::vec3 rayWorld = glm::vec3(invView * rayEye);
    return glm::normalize(rayWorld);
}

glm::vec2 Gizmo::worldToScreen(const glm::vec3& worldPos, Pina::Camera* camera) const {
    if (!camera) return glm::vec2(0.0f);

    glm::mat4 viewProj = camera->getProjectionMatrix() * camera->getViewMatrix();
    glm::vec4 clipPos = viewProj * glm::vec4(worldPos, 1.0f);

    if (clipPos.w <= 0.0f) return glm::vec2(-1000.0f);

    glm::vec3 ndcPos = glm::vec3(clipPos) / clipPos.w;

    // Use actual viewport dimensions
    float width = m_viewportSize.x;
    float height = m_viewportSize.y;

    // Convert NDC to viewport-relative coordinates (NOT window coordinates)
    // This matches the viewport-relative mouse coordinates used in checkAxisHover
    float screenX = (ndcPos.x + 1.0f) * 0.5f * width;
    float screenY = (1.0f - ndcPos.y) * 0.5f * height;

    return glm::vec2(screenX, screenY);
}

float Gizmo::calculateGizmoScale(const glm::vec3& position, Pina::Camera* camera) const {
    if (!camera) return m_size;

    float distance = glm::length(position - camera->getPosition());
    // Scale gizmo based on distance so it appears constant size on screen
    return m_size * distance * 0.15f;
}

} // namespace PinaEditor
