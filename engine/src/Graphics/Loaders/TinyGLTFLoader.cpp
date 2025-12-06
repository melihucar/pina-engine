/// Pina Engine - TinyGLTF Model Loader Implementation

#define TINYGLTF_IMPLEMENTATION
#define TINYGLTF_NO_STB_IMAGE
#define TINYGLTF_NO_STB_IMAGE_WRITE
#include <tiny_gltf.h>

// Use STB for image loading (already linked via stb library)
#include <stb_image.h>

#include "TinyGLTFLoader.h"
#include "../Primitives/StaticMesh.h"
#include "../Texture.h"
#include "../Material.h"
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/quaternion.hpp>

namespace Pina {

// Custom image loading callback for TinyGLTF using STB
static bool LoadImageData(tinygltf::Image* image, const int image_idx,
                          std::string* err, std::string* warn,
                          int req_width, int req_height,
                          const unsigned char* bytes, int size, void* user_data) {
    (void)image_idx;
    (void)warn;
    (void)req_width;
    (void)req_height;
    (void)user_data;

    int width, height, channels;
    unsigned char* data = stbi_load_from_memory(bytes, size, &width, &height, &channels, 0);

    if (!data) {
        if (err) {
            *err = "Failed to load image with stb_image";
        }
        return false;
    }

    image->width = width;
    image->height = height;
    image->component = channels;
    image->bits = 8;
    image->pixel_type = TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE;
    image->image.resize(static_cast<size_t>(width * height * channels));
    std::copy(data, data + width * height * channels, image->image.begin());

    stbi_image_free(data);
    return true;
}

// ============================================================================
// Node Transform Helper
// ============================================================================

glm::mat4 TinyGLTFLoader::getNodeLocalTransform(const tinygltf::Node& node) {
    // If node has a matrix, use it directly
    // Note: TinyGLTF stores matrix as std::vector<double>, need to convert to float
    if (node.matrix.size() == 16) {
        // glTF matrices are column-major, same as GLM
        glm::mat4 mat;
        for (int i = 0; i < 16; ++i) {
            glm::value_ptr(mat)[i] = static_cast<float>(node.matrix[i]);
        }
        return mat;
    }

    // Otherwise, compose from TRS
    glm::mat4 transform(1.0f);

    // Translation
    if (node.translation.size() == 3) {
        transform = glm::translate(transform, glm::vec3(
            node.translation[0],
            node.translation[1],
            node.translation[2]
        ));
    }

    // Rotation (quaternion: x, y, z, w)
    if (node.rotation.size() == 4) {
        glm::quat q(
            static_cast<float>(node.rotation[3]),  // w
            static_cast<float>(node.rotation[0]),  // x
            static_cast<float>(node.rotation[1]),  // y
            static_cast<float>(node.rotation[2])   // z
        );
        transform = transform * glm::mat4_cast(q);
    }

    // Scale
    if (node.scale.size() == 3) {
        transform = glm::scale(transform, glm::vec3(
            node.scale[0],
            node.scale[1],
            node.scale[2]
        ));
    }

    return transform;
}

// Use TinyGLTFLoadContext from header

// ============================================================================
// Process a single primitive with transform applied
// ============================================================================

UNIQUE<StaticMesh> TinyGLTFLoader::processPrimitive(
    const tinygltf::Model& gltfModel,
    const tinygltf::Primitive& primitive,
    const glm::mat4& transform,
    TinyGLTFLoadContext& ctx,
    size_t /*meshIdx*/,
    size_t /*primIdx*/)
{
    // Get accessors
    int posAccessor = -1, normAccessor = -1, texAccessor = -1, idxAccessor = -1;

    auto posIt = primitive.attributes.find("POSITION");
    if (posIt != primitive.attributes.end()) posAccessor = posIt->second;

    auto normIt = primitive.attributes.find("NORMAL");
    if (normIt != primitive.attributes.end()) normAccessor = normIt->second;

    auto texIt = primitive.attributes.find("TEXCOORD_0");
    if (texIt != primitive.attributes.end()) texAccessor = texIt->second;

    idxAccessor = primitive.indices;

    if (posAccessor < 0) {
        std::cerr << "    Primitive: No position data, skipping" << std::endl;
        return nullptr;
    }

    const auto& posAcc = gltfModel.accessors[posAccessor];
    size_t vertexCount = posAcc.count;

    // Get buffer pointers and strides
    const auto& posBufView = gltfModel.bufferViews[posAcc.bufferView];
    const auto& posBuf = gltfModel.buffers[posBufView.buffer];
    const uint8_t* posData = &posBuf.data[posBufView.byteOffset + posAcc.byteOffset];
    size_t posStride = posBufView.byteStride > 0 ? posBufView.byteStride : 12; // 3 floats

    const uint8_t* normData = nullptr;
    size_t normStride = 12;
    if (normAccessor >= 0) {
        const auto& normAcc = gltfModel.accessors[normAccessor];
        const auto& normBufView = gltfModel.bufferViews[normAcc.bufferView];
        const auto& normBuf = gltfModel.buffers[normBufView.buffer];
        normData = &normBuf.data[normBufView.byteOffset + normAcc.byteOffset];
        normStride = normBufView.byteStride > 0 ? normBufView.byteStride : 12;
    }

    const uint8_t* texData = nullptr;
    size_t texStride = 8;
    if (texAccessor >= 0) {
        const auto& texAcc = gltfModel.accessors[texAccessor];
        const auto& texBufView = gltfModel.bufferViews[texAcc.bufferView];
        const auto& texBuf = gltfModel.buffers[texBufView.buffer];
        texData = &texBuf.data[texBufView.byteOffset + texAcc.byteOffset];
        texStride = texBufView.byteStride > 0 ? texBufView.byteStride : 8;
    }

    // Calculate normal matrix for transforming normals
    glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(transform)));

    // Build interleaved vertex data with transform applied
    std::vector<float> vertices;
    vertices.reserve(vertexCount * 8);

    for (size_t i = 0; i < vertexCount; ++i) {
        // Read local position
        const float* pos = reinterpret_cast<const float*>(posData + i * posStride);
        glm::vec4 localPos(pos[0], pos[1], pos[2], 1.0f);

        // Apply transform
        glm::vec4 worldPos = transform * localPos;

        vertices.push_back(worldPos.x);
        vertices.push_back(worldPos.y);
        vertices.push_back(worldPos.z);

        ctx.model->m_boundingBox.expand(glm::vec3(worldPos));

        // Normal - apply normal matrix
        if (normData) {
            const float* norm = reinterpret_cast<const float*>(normData + i * normStride);
            glm::vec3 localNorm(norm[0], norm[1], norm[2]);
            glm::vec3 worldNorm = glm::normalize(normalMatrix * localNorm);
            vertices.push_back(worldNorm.x);
            vertices.push_back(worldNorm.y);
            vertices.push_back(worldNorm.z);
        } else {
            vertices.push_back(0.0f);
            vertices.push_back(1.0f);
            vertices.push_back(0.0f);
        }

        // TexCoord (unchanged by transform)
        if (texData) {
            const float* tex = reinterpret_cast<const float*>(texData + i * texStride);
            vertices.push_back(tex[0]);
            vertices.push_back(tex[1]);
        } else {
            vertices.push_back(0.0f);
            vertices.push_back(0.0f);
        }
    }

    // Process indices
    std::vector<uint32_t> indices;
    if (idxAccessor >= 0) {
        const auto& idxAcc = gltfModel.accessors[idxAccessor];
        const auto& idxBufView = gltfModel.bufferViews[idxAcc.bufferView];
        const auto& idxBuf = gltfModel.buffers[idxBufView.buffer];
        const uint8_t* idxData = &idxBuf.data[idxBufView.byteOffset + idxAcc.byteOffset];

        indices.reserve(idxAcc.count);

        // Determine element size based on component type
        size_t elementSize = 4; // default uint32
        switch (idxAcc.componentType) {
            case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT: elementSize = 4; break;
            case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT: elementSize = 2; break;
            case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE: elementSize = 1; break;
        }

        // Use byte stride if specified, otherwise use element size
        size_t idxStride = idxBufView.byteStride > 0 ? idxBufView.byteStride : elementSize;

        for (size_t i = 0; i < idxAcc.count; ++i) {
            uint32_t idx = 0;
            const uint8_t* ptr = idxData + i * idxStride;
            switch (idxAcc.componentType) {
                case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT:
                    idx = *reinterpret_cast<const uint32_t*>(ptr);
                    break;
                case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT:
                    idx = *reinterpret_cast<const uint16_t*>(ptr);
                    break;
                case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE:
                    idx = *ptr;
                    break;
            }
            indices.push_back(idx);
        }
    } else {
        // Non-indexed: generate sequential indices
        for (uint32_t i = 0; i < vertexCount; ++i) {
            indices.push_back(i);
        }
    }


    return StaticMesh::create(ctx.device, vertices, indices);
}

// ============================================================================
// Recursive Node Processing
// ============================================================================

void TinyGLTFLoader::processNode(
    const tinygltf::Model& gltfModel,
    int nodeIdx,
    const glm::mat4& parentTransform,
    TinyGLTFLoadContext& ctx)
{
    const tinygltf::Node& node = gltfModel.nodes[nodeIdx];

    // Calculate this node's world transform
    glm::mat4 localTransform = getNodeLocalTransform(node);
    glm::mat4 worldTransform = parentTransform * localTransform;


    // If this node has a mesh, process it
    if (node.mesh >= 0 && node.mesh < static_cast<int>(gltfModel.meshes.size())) {
        const auto& mesh = gltfModel.meshes[node.mesh];

        for (size_t primIdx = 0; primIdx < mesh.primitives.size(); ++primIdx) {
            const auto& primitive = mesh.primitives[primIdx];

            // Only process triangles
            if (primitive.mode != TINYGLTF_MODE_TRIANGLES && primitive.mode != -1) {
                std::cerr << "    WARNING: Skipping non-triangle primitive (mode=" << primitive.mode << ")" << std::endl;
                continue;
            }

            auto staticMesh = processPrimitive(gltfModel, primitive, worldTransform, ctx, node.mesh, primIdx);
            if (staticMesh) {
                ctx.model->m_meshes.push_back(std::move(staticMesh));
                ctx.model->m_meshMaterialIndices.push_back(0);
            }
        }
    }

    // Recursively process children
    for (int childIdx : node.children) {
        if (childIdx >= 0 && childIdx < static_cast<int>(gltfModel.nodes.size())) {
            processNode(gltfModel, childIdx, worldTransform, ctx);
        }
    }
}

// ============================================================================
// Main Load Function
// ============================================================================

UNIQUE<Model> TinyGLTFLoader::load(GraphicsDevice* device, const std::string& path) {
    std::cout << "TinyGLTF: Loading " << path << std::endl;

    tinygltf::Model gltfModel;
    tinygltf::TinyGLTF loader;
    std::string err, warn;

    // Set custom image loading callback
    loader.SetImageLoader(LoadImageData, nullptr);

    bool success = false;
    if (path.find(".glb") != std::string::npos) {
        success = loader.LoadBinaryFromFile(&gltfModel, &err, &warn, path);
    } else {
        success = loader.LoadASCIIFromFile(&gltfModel, &err, &warn, path);
    }

    if (!warn.empty()) {
        std::cout << "TinyGLTF Warning: " << warn << std::endl;
    }

    if (!success) {
        std::cerr << "TinyGLTF Error: " << err << std::endl;
        return nullptr;
    }

    auto model = UNIQUE<Model>(new Model());
    model->m_path = path;

    // Extract directory
    size_t lastSlash = path.find_last_of("/\\");
    model->m_directory = (lastSlash != std::string::npos) ? path.substr(0, lastSlash) : ".";

    // Add default material
    model->m_materials.push_back(Material::createDefault());

    // Setup loading context
    TinyGLTFLoadContext ctx;
    ctx.device = device;
    ctx.model = model.get();
    ctx.gltfModel = &gltfModel;

    std::cout << "TinyGLTF: " << gltfModel.nodes.size() << " nodes, "
              << gltfModel.meshes.size() << " meshes" << std::endl;

    // Process the default scene (or scene 0 if no default)
    int sceneIdx = gltfModel.defaultScene >= 0 ? gltfModel.defaultScene : 0;

    if (sceneIdx < static_cast<int>(gltfModel.scenes.size())) {
        const auto& scene = gltfModel.scenes[sceneIdx];

        // Start with identity transform for root nodes
        glm::mat4 rootTransform(1.0f);

        // Process each root node in the scene
        for (int rootNodeIdx : scene.nodes) {
            if (rootNodeIdx >= 0 && rootNodeIdx < static_cast<int>(gltfModel.nodes.size())) {
                processNode(gltfModel, rootNodeIdx, rootTransform, ctx);
            }
        }
    } else {
        std::cerr << "TinyGLTF: No valid scene found" << std::endl;
    }

    std::cout << "TinyGLTF: Loaded " << model->m_meshes.size() << " meshes" << std::endl;
    std::cout << "  Bounds: " << model->getSize().x << " x " << model->getSize().y << " x " << model->getSize().z << std::endl;

    return model;
}

} // namespace Pina
