#pragma once

/// Pina Engine - Ray
/// Represents a ray with origin and direction

#include "../Core/Export.h"
#include "Vector3.h"
#include <glm/vec3.hpp>

namespace Pina {

/// A ray with origin and direction
class PINA_API Ray {
public:
    Vector3 origin;
    Vector3 direction;

    // Constructors
    Ray() : origin(0.0f, 0.0f, 0.0f), direction(0.0f, 0.0f, -1.0f) {}
    Ray(const Vector3& origin, const Vector3& direction)
        : origin(origin), direction(direction.normalized()) {}
    Ray(const glm::vec3& origin, const glm::vec3& direction)
        : origin(origin), direction(Vector3(direction).normalized()) {}

    /// Get a point along the ray at distance t
    Vector3 getPoint(float t) const {
        return origin + direction * t;
    }

    /// Create a ray from camera screen point
    /// screenPos: normalized screen coordinates (0-1 range, origin at bottom-left)
    /// invViewProj: inverse of view-projection matrix
    static Ray fromScreen(const Vector2& screenPos, const glm::mat4& invViewProj) {
        // Convert to NDC (-1 to 1)
        float ndcX = screenPos.x * 2.0f - 1.0f;
        float ndcY = screenPos.y * 2.0f - 1.0f;

        // Near and far points in NDC
        glm::vec4 nearPoint(ndcX, ndcY, -1.0f, 1.0f);
        glm::vec4 farPoint(ndcX, ndcY, 1.0f, 1.0f);

        // Transform to world space
        glm::vec4 nearWorld = invViewProj * nearPoint;
        glm::vec4 farWorld = invViewProj * farPoint;

        // Perspective divide
        nearWorld /= nearWorld.w;
        farWorld /= farWorld.w;

        Vector3 origin(nearWorld.x, nearWorld.y, nearWorld.z);
        Vector3 direction = Vector3(farWorld.x - nearWorld.x,
                                     farWorld.y - nearWorld.y,
                                     farWorld.z - nearWorld.z).normalized();

        return Ray(origin, direction);
    }
};

} // namespace Pina
