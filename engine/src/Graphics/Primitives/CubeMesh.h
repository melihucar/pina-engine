#pragma once

/// Pina Engine - Cube Mesh Primitive
/// Textured cube with position, normal, and texture coordinates

#include "../Mesh.h"

namespace Pina {

/// Cube mesh primitive
/// Creates a textured cube with normals
class PINA_API CubeMesh : public Mesh {
public:
    /// Create a cube mesh
    /// @param device Graphics device to create resources with
    /// @param size Size of the cube (default 1.0)
    /// @return Unique pointer to the created cube mesh
    static UNIQUE<CubeMesh> create(GraphicsDevice* device, float size = 1.0f);

    ~CubeMesh() override = default;

private:
    CubeMesh(GraphicsDevice* device, float size);
};

} // namespace Pina
