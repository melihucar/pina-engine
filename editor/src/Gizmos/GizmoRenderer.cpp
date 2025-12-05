#include "GizmoRenderer.h"
#include <Pina.h>
#include <iostream>

#ifdef __APPLE__
#include <OpenGL/gl3.h>
#else
#include <GL/gl.h>
#endif

namespace PinaEditor {

// Simple line shader source
static const char* lineVertexShader = R"(
#version 330 core
layout (location = 0) in vec3 a_position;
layout (location = 1) in vec4 a_color;

uniform mat4 u_viewProjection;

out vec4 v_color;

void main() {
    gl_Position = u_viewProjection * vec4(a_position, 1.0);
    v_color = a_color;
}
)";

static const char* lineFragmentShader = R"(
#version 330 core
in vec4 v_color;
out vec4 fragColor;

void main() {
    fragColor = v_color;
}
)";

GizmoRenderer::GizmoRenderer(Pina::GraphicsDevice* device)
    : m_device(device)
{
    createShader();
    createBuffers();
    m_lineVertices.reserve(MAX_LINES * 2);
}

GizmoRenderer::~GizmoRenderer() = default;

void GizmoRenderer::createShader() {
    m_lineShader = m_device->createShader();
    if (!m_lineShader->load(lineVertexShader, lineFragmentShader)) {
        std::cerr << "Failed to create gizmo line shader!" << std::endl;
    }
}

void GizmoRenderer::createBuffers() {
    // Create vertex buffer with initial empty data
    m_vertexBuffer = m_device->createVertexBuffer(nullptr, MAX_LINES * 2 * sizeof(LineVertex));

    // Create vertex array
    m_vertexArray = m_device->createVertexArray();

    // Define layout: position (vec3) + color (vec4)
    Pina::VertexLayout layout;
    layout.push("a_position", Pina::ShaderDataType::Float3);  // location 0: vec3
    layout.push("a_color", Pina::ShaderDataType::Float4);     // location 1: vec4

    m_vertexArray->addVertexBuffer(m_vertexBuffer.get(), layout);
}

void GizmoRenderer::begin() {
    m_lineVertices.clear();
}

void GizmoRenderer::drawLine(const glm::vec3& start, const glm::vec3& end, const glm::vec4& color) {
    if (m_lineVertices.size() >= MAX_LINES * 2) return;

    m_lineVertices.push_back({start, color});
    m_lineVertices.push_back({end, color});
}

void GizmoRenderer::drawCircle(const glm::vec3& center, const glm::vec3& normal, float radius, const glm::vec4& color, int segments) {
    // Create orthonormal basis
    glm::vec3 n = glm::normalize(normal);
    glm::vec3 up = (glm::abs(n.y) < 0.99f) ? glm::vec3(0, 1, 0) : glm::vec3(1, 0, 0);
    glm::vec3 right = glm::normalize(glm::cross(n, up));
    glm::vec3 forward = glm::cross(right, n);

    float angleStep = 2.0f * 3.14159265f / static_cast<float>(segments);
    glm::vec3 lastPoint = center + right * radius;

    for (int i = 1; i <= segments; ++i) {
        float angle = angleStep * static_cast<float>(i);
        glm::vec3 point = center + (right * cos(angle) + forward * sin(angle)) * radius;
        drawLine(lastPoint, point, color);
        lastPoint = point;
    }
}

void GizmoRenderer::drawCube(const glm::vec3& center, float size, const glm::vec4& color) {
    float h = size * 0.5f;

    // Bottom face
    drawLine(center + glm::vec3(-h, -h, -h), center + glm::vec3(h, -h, -h), color);
    drawLine(center + glm::vec3(h, -h, -h), center + glm::vec3(h, -h, h), color);
    drawLine(center + glm::vec3(h, -h, h), center + glm::vec3(-h, -h, h), color);
    drawLine(center + glm::vec3(-h, -h, h), center + glm::vec3(-h, -h, -h), color);

    // Top face
    drawLine(center + glm::vec3(-h, h, -h), center + glm::vec3(h, h, -h), color);
    drawLine(center + glm::vec3(h, h, -h), center + glm::vec3(h, h, h), color);
    drawLine(center + glm::vec3(h, h, h), center + glm::vec3(-h, h, h), color);
    drawLine(center + glm::vec3(-h, h, h), center + glm::vec3(-h, h, -h), color);

    // Vertical edges
    drawLine(center + glm::vec3(-h, -h, -h), center + glm::vec3(-h, h, -h), color);
    drawLine(center + glm::vec3(h, -h, -h), center + glm::vec3(h, h, -h), color);
    drawLine(center + glm::vec3(h, -h, h), center + glm::vec3(h, h, h), color);
    drawLine(center + glm::vec3(-h, -h, h), center + glm::vec3(-h, h, h), color);
}

void GizmoRenderer::drawCubeTransformed(const glm::mat4& transform, float size, const glm::vec4& color) {
    float h = size * 0.5f;

    // Define cube vertices in local space
    glm::vec3 vertices[8] = {
        glm::vec3(-h, -h, -h),
        glm::vec3( h, -h, -h),
        glm::vec3( h, -h,  h),
        glm::vec3(-h, -h,  h),
        glm::vec3(-h,  h, -h),
        glm::vec3( h,  h, -h),
        glm::vec3( h,  h,  h),
        glm::vec3(-h,  h,  h)
    };

    // Transform all vertices
    glm::vec3 transformed[8];
    for (int i = 0; i < 8; ++i) {
        glm::vec4 v = transform * glm::vec4(vertices[i], 1.0f);
        transformed[i] = glm::vec3(v);
    }

    // Bottom face (0-1-2-3)
    drawLine(transformed[0], transformed[1], color);
    drawLine(transformed[1], transformed[2], color);
    drawLine(transformed[2], transformed[3], color);
    drawLine(transformed[3], transformed[0], color);

    // Top face (4-5-6-7)
    drawLine(transformed[4], transformed[5], color);
    drawLine(transformed[5], transformed[6], color);
    drawLine(transformed[6], transformed[7], color);
    drawLine(transformed[7], transformed[4], color);

    // Vertical edges
    drawLine(transformed[0], transformed[4], color);
    drawLine(transformed[1], transformed[5], color);
    drawLine(transformed[2], transformed[6], color);
    drawLine(transformed[3], transformed[7], color);
}

void GizmoRenderer::drawArrow(const glm::vec3& from, const glm::vec3& to, const glm::vec4& color, float headSize) {
    // Main line
    drawLine(from, to, color);

    // Arrow head
    glm::vec3 dir = glm::normalize(to - from);
    glm::vec3 up = (glm::abs(dir.y) < 0.99f) ? glm::vec3(0, 1, 0) : glm::vec3(1, 0, 0);
    glm::vec3 right = glm::normalize(glm::cross(dir, up));
    glm::vec3 forward = glm::cross(right, dir);

    glm::vec3 headBase = to - dir * headSize;
    float headRadius = headSize * 0.3f;

    // Draw 4 lines for arrow head
    drawLine(to, headBase + right * headRadius, color);
    drawLine(to, headBase - right * headRadius, color);
    drawLine(to, headBase + forward * headRadius, color);
    drawLine(to, headBase - forward * headRadius, color);
}

void GizmoRenderer::flush(Pina::Camera* camera) {
    if (m_lineVertices.empty() || !camera) return;

    // Upload vertex data
    m_vertexBuffer->setData(m_lineVertices.data(), m_lineVertices.size() * sizeof(LineVertex));

    // Setup shader
    m_lineShader->bind();
    glm::mat4 viewProj = camera->getProjectionMatrix() * camera->getViewMatrix();
    m_lineShader->setMat4("u_viewProjection", viewProj);

    // Bind VAO
    m_vertexArray->bind();

    // Set line width
    glLineWidth(m_lineWidth);

    // Disable depth test for gizmos to always be visible
    glDisable(GL_DEPTH_TEST);

    // Draw lines
    glDrawArrays(GL_LINES, 0, static_cast<GLsizei>(m_lineVertices.size()));

    // Re-enable depth test
    glEnable(GL_DEPTH_TEST);

    // Cleanup
    m_vertexArray->unbind();

    m_lineVertices.clear();
}

} // namespace PinaEditor
