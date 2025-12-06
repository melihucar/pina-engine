#include "SceneRenderer.h"
#include "Scene.h"
#include "Node.h"
#include "../Graphics/GraphicsDevice.h"
#include "../Graphics/Shader.h"
#include "../Graphics/Camera.h"
#include "../Graphics/Model.h"
#include "../Graphics/Lighting/LightManager.h"

namespace Pina {

SceneRenderer::SceneRenderer(GraphicsDevice* device)
    : m_device(device)
{
}

SceneRenderer::~SceneRenderer() = default;

void SceneRenderer::render(Scene* scene, Shader* shader) {
    if (!scene || !shader) return;

    // Reset statistics
    m_renderedNodeCount = 0;
    m_drawCallCount = 0;

    Camera* camera = scene->getActiveCamera();
    if (!camera) return;

    // Upload camera matrices
    shader->bind();
    shader->setMat4("uView", camera->getViewMatrix());
    shader->setMat4("uProjection", camera->getProjectionMatrix());
    shader->setVec3("uViewPosition", camera->getPosition());

    // Upload lights
    LightManager& lightManager = scene->getLightManager();
    lightManager.uploadToShader(shader);

    // Render the scene starting from root
    renderNodeRecursive(scene->getRoot(), shader, camera, &lightManager);
}

void SceneRenderer::renderOpaque(Scene* scene, Shader* shader) {
    if (!scene || !shader) return;

    LightManager& lightManager = scene->getLightManager();
    renderNodeRecursivePass(scene->getRoot(), shader, &lightManager, RenderPass::OpaqueOnly);
}

void SceneRenderer::renderTransparent(Scene* scene, Shader* shader) {
    if (!scene || !shader) return;

    LightManager& lightManager = scene->getLightManager();
    renderNodeRecursivePass(scene->getRoot(), shader, &lightManager, RenderPass::TransparentOnly);
}

void SceneRenderer::renderNode(Node* node, Shader* shader, Camera* camera, LightManager* lightManager) {
    if (!node || !shader || !camera) return;

    // Reset statistics
    m_renderedNodeCount = 0;
    m_drawCallCount = 0;

    // Upload camera matrices
    shader->bind();
    shader->setMat4("uView", camera->getViewMatrix());
    shader->setMat4("uProjection", camera->getProjectionMatrix());
    shader->setVec3("uViewPosition", camera->getPosition());

    // Upload lights if provided
    if (lightManager) {
        lightManager->uploadToShader(shader);
    }

    renderNodeRecursive(node, shader, camera, lightManager);
}

void SceneRenderer::renderNodeRecursive(Node* node, Shader* shader, Camera* camera, LightManager* lightManager) {
    if (!node) return;

    // Skip disabled nodes unless rendering disabled is enabled
    if (!m_renderDisabled && !node->isEnabled()) return;

    m_renderedNodeCount++;

    // Render this node's model if it has one
    if (node->hasModel()) {
        Model* model = node->getModel();

        // Get world transform
        const glm::mat4& worldMatrix = node->getTransform().getWorldMatrix();
        glm::mat3 normalMatrix = node->getTransform().getNormalMatrix();

        // Upload model matrix
        shader->setMat4("uModel", worldMatrix);
        shader->setMat3("uNormalMatrix", normalMatrix);

        // Draw the model
        model->draw(shader, lightManager);
        m_drawCallCount += model->getMeshCount();
    }

    // Recursively render children
    for (size_t i = 0; i < node->getChildCount(); ++i) {
        renderNodeRecursive(node->getChild(i), shader, camera, lightManager);
    }
}

void SceneRenderer::renderNodeRecursivePass(Node* node, Shader* shader, LightManager* lightManager, RenderPass pass) {
    if (!node) return;

    // Skip disabled nodes unless rendering disabled is enabled
    if (!m_renderDisabled && !node->isEnabled()) return;

    m_renderedNodeCount++;

    // Render this node's model if it has one
    if (node->hasModel()) {
        Model* model = node->getModel();

        // Get world transform
        const glm::mat4& worldMatrix = node->getTransform().getWorldMatrix();
        glm::mat3 normalMatrix = node->getTransform().getNormalMatrix();

        // Upload model matrix
        shader->setMat4("uModel", worldMatrix);
        shader->setMat3("uNormalMatrix", normalMatrix);

        // Draw based on pass type
        switch (pass) {
            case RenderPass::All:
                model->draw(shader, lightManager);
                m_drawCallCount += model->getMeshCount();
                break;
            case RenderPass::OpaqueOnly:
                model->drawOpaque(shader, lightManager);
                m_drawCallCount += model->getMeshCount();  // Approximate
                break;
            case RenderPass::TransparentOnly:
                model->drawTransparent(shader, lightManager);
                m_drawCallCount += model->getMeshCount();  // Approximate
                break;
        }
    }

    // Recursively render children
    for (size_t i = 0; i < node->getChildCount(); ++i) {
        renderNodeRecursivePass(node->getChild(i), shader, lightManager, pass);
    }
}

} // namespace Pina
