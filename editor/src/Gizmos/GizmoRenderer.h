#pragma once

#include <Pina.h>
#include <glm/glm.hpp>
#include <vector>

namespace Pina {
    class Shader;
    class Camera;
    class VertexBuffer;
    class VertexArray;
    class GraphicsDevice;
}

namespace PinaEditor {

/// Renders lines and simple shapes for gizmos
class GizmoRenderer {
public:
    explicit GizmoRenderer(Pina::GraphicsDevice* device);
    ~GizmoRenderer();

    // Begin a new frame of gizmo drawing
    void begin();

    // Draw primitives (batched until flush)
    void drawLine(const glm::vec3& start, const glm::vec3& end, const glm::vec4& color);
    void drawCircle(const glm::vec3& center, const glm::vec3& normal, float radius, const glm::vec4& color, int segments = 32);
    void drawCube(const glm::vec3& center, float size, const glm::vec4& color);
    void drawCubeTransformed(const glm::mat4& transform, float size, const glm::vec4& color);
    void drawArrow(const glm::vec3& from, const glm::vec3& to, const glm::vec4& color, float headSize = 0.1f);

    // Flush all batched primitives
    void flush(Pina::Camera* camera);

    // Set line width (may not be supported on all platforms)
    void setLineWidth(float width) { m_lineWidth = width; }

private:
    struct LineVertex {
        glm::vec3 position;
        glm::vec4 color;
    };

    void createShader();
    void createBuffers();

    Pina::GraphicsDevice* m_device = nullptr;
    Pina::UNIQUE<Pina::Shader> m_lineShader;
    Pina::UNIQUE<Pina::VertexBuffer> m_vertexBuffer;
    Pina::UNIQUE<Pina::VertexArray> m_vertexArray;

    std::vector<LineVertex> m_lineVertices;
    float m_lineWidth = 2.0f;

    static constexpr size_t MAX_LINES = 10000;
};

} // namespace PinaEditor
