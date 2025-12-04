/// Pina Engine - OpenGL Shader Implementation

#include "GLShader.h"
#include <glm/gtc/type_ptr.hpp>
#include <iostream>

namespace Pina {

GLShader::GLShader()
    : m_programID(0)
{
}

GLShader::~GLShader() {
    if (m_programID != 0) {
        glDeleteProgram(m_programID);
    }
}

bool GLShader::load(const std::string& vertexSrc, const std::string& fragmentSrc) {
    // Compile shaders
    GLuint vertexShader = compileShader(GL_VERTEX_SHADER, vertexSrc);
    if (vertexShader == 0) {
        return false;
    }

    GLuint fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentSrc);
    if (fragmentShader == 0) {
        glDeleteShader(vertexShader);
        return false;
    }

    // Create and link program
    m_programID = glCreateProgram();
    glAttachShader(m_programID, vertexShader);
    glAttachShader(m_programID, fragmentShader);
    glLinkProgram(m_programID);

    // Check for link errors
    GLint success;
    glGetProgramiv(m_programID, GL_LINK_STATUS, &success);
    if (!success) {
        GLchar infoLog[512];
        glGetProgramInfoLog(m_programID, 512, nullptr, infoLog);
        std::cerr << "Shader program link error: " << infoLog << std::endl;
        glDeleteProgram(m_programID);
        m_programID = 0;
    }

    // Clean up shaders (they're linked into the program now)
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return m_programID != 0;
}

void GLShader::bind() {
    glUseProgram(m_programID);
}

void GLShader::unbind() {
    glUseProgram(0);
}

void GLShader::setInt(const std::string& name, int value) {
    glUniform1i(getUniformLocation(name), value);
}

void GLShader::setFloat(const std::string& name, float value) {
    glUniform1f(getUniformLocation(name), value);
}

void GLShader::setVec2(const std::string& name, const glm::vec2& value) {
    glUniform2fv(getUniformLocation(name), 1, glm::value_ptr(value));
}

void GLShader::setVec3(const std::string& name, const glm::vec3& value) {
    glUniform3fv(getUniformLocation(name), 1, glm::value_ptr(value));
}

void GLShader::setVec4(const std::string& name, const glm::vec4& value) {
    glUniform4fv(getUniformLocation(name), 1, glm::value_ptr(value));
}

void GLShader::setMat3(const std::string& name, const glm::mat3& value) {
    glUniformMatrix3fv(getUniformLocation(name), 1, GL_FALSE, glm::value_ptr(value));
}

void GLShader::setMat4(const std::string& name, const glm::mat4& value) {
    glUniformMatrix4fv(getUniformLocation(name), 1, GL_FALSE, glm::value_ptr(value));
}

GLuint GLShader::compileShader(GLenum type, const std::string& source) {
    GLuint shader = glCreateShader(type);
    const char* src = source.c_str();
    glShaderSource(shader, 1, &src, nullptr);
    glCompileShader(shader);

    // Check for compile errors
    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        GLchar infoLog[512];
        glGetShaderInfoLog(shader, 512, nullptr, infoLog);
        const char* typeName = (type == GL_VERTEX_SHADER) ? "vertex" : "fragment";
        std::cerr << "Shader compile error (" << typeName << "): " << infoLog << std::endl;
        glDeleteShader(shader);
        return 0;
    }

    return shader;
}

GLint GLShader::getUniformLocation(const std::string& name) {
    auto it = m_uniformLocationCache.find(name);
    if (it != m_uniformLocationCache.end()) {
        return it->second;
    }

    GLint location = glGetUniformLocation(m_programID, name.c_str());
    m_uniformLocationCache[name] = location;
    return location;
}

} // namespace Pina
