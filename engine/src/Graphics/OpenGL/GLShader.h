#pragma once

/// Pina Engine - OpenGL Shader Implementation

#include "../Shader.h"
#include "GLCommon.h"
#include <unordered_map>

namespace Pina {

class GLShader : public Shader {
public:
    GLShader();
    ~GLShader() override;

    bool load(const std::string& vertexSrc, const std::string& fragmentSrc) override;

    void bind() override;
    void unbind() override;

    void setInt(const std::string& name, int value) override;
    void setFloat(const std::string& name, float value) override;
    void setVec2(const std::string& name, const glm::vec2& value) override;
    void setVec3(const std::string& name, const glm::vec3& value) override;
    void setVec4(const std::string& name, const glm::vec4& value) override;
    void setMat3(const std::string& name, const glm::mat3& value) override;
    void setMat4(const std::string& name, const glm::mat4& value) override;

    uint32_t getID() const override { return m_programID; }

private:
    GLuint compileShader(GLenum type, const std::string& source);
    GLint getUniformLocation(const std::string& name);

    GLuint m_programID = 0;
    std::unordered_map<std::string, GLint> m_uniformLocationCache;
};

} // namespace Pina
