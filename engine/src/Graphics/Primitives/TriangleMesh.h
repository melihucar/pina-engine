#pragma once

/// Pina Engine - Triangle Mesh Primitive
/// RGB colored triangle with position and color attributes

#include "../Mesh.h"

namespace Pina {

/// Triangle mesh primitive
/// Creates a simple RGB colored triangle
class PINA_API TriangleMesh : public Mesh {
public:
    /// Create a triangle mesh
    /// @param device Graphics device to create resources with
    /// @return Unique pointer to the created triangle mesh
    static UNIQUE<TriangleMesh> create(GraphicsDevice* device);

    ~TriangleMesh() override = default;

private:
    TriangleMesh(GraphicsDevice* device);
};

} // namespace Pina
