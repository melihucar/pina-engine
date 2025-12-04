#pragma once

/// Pina Engine - Shader Interface
/// Abstract interface for GPU shader programs

#include "../Core/Export.h"
#include <string>
#include <glm/glm.hpp>

namespace Pina {

/// Abstract shader interface
class PINA_API Shader {
public:
    virtual ~Shader() = default;

    /// Load shader from source strings
    virtual bool load(const std::string& vertexSrc, const std::string& fragmentSrc) = 0;

    /// Bind/unbind shader for use
    virtual void bind() = 0;
    virtual void unbind() = 0;

    /// Uniform setters
    virtual void setInt(const std::string& name, int value) = 0;
    virtual void setFloat(const std::string& name, float value) = 0;
    virtual void setVec2(const std::string& name, const glm::vec2& value) = 0;
    virtual void setVec3(const std::string& name, const glm::vec3& value) = 0;
    virtual void setVec4(const std::string& name, const glm::vec4& value) = 0;
    virtual void setMat3(const std::string& name, const glm::mat3& value) = 0;
    virtual void setMat4(const std::string& name, const glm::mat4& value) = 0;

    /// Get shader program ID (implementation-specific)
    virtual uint32_t getID() const = 0;
};

} // namespace Pina
