Scaffold a new sample application for Pina Engine.

## Arguments
- `$ARGUMENTS`: sample-name (required, lowercase with hyphens)

## Instructions

1. Parse the sample name from arguments
2. Create directory: `samples/[name]/`
3. Create `samples/[name]/main.cpp` using the template below
4. Create `samples/[name]/CMakeLists.txt`
5. Add `add_subdirectory([name])` to `samples/CMakeLists.txt` if not already present

### main.cpp Template
```cpp
/// [Name] Sample
/// [Brief description]

#include <Pina.h>
#include <iostream>

class [PascalName]Sample : public Pina::Application {
public:
    [PascalName]Sample() {
        m_config.title = "Pina Engine - [Name] Sample";
        m_config.windowWidth = 1280;
        m_config.windowHeight = 720;
        m_config.vsync = true;
        m_config.resizable = true;
    }

protected:
    void onInit() override {
        m_device = Pina::GraphicsDevice::create(Pina::GraphicsBackend::OpenGL);
        m_device->setDepthTest(true);

        std::cout << "=== [Name] Sample ===" << std::endl;
        std::cout << "Escape - Quit" << std::endl;
    }

    void onUpdate(float deltaTime) override {
        auto* input = getInput();
        if (!input) return;

        if (input->isKeyPressed(Pina::Key::Escape)) {
            quit();
        }
    }

    void onRender() override {
        m_device->beginFrame();
        m_device->clear(0.1f, 0.1f, 0.12f);

        // TODO: Add rendering code

        m_device->endFrame();
    }

    void onShutdown() override {
        m_device.reset();
    }

private:
    Pina::UNIQUE<Pina::GraphicsDevice> m_device;
};

PINA_APPLICATION([PascalName]Sample)
```

### CMakeLists.txt Template
```cmake
pina_sample([name])
```

6. Convert sample-name to PascalCase for class name (e.g., "my-sample" -> "MySample")
7. Report created files and remind to rebuild with `./build-macos.sh`
