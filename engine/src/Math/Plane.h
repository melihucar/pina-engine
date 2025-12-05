#pragma once

/// Pina Engine - Plane
/// Represents a plane in 3D space

#include "../Core/Export.h"
#include "Vector3.h"
#include "Ray.h"
#include <glm/vec3.hpp>

namespace Pina {

/// A plane defined by a normal and distance from origin
class PINA_API Plane {
public:
    Vector3 normal;
    float distance;

    // Constructors
    Plane() : normal(0.0f, 1.0f, 0.0f), distance(0.0f) {}
    Plane(const Vector3& normal, float distance)
        : normal(normal.normalized()), distance(distance) {}
    Plane(const Vector3& normal, const Vector3& point)
        : normal(normal.normalized()), distance(-Vector3::dot(normal.normalized(), point)) {}
    Plane(const Vector3& a, const Vector3& b, const Vector3& c) {
        normal = Vector3::cross(b - a, c - a).normalized();
        distance = -Vector3::dot(normal, a);
    }

    /// Get the signed distance from a point to the plane
    float getDistanceToPoint(const Vector3& point) const {
        return Vector3::dot(normal, point) + distance;
    }

    /// Check which side of the plane a point is on
    /// Returns positive if on the normal side, negative if behind
    float getSide(const Vector3& point) const {
        return getDistanceToPoint(point);
    }

    /// Check if a point is on the positive side of the plane
    bool isOnPositiveSide(const Vector3& point) const {
        return getDistanceToPoint(point) > 0.0f;
    }

    /// Get the closest point on the plane to a given point
    Vector3 closestPoint(const Vector3& point) const {
        float dist = getDistanceToPoint(point);
        return point - normal * dist;
    }

    /// Raycast against this plane
    /// Returns true if the ray intersects the plane, and sets distance to the intersection point
    bool raycast(const Ray& ray, float& hitDistance) const {
        float denom = Vector3::dot(normal, ray.direction);
        if (std::abs(denom) < 1e-6f) {
            return false;  // Ray is parallel to plane
        }

        float t = -(Vector3::dot(normal, ray.origin) + distance) / denom;
        if (t < 0.0f) {
            return false;  // Intersection is behind the ray origin
        }

        hitDistance = t;
        return true;
    }

    // Common planes
    static Plane XY() { return Plane(Vector3::forward(), 0.0f); }
    static Plane XZ() { return Plane(Vector3::up(), 0.0f); }
    static Plane YZ() { return Plane(Vector3::right(), 0.0f); }
};

} // namespace Pina
