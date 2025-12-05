# Pina Engine

C++17 cross-platform graphics engine with OpenGL backend.

## Quick Start

```bash
# Build (debug)
./build-macos.sh

# Build (release)
./build-macos.sh release

# Run tests
./build-macos-debug/bin/pina-tests

# Run sample
./build-macos-debug/bin/sample-cube.app/Contents/MacOS/sample-cube
```

## Architecture

```
Application (user code)
    └── Context (owns subsystems)
         ├── GraphicsContext (window, OpenGL)
         ├── InputSystem (keyboard, mouse)
         └── UISystem (ImGui integration)

Platform Abstraction:
    Window       → CocoaWindow (macOS)
    Graphics     → GLDevice (OpenGL 4.1)
```

### Key Components

| Layer | Classes | Purpose |
|-------|---------|---------|
| Core | `Application`, `Context` | Entry point, lifecycle |
| Graphics | `GraphicsDevice`, `Shader`, `Buffer`, `Texture` | GPU resources |
| Scene | `Camera`, `Model`, `Mesh`, `Material` | 3D rendering |
| Lighting | `LightManager`, `DirectionalLight`, `PointLight` | Blinn-Phong |
| UI | `UISystem`, `Widgets::*` | ImGui wrapper |
| Input | `InputSystem`, `Key`, `MouseButton` | User input |

## Coding Style

### Naming

| Element | Convention | Example |
|---------|------------|---------|
| Classes | PascalCase | `GraphicsDevice`, `LightManager` |
| Members | m_ prefix | `m_device`, `m_position` |
| Methods | camelCase | `getPosition()`, `setDevice()` |
| Getters | get/is/has | `getSize()`, `isEnabled()`, `hasTexture()` |
| Lifecycle | on* prefix | `onInit()`, `onUpdate()`, `onRender()`, `onShutdown()` |

### Headers

```cpp
#pragma once

/// Pina Engine - Component Name

#include "Dependencies.h"

namespace Pina {

class PINA_API ClassName {
public:
    // ...
};

} // namespace Pina
```

### Smart Pointers

Use engine macros instead of std types:

```cpp
// Good
UNIQUE<Shader> m_shader;
m_shader = MAKE_UNIQUE<GLShader>();

SHARED<Texture> m_texture;
m_texture = MAKE_SHARED<GLTexture>();

// Bad
std::unique_ptr<Shader> m_shader;
m_shader = std::make_unique<GLShader>();
```

### Section Headers

Use for organizing class implementations:

```cpp
// ========================================================================
// Lifecycle
// ========================================================================

void ClassName::onInit() { ... }

// ========================================================================
// Resource Creation
// ========================================================================

UNIQUE<Shader> ClassName::createShader() { ... }
```

### Virtual Methods

```cpp
class Derived : public Base {
public:
    ~Derived() override = default;
    void update() override;  // Always use override
};
```

### Error Handling

- Return `bool` for success/failure
- Return `nullptr` for factory failures
- No exceptions
- Use `std::cerr` for error messages

```cpp
bool Shader::load(const char* vertex, const char* fragment) {
    if (!compile(vertex)) {
        std::cerr << "Failed to compile vertex shader" << std::endl;
        return false;
    }
    return true;
}

UNIQUE<Model> Model::load(GraphicsDevice* device, const char* path) {
    if (!fileExists(path)) {
        return nullptr;
    }
    // ...
}
```

## Project Structure

```
pina-engine/
├── engine/
│   ├── include/          # Public headers (Pina.h)
│   └── src/
│       ├── Core/         # Application, Context, Memory
│       ├── Graphics/     # Device, Shader, Buffer, Texture
│       ├── Platform/     # Window, Cocoa, OpenGL
│       ├── Scene/        # Camera, Model, Mesh, Material
│       ├── Lighting/     # LightManager, Light types
│       └── UI/           # UISystem, Widgets
├── samples/              # Example applications
│   ├── cube/
│   ├── lighting/
│   ├── model/
│   └── ...
└── tests/                # Unit tests (Catch2)
```

## Sample Application Template

```cpp
#include <Pina.h>

class MySample : public Pina::Application {
public:
    MySample() {
        m_config.title = "My Sample";
        m_config.windowWidth = 1280;
        m_config.windowHeight = 720;
    }

protected:
    void onInit() override {
        m_device = Pina::GraphicsDevice::create(Pina::GraphicsBackend::OpenGL);
    }

    void onUpdate(float deltaTime) override {
        // Game logic
    }

    void onRender() override {
        m_device->beginFrame();
        m_device->clear(0.1f, 0.1f, 0.12f);
        // Draw calls
        m_device->endFrame();
    }

    void onShutdown() override {
        m_device.reset();
    }

private:
    Pina::UNIQUE<Pina::GraphicsDevice> m_device;
};

PINA_APPLICATION(MySample)
```
