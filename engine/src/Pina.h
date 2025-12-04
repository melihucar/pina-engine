#pragma once

/// Pina Engine - Main Include Header
/// Include this single header to access the entire engine API

// Version
#define PINA_VERSION_MAJOR 0
#define PINA_VERSION_MINOR 1
#define PINA_VERSION_PATCH 0

// Core
#include "Core/Memory.h"
#include "Core/Application.h"

// Platform
#include "Platform/Window.h"
#include "Platform/GraphicsContext.h"

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

// Math (TODO)
// #include "Math/Vector2.h"
// #include "Math/Vector3.h"
// #include "Math/Matrix4.h"

// IO (TODO)
// #include "IO/Log.h"
// #include "IO/File.h"

// Scene (TODO)
// #include "Scene/Scene.h"
// #include "Scene/Node.h"

// Resource (TODO)
// #include "Resource/ResourceCache.h"
