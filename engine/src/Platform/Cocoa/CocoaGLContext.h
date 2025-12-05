#pragma once

#include "../Graphics.h"

#ifdef __OBJC__
@class NSOpenGLContext;
@class NSOpenGLPixelFormat;
#else
typedef void NSOpenGLContext;
typedef void NSOpenGLPixelFormat;
#endif

namespace Pina {

/// macOS OpenGL context implementation using NSOpenGLContext
class CocoaGLContext : public Graphics {
public:
    CocoaGLContext();
    ~CocoaGLContext() override;

    bool create(Window* window, const GraphicsConfig& config) override;
    void destroy() override;
    void makeCurrent() override;
    void swapBuffers() override;
    void setVSync(bool enabled) override;
    void updateContext() override;
    GraphicsBackend getBackend() const override;

private:
    NSOpenGLContext* m_context = nullptr;
    NSOpenGLPixelFormat* m_pixelFormat = nullptr;
    Window* m_window = nullptr;
};

} // namespace Pina
