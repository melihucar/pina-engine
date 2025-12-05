#import "CocoaGLContext.h"
#import "../Window.h"
#import <Cocoa/Cocoa.h>
#import <OpenGL/gl3.h>

namespace Pina {

CocoaGLContext::CocoaGLContext() = default;

CocoaGLContext::~CocoaGLContext() {
    destroy();
}

bool CocoaGLContext::create(Window* window, const GraphicsConfig& config) {
    @autoreleasepool {
        m_window = window;

        // Build pixel format attributes
        std::vector<NSOpenGLPixelFormatAttribute> attrs;

        // OpenGL version (4.1 core is max on macOS)
        attrs.push_back(NSOpenGLPFAOpenGLProfile);
        if (config.coreProfile && config.majorVersion >= 3) {
            attrs.push_back(NSOpenGLProfileVersion3_2Core);  // 3.2+ core profile
        } else {
            attrs.push_back(NSOpenGLProfileVersionLegacy);
        }

        // Color buffer
        attrs.push_back(NSOpenGLPFAColorSize);
        attrs.push_back(24);

        // Alpha buffer
        attrs.push_back(NSOpenGLPFAAlphaSize);
        attrs.push_back(8);

        // Depth buffer
        attrs.push_back(NSOpenGLPFADepthSize);
        attrs.push_back(24);

        // Stencil buffer
        attrs.push_back(NSOpenGLPFAStencilSize);
        attrs.push_back(8);

        // Double buffering
        attrs.push_back(NSOpenGLPFADoubleBuffer);

        // Accelerated
        attrs.push_back(NSOpenGLPFAAccelerated);

        // Multisampling
        if (config.samples > 0) {
            attrs.push_back(NSOpenGLPFAMultisample);
            attrs.push_back(NSOpenGLPFASampleBuffers);
            attrs.push_back(1);
            attrs.push_back(NSOpenGLPFASamples);
            attrs.push_back(config.samples);
        }

        // Terminate attributes list
        attrs.push_back(0);

        // Create pixel format
        m_pixelFormat = [[NSOpenGLPixelFormat alloc] initWithAttributes:attrs.data()];
        if (!m_pixelFormat) {
            NSLog(@"Failed to create OpenGL pixel format");
            return false;
        }

        // Create OpenGL context
        m_context = [[NSOpenGLContext alloc] initWithFormat:m_pixelFormat shareContext:nil];
        if (!m_context) {
            NSLog(@"Failed to create OpenGL context");
            return false;
        }

        // Set the view for rendering
        NSView* view = (__bridge NSView*)window->getNativeView();
        [m_context setView:view];

        // Make current and configure
        [m_context makeCurrentContext];

        // Set vsync
        setVSync(config.vsync);

        // Log OpenGL info
        const char* vendor = (const char*)glGetString(GL_VENDOR);
        const char* renderer = (const char*)glGetString(GL_RENDERER);
        const char* version = (const char*)glGetString(GL_VERSION);
        NSLog(@"OpenGL Vendor: %s", vendor);
        NSLog(@"OpenGL Renderer: %s", renderer);
        NSLog(@"OpenGL Version: %s", version);

        return true;
    }
}

void CocoaGLContext::destroy() {
    @autoreleasepool {
        if (m_context) {
            [NSOpenGLContext clearCurrentContext];
            m_context = nil;
        }
        if (m_pixelFormat) {
            m_pixelFormat = nil;
        }
        m_window = nullptr;
    }
}

void CocoaGLContext::makeCurrent() {
    @autoreleasepool {
        if (m_context) {
            [m_context makeCurrentContext];
        }
    }
}

void CocoaGLContext::swapBuffers() {
    @autoreleasepool {
        if (m_context) {
            [m_context flushBuffer];
        }
    }
}

void CocoaGLContext::setVSync(bool enabled) {
    @autoreleasepool {
        if (m_context) {
            GLint sync = enabled ? 1 : 0;
            [m_context setValues:&sync forParameter:NSOpenGLContextParameterSwapInterval];
        }
    }
}

void CocoaGLContext::updateContext() {
    @autoreleasepool {
        if (m_context) {
            [m_context update];
        }
    }
}

GraphicsBackend CocoaGLContext::getBackend() const {
    return GraphicsBackend::OpenGL;
}

// Factory implementation
Graphics* Graphics::createDefault(GraphicsBackend backend) {
    switch (backend) {
        case GraphicsBackend::OpenGL:
            return new CocoaGLContext();
        case GraphicsBackend::Metal:
            // TODO: Implement CocoaMetalContext
            return nullptr;
        default:
            return nullptr;
    }
}

} // namespace Pina
