#pragma once

/// Pina Engine - Vertex Layout
/// Defines vertex attribute structure for shaders

#include "../Core/Export.h"
#include <vector>
#include <string>
#include <cstdint>

namespace Pina {

/// Shader data types for vertex attributes
enum class ShaderDataType {
    None = 0,
    Float, Float2, Float3, Float4,
    Int, Int2, Int3, Int4,
    Mat3, Mat4,
    Bool
};

/// Get the size in bytes of a shader data type
inline uint32_t ShaderDataTypeSize(ShaderDataType type) {
    switch (type) {
        case ShaderDataType::Float:  return 4;
        case ShaderDataType::Float2: return 4 * 2;
        case ShaderDataType::Float3: return 4 * 3;
        case ShaderDataType::Float4: return 4 * 4;
        case ShaderDataType::Int:    return 4;
        case ShaderDataType::Int2:   return 4 * 2;
        case ShaderDataType::Int3:   return 4 * 3;
        case ShaderDataType::Int4:   return 4 * 4;
        case ShaderDataType::Mat3:   return 4 * 3 * 3;
        case ShaderDataType::Mat4:   return 4 * 4 * 4;
        case ShaderDataType::Bool:   return 1;
        case ShaderDataType::None:   return 0;
    }
    return 0;
}

/// Get the component count of a shader data type
inline uint32_t ShaderDataTypeComponentCount(ShaderDataType type) {
    switch (type) {
        case ShaderDataType::Float:  return 1;
        case ShaderDataType::Float2: return 2;
        case ShaderDataType::Float3: return 3;
        case ShaderDataType::Float4: return 4;
        case ShaderDataType::Int:    return 1;
        case ShaderDataType::Int2:   return 2;
        case ShaderDataType::Int3:   return 3;
        case ShaderDataType::Int4:   return 4;
        case ShaderDataType::Mat3:   return 3 * 3;
        case ShaderDataType::Mat4:   return 4 * 4;
        case ShaderDataType::Bool:   return 1;
        case ShaderDataType::None:   return 0;
    }
    return 0;
}

/// A single vertex attribute
struct PINA_API VertexAttribute {
    std::string name;
    ShaderDataType type;
    uint32_t size;
    uint32_t offset;
    bool normalized;

    VertexAttribute() = default;

    VertexAttribute(const std::string& name, ShaderDataType type, bool normalized = false)
        : name(name)
        , type(type)
        , size(ShaderDataTypeSize(type))
        , offset(0)
        , normalized(normalized)
    {}
};

/// Describes the layout of vertex data
class PINA_API VertexLayout {
public:
    VertexLayout() = default;

    VertexLayout(std::initializer_list<VertexAttribute> attributes)
        : m_attributes(attributes)
    {
        calculateOffsetsAndStride();
    }

    /// Add an attribute to the layout
    void push(const std::string& name, ShaderDataType type, bool normalized = false) {
        m_attributes.emplace_back(name, type, normalized);
        calculateOffsetsAndStride();
    }

    /// Get the stride (total size of one vertex)
    uint32_t getStride() const { return m_stride; }

    /// Get the attributes
    const std::vector<VertexAttribute>& getAttributes() const { return m_attributes; }

    /// Iterator support
    std::vector<VertexAttribute>::iterator begin() { return m_attributes.begin(); }
    std::vector<VertexAttribute>::iterator end() { return m_attributes.end(); }
    std::vector<VertexAttribute>::const_iterator begin() const { return m_attributes.begin(); }
    std::vector<VertexAttribute>::const_iterator end() const { return m_attributes.end(); }

private:
    void calculateOffsetsAndStride() {
        uint32_t offset = 0;
        m_stride = 0;
        for (auto& attr : m_attributes) {
            attr.offset = offset;
            offset += attr.size;
            m_stride += attr.size;
        }
    }

    std::vector<VertexAttribute> m_attributes;
    uint32_t m_stride = 0;
};

} // namespace Pina
