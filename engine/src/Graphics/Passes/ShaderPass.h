#pragma once

/// Pina Engine - Shader Pass
/// Generic fullscreen shader effect pass

#include "../RenderPass.h"
#include "../RenderContext.h"
#include "../Shader.h"
#include "../GraphicsDevice.h"
#include "../../Core/Memory.h"
#include <glm/glm.hpp>
#include <unordered_map>
#include <variant>
#include <string>
#include <iostream>

namespace Pina {

/// Variant type for shader uniforms
using UniformValue = std::variant<
    int,
    float,
    glm::vec2,
    glm::vec3,
    glm::vec4,
    glm::mat3,
    glm::mat4
>;

/// Generic fullscreen shader pass for post-processing effects
class PINA_API ShaderPass : public RenderPass {
public:
    /// Create a shader pass with an existing shader
    explicit ShaderPass(const std::string& passName, Shader* shader)
        : m_shader(shader)
        , m_ownsShader(false)
    {
        name = passName;
        needsSwap = true;  // Post-process passes typically need buffer swap
    }

    /// Create a shader pass with shader source code
    ShaderPass(const std::string& passName,
               const std::string& vertexSource,
               const std::string& fragmentSource)
        : m_ownsShader(true)
    {
        name = passName;
        needsSwap = true;
        m_vertexSource = vertexSource;
        m_fragmentSource = fragmentSource;
    }

    void initialize(RenderContext& ctx) override {
        // Create shader from source if needed
        if (m_ownsShader && !m_ownedShader && ctx.device) {
            m_ownedShader = ctx.device->createShader();
            if (m_ownedShader) {
                if (!m_ownedShader->load(m_vertexSource.c_str(), m_fragmentSource.c_str())) {
                    std::cerr << "ShaderPass '" << name << "': Failed to compile shader" << std::endl;
                }
                m_shader = m_ownedShader.get();
            }
        }
    }

    void execute(RenderContext& ctx) override {
        if (!m_shader) {
            return;
        }

        // Bind output
        bindOutput(ctx);

        // Bind shader
        m_shader->bind();

        // Bind input texture from read buffer
        if (ctx.readBuffer) {
            m_shader->setInt("uInputTexture", 0);
            // Bind the texture - would need OpenGL call
            // glActiveTexture(GL_TEXTURE0);
            // glBindTexture(GL_TEXTURE_2D, ctx.readBuffer->getColorAttachmentID());
        }

        // Upload built-in uniforms
        m_shader->setVec2("uResolution", glm::vec2(ctx.viewportWidth, ctx.viewportHeight));
        m_shader->setFloat("uTime", ctx.totalTime);

        // Upload custom uniforms
        for (const auto& [uniformName, value] : m_uniforms) {
            uploadUniform(uniformName, value);
        }

        // Draw fullscreen quad
        if (ctx.drawFullscreenQuad) {
            ctx.drawFullscreenQuad();
        }
    }

    // ========================================================================
    // Uniform Management
    // ========================================================================

    /// Set an integer uniform
    void setInt(const std::string& uniformName, int value) {
        m_uniforms[uniformName] = value;
    }

    /// Set a float uniform
    void setFloat(const std::string& uniformName, float value) {
        m_uniforms[uniformName] = value;
    }

    /// Set a vec2 uniform
    void setVec2(const std::string& uniformName, const glm::vec2& value) {
        m_uniforms[uniformName] = value;
    }

    /// Set a vec3 uniform
    void setVec3(const std::string& uniformName, const glm::vec3& value) {
        m_uniforms[uniformName] = value;
    }

    /// Set a vec4 uniform
    void setVec4(const std::string& uniformName, const glm::vec4& value) {
        m_uniforms[uniformName] = value;
    }

    /// Set a mat3 uniform
    void setMat3(const std::string& uniformName, const glm::mat3& value) {
        m_uniforms[uniformName] = value;
    }

    /// Set a mat4 uniform
    void setMat4(const std::string& uniformName, const glm::mat4& value) {
        m_uniforms[uniformName] = value;
    }

    /// Get the shader
    Shader* getShader() const { return m_shader; }

protected:
    void uploadUniform(const std::string& uniformName, const UniformValue& value) {
        if (!m_shader) return;

        std::visit([this, &uniformName](auto&& arg) {
            using T = std::decay_t<decltype(arg)>;
            if constexpr (std::is_same_v<T, int>) {
                m_shader->setInt(uniformName, arg);
            } else if constexpr (std::is_same_v<T, float>) {
                m_shader->setFloat(uniformName, arg);
            } else if constexpr (std::is_same_v<T, glm::vec2>) {
                m_shader->setVec2(uniformName, arg);
            } else if constexpr (std::is_same_v<T, glm::vec3>) {
                m_shader->setVec3(uniformName, arg);
            } else if constexpr (std::is_same_v<T, glm::vec4>) {
                m_shader->setVec4(uniformName, arg);
            } else if constexpr (std::is_same_v<T, glm::mat3>) {
                m_shader->setMat3(uniformName, arg);
            } else if constexpr (std::is_same_v<T, glm::mat4>) {
                m_shader->setMat4(uniformName, arg);
            }
        }, value);
    }

    Shader* m_shader = nullptr;
    UNIQUE<Shader> m_ownedShader;
    bool m_ownsShader = false;
    std::string m_vertexSource;
    std::string m_fragmentSource;
    std::unordered_map<std::string, UniformValue> m_uniforms;
};

} // namespace Pina
