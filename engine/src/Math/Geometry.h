#pragma once

/// Pina Engine - Geometry utilities
/// Common geometric operations and intersection tests

#include "../Core/Export.h"
#include "Vector2.h"
#include "Vector3.h"
#include "Ray.h"
#include "Plane.h"
#include "Mathf.h"
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>

namespace Pina {

/// Collection of geometric utility functions
class PINA_API Geometry {
public:
    /// Find the closest point on a line segment to a point
    static Vector3 closestPointOnLineSegment(const Vector3& point,
                                              const Vector3& lineStart,
                                              const Vector3& lineEnd) {
        Vector3 line = lineEnd - lineStart;
        float lineLength = line.length();
        if (lineLength < Mathf::EPSILON) {
            return lineStart;
        }

        Vector3 lineDir = line / lineLength;
        Vector3 toPoint = point - lineStart;
        float t = Vector3::dot(toPoint, lineDir);
        t = Mathf::clamp(t, 0.0f, lineLength);

        return lineStart + lineDir * t;
    }

    /// Find the closest point on an infinite line to a point
    static Vector3 closestPointOnLine(const Vector3& point,
                                       const Vector3& lineOrigin,
                                       const Vector3& lineDirection) {
        Vector3 dir = lineDirection.normalized();
        Vector3 toPoint = point - lineOrigin;
        float t = Vector3::dot(toPoint, dir);
        return lineOrigin + dir * t;
    }

    /// Find the closest points between two lines (or the single closest point if they intersect)
    /// Returns the parameter t on line1 and s on line2
    /// Line1: origin1 + t * direction1
    /// Line2: origin2 + s * direction2
    static bool closestPointsBetweenLines(const Vector3& origin1, const Vector3& direction1,
                                           const Vector3& origin2, const Vector3& direction2,
                                           float& t, float& s) {
        Vector3 w0 = origin1 - origin2;
        float a = Vector3::dot(direction1, direction1);
        float b = Vector3::dot(direction1, direction2);
        float c = Vector3::dot(direction2, direction2);
        float d = Vector3::dot(direction1, w0);
        float e = Vector3::dot(direction2, w0);

        float denom = a * c - b * b;
        if (Mathf::abs(denom) < Mathf::EPSILON) {
            // Lines are parallel
            t = 0.0f;
            s = (b > c ? d / b : e / c);
            return false;
        }

        t = (b * e - c * d) / denom;
        s = (a * e - b * d) / denom;
        return true;
    }

    /// Project a ray onto an axis line, returning the point on the axis
    /// axis: normalized direction of the axis
    /// rayOrigin: camera position
    /// rayDir: normalized ray direction
    /// axisCenter: a point on the axis
    static Vector3 projectRayOntoAxis(const Vector3& axis,
                                       const Vector3& rayOrigin,
                                       const Vector3& rayDir,
                                       const Vector3& axisCenter) {
        Vector3 w0 = rayOrigin - axisCenter;
        float a = Vector3::dot(rayDir, rayDir);
        float b = Vector3::dot(rayDir, axis);
        float c = Vector3::dot(axis, axis);
        float d = Vector3::dot(rayDir, w0);
        float e = Vector3::dot(axis, w0);

        float denom = a * c - b * b;
        if (Mathf::abs(denom) < Mathf::EPSILON) {
            return axisCenter;
        }

        float t = (a * e - d * b) / denom;
        return axisCenter + axis * t;
    }

    /// Distance from a 2D point to a line segment
    static float distanceToLineSegment2D(const Vector2& point,
                                          const Vector2& lineStart,
                                          const Vector2& lineEnd) {
        Vector2 line = lineEnd - lineStart;
        float lineLength = line.length();
        if (lineLength < Mathf::EPSILON) {
            return Vector2::distance(point, lineStart);
        }

        Vector2 lineDir = line / lineLength;
        Vector2 toPoint = point - lineStart;
        float t = Vector2::dot(toPoint, lineDir);
        t = Mathf::clamp(t, 0.0f, lineLength);

        Vector2 closest = lineStart + lineDir * t;
        return Vector2::distance(point, closest);
    }

    /// Project a 3D world point to 2D screen coordinates
    /// Returns coordinates in pixels from top-left origin
    static Vector2 worldToScreen(const Vector3& worldPos,
                                  const glm::mat4& viewProjection,
                                  float screenWidth,
                                  float screenHeight) {
        glm::vec4 clipPos = viewProjection * glm::vec4(worldPos.x, worldPos.y, worldPos.z, 1.0f);

        if (clipPos.w <= 0.0f) {
            return Vector2(-10000.0f, -10000.0f);  // Behind camera
        }

        glm::vec3 ndcPos = glm::vec3(clipPos) / clipPos.w;

        float screenX = (ndcPos.x + 1.0f) * 0.5f * screenWidth;
        float screenY = (1.0f - ndcPos.y) * 0.5f * screenHeight;  // Y-flip for top-left origin

        return Vector2(screenX, screenY);
    }

    /// Create a ray from screen coordinates through the camera
    /// screenPos: pixel coordinates from top-left origin
    static Ray screenToRay(const Vector2& screenPos,
                            float screenWidth,
                            float screenHeight,
                            const glm::mat4& invViewProjection) {
        // Convert to NDC
        float ndcX = (screenPos.x / screenWidth) * 2.0f - 1.0f;
        float ndcY = 1.0f - (screenPos.y / screenHeight) * 2.0f;  // Y-flip for top-left origin

        // Near and far points in clip space
        glm::vec4 nearClip(ndcX, ndcY, -1.0f, 1.0f);
        glm::vec4 farClip(ndcX, ndcY, 1.0f, 1.0f);

        // Transform to world space
        glm::vec4 nearWorld = invViewProjection * nearClip;
        glm::vec4 farWorld = invViewProjection * farClip;
        nearWorld /= nearWorld.w;
        farWorld /= farWorld.w;

        Vector3 origin(nearWorld.x, nearWorld.y, nearWorld.z);
        Vector3 direction(farWorld.x - nearWorld.x,
                          farWorld.y - nearWorld.y,
                          farWorld.z - nearWorld.z);

        return Ray(origin, direction.normalized());
    }

    /// Create orthonormal basis vectors from a normal
    static void createOrthonormalBasis(const Vector3& normal,
                                        Vector3& outRight,
                                        Vector3& outForward) {
        Vector3 n = normal.normalized();
        Vector3 up = (Mathf::abs(n.y) < 0.99f) ? Vector3::up() : Vector3::right();
        outRight = Vector3::cross(n, up).normalized();
        outForward = Vector3::cross(outRight, n);
    }

    /// Calculate the angle between two vectors in radians
    static float angleBetween(const Vector3& a, const Vector3& b) {
        float dot = Vector3::dot(a.normalized(), b.normalized());
        dot = Mathf::clamp(dot, -1.0f, 1.0f);
        return Mathf::acos(dot);
    }

    /// Calculate the signed angle between two vectors around an axis in radians
    static float signedAngleBetween(const Vector3& from, const Vector3& to, const Vector3& axis) {
        float angle = angleBetween(from, to);
        Vector3 cross = Vector3::cross(from, to);
        if (Vector3::dot(cross, axis) < 0.0f) {
            angle = -angle;
        }
        return angle;
    }
};

} // namespace Pina
