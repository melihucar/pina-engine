#pragma once

/// Pina Engine - Quad Mesh Primitive
/// Textured quad with position and texture coordinates

#include "../Mesh.h"

namespace Pina {

/// Quad mesh primitive
/// Creates a textured quad (two triangles)
class PINA_API QuadMesh : public Mesh {
public:
    /// Create a quad mesh
    /// @param device Graphics device to create resources with
    /// @param width Width of the quad (default 1.0)
    /// @param height Height of the quad (default 1.0)
    /// @return Unique pointer to the created quad mesh
    static UNIQUE<QuadMesh> create(GraphicsDevice* device, float width = 1.0f, float height = 1.0f);

    ~QuadMesh() override = default;

private:
    QuadMesh(GraphicsDevice* device, float width, float height);
};

} // namespace Pina
