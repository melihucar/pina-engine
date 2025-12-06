#pragma once

/// Pina Engine - Main Include Header
/// Include this single header to access the entire engine API

// Version
#define PINA_VERSION_MAJOR 0
#define PINA_VERSION_MINOR 1
#define PINA_VERSION_PATCH 0

// Core
#include "Core/Memory.h"
#include "Core/Subsystem.h"
#include "Core/Context.h"
#include "Core/Application.h"
#include "Core/Event.h"
#include "Core/EventDispatcher.h"

// Platform
#include "Platform/Window.h"
#include "Platform/Graphics.h"

// Input
#include "Input/KeyCodes.h"
#include "Input/Input.h"
#include "Input/InputEvents.h"

// Graphics
#include "Graphics/GraphicsDevice.h"
#include "Graphics/Shader.h"
#include "Graphics/Buffer.h"
#include "Graphics/VertexLayout.h"
#include "Graphics/Mesh.h"
#include "Graphics/Primitives/TriangleMesh.h"
#include "Graphics/Primitives/QuadMesh.h"
#include "Graphics/Primitives/CubeMesh.h"
#include "Graphics/Camera.h"
#include "Graphics/OrbitCamera.h"
#include "Graphics/FreelookCamera.h"
#include "Graphics/Material.h"
#include "Graphics/Texture.h"
#include "Graphics/Model.h"
#include "Graphics/Primitives/StaticMesh.h"

// Lighting
#include "Graphics/Lighting/Light.h"
#include "Graphics/Lighting/DirectionalLight.h"
#include "Graphics/Lighting/PointLight.h"
#include "Graphics/Lighting/SpotLight.h"
#include "Graphics/Lighting/LightManager.h"

// Shaders
#include "Graphics/Shaders/ShaderLibrary.h"

// Math
#include "Math/Vector2.h"
#include "Math/Vector3.h"
#include "Math/Vector4.h"
#include "Math/Color.h"
#include "Math/Mathf.h"
#include "Math/Ray.h"
#include "Math/Plane.h"
#include "Math/Geometry.h"

// UI
#include "UI/UI.h"
#include "UI/UITypes.h"
#include "UI/UIWidgets.h"

// IO (TODO)
// #include "IO/Log.h"
// #include "IO/File.h"

// Scene
#include "Scene/Transform.h"
#include "Scene/Node.h"
#include "Scene/Scene.h"
#include "Scene/SceneRenderer.h"

// Resource (TODO)
// #include "Resource/ResourceCache.h"
