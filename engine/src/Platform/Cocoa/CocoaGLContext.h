#pragma once

#include "../GraphicsContext.h"

#ifdef __OBJC__
@class NSOpenGLContext;
@class NSOpenGLPixelFormat;
#else
typedef void NSOpenGLContext;
typedef void NSOpenGLPixelFormat;
#endif

namespace Pina {

/// macOS OpenGL context implementation using NSOpenGLContext
class CocoaGLContext : public GraphicsContext {
public:
    CocoaGLContext();
    ~CocoaGLContext() override;

    bool create(Window* window, const ContextConfig& config) override;
    void destroy() override;
    void makeCurrent() override;
    void swapBuffers() override;
    void setVSync(bool enabled) override;
    void update() override;
    GraphicsBackend getBackend() const override;

private:
    NSOpenGLContext* m_context = nullptr;
    NSOpenGLPixelFormat* m_pixelFormat = nullptr;
    Window* m_window = nullptr;
};

} // namespace Pina
