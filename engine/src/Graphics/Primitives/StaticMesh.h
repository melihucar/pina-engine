#pragma once

/// Pina Engine - Static Mesh
/// Mesh class for loaded 3D geometry with indexed rendering

#include "../Mesh.h"
#include <vector>
#include <cstdint>

namespace Pina {

/// Static mesh for loaded 3D geometry
/// Uses indexed rendering for efficient vertex reuse
class PINA_API StaticMesh : public Mesh {
public:
    /// Create a static mesh from vertex and index data
    /// @param device Graphics device
    /// @param vertices Interleaved vertex data (position, normal, texcoord)
    /// @param vertexCount Number of vertices
    /// @param indices Index data
    /// @param indexCount Number of indices
    /// @return Unique pointer to the created mesh
    static UNIQUE<StaticMesh> create(GraphicsDevice* device,
                                     const float* vertices,
                                     uint32_t vertexCount,
                                     const uint32_t* indices,
                                     uint32_t indexCount);

    /// Create from vectors (convenience)
    static UNIQUE<StaticMesh> create(GraphicsDevice* device,
                                     const std::vector<float>& vertices,
                                     const std::vector<uint32_t>& indices);

    ~StaticMesh() override = default;

    /// Draw using indexed rendering
    void draw();

    /// Get index count
    uint32_t getIndexCount() const { return m_indexCount; }

private:
    StaticMesh(GraphicsDevice* device,
               const float* vertices,
               uint32_t vertexCount,
               const uint32_t* indices,
               uint32_t indexCount);

    UNIQUE<IndexBuffer> m_ibo;
    uint32_t m_indexCount = 0;
};

} // namespace Pina
