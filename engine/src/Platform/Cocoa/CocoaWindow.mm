#import "CocoaWindow.h"
#import <Cocoa/Cocoa.h>

namespace Pina {

// Forward declaration
class CocoaWindow;

} // namespace Pina

// Objective-C Window Delegate
@interface PinaWindowDelegate : NSObject <NSWindowDelegate>
@property (nonatomic, assign) Pina::CocoaWindow* pinaWindow;
@end

@implementation PinaWindowDelegate

- (void)windowWillClose:(NSNotification*)notification {
    (void)notification;
    if (self.pinaWindow) {
        self.pinaWindow->onClose();
    }
}

- (void)windowDidResize:(NSNotification*)notification {
    (void)notification;
    if (self.pinaWindow) {
        NSWindow* window = [notification object];
        NSView* view = [window contentView];
        // Use backing dimensions (pixels) for Retina displays
        NSRect backingBounds = [view convertRectToBacking:[view bounds]];
        self.pinaWindow->onResize((int)backingBounds.size.width, (int)backingBounds.size.height);
    }
}

@end

// Custom NSView for rendering
@interface PinaView : NSView
@end

@implementation PinaView

- (BOOL)acceptsFirstResponder {
    return YES;
}

- (BOOL)canBecomeKeyView {
    return YES;
}

- (BOOL)wantsBestResolutionOpenGLSurface {
    return YES;
}

@end

namespace Pina {

CocoaWindow::CocoaWindow() = default;

CocoaWindow::~CocoaWindow() {
    destroy();
}

bool CocoaWindow::create(const WindowConfig& config) {
    @autoreleasepool {
        // Initialize the application if not already done
        [NSApplication sharedApplication];
        [NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];

        // Create window style mask
        NSWindowStyleMask styleMask = NSWindowStyleMaskTitled |
                                      NSWindowStyleMaskClosable |
                                      NSWindowStyleMaskMiniaturizable;
        if (config.resizable) {
            styleMask |= NSWindowStyleMaskResizable;
        }

        // Create content rect
        NSRect contentRect = NSMakeRect(0, 0, config.width, config.height);

        // Create the window
        m_window = [[NSWindow alloc] initWithContentRect:contentRect
                                               styleMask:styleMask
                                                 backing:NSBackingStoreBuffered
                                                   defer:NO];

        if (!m_window) {
            return false;
        }

        // Create and set delegate
        m_delegate = [[PinaWindowDelegate alloc] init];
        m_delegate.pinaWindow = this;
        [m_window setDelegate:m_delegate];

        // Create custom view
        m_view = [[PinaView alloc] initWithFrame:contentRect];
        [m_window setContentView:m_view];

        // Configure window
        [m_window setTitle:[NSString stringWithUTF8String:config.title.c_str()]];
        [m_window setAcceptsMouseMovedEvents:YES];

        // Set min/max size constraints
        if (config.minWidth > 0 && config.minHeight > 0) {
            [m_window setMinSize:NSMakeSize(config.minWidth, config.minHeight)];
        }
        if (config.maxWidth > 0 && config.maxHeight > 0) {
            [m_window setMaxSize:NSMakeSize(config.maxWidth, config.maxHeight)];
        }

        // Center window before applying maximized/fullscreen
        [m_window center];

        // Show window
        [m_window makeKeyAndOrderFront:nil];
        [NSApp activateIgnoringOtherApps:YES];

        // Apply fullscreen or maximized state after window is visible
        if (config.fullscreen) {
            [m_window setCollectionBehavior:NSWindowCollectionBehaviorFullScreenPrimary];
            [m_window toggleFullScreen:nil];
        } else if (config.maximized) {
            [m_window zoom:nil];
        }

        // Store dimensions
        m_width = config.width;
        m_height = config.height;

        // Finish launching
        [NSApp finishLaunching];

        return true;
    }
}

void CocoaWindow::destroy() {
    @autoreleasepool {
        if (m_delegate) {
            m_delegate.pinaWindow = nullptr;
            m_delegate = nil;
        }
        if (m_window) {
            [m_window close];
            m_window = nil;
        }
        m_view = nil;
    }
}

void CocoaWindow::pollEvents() {
    @autoreleasepool {
        NSEvent* event;
        while ((event = [NSApp nextEventMatchingMask:NSEventMaskAny
                                           untilDate:nil
                                              inMode:NSDefaultRunLoopMode
                                             dequeue:YES])) {
            [NSApp sendEvent:event];
        }
    }
}

bool CocoaWindow::shouldClose() const {
    return m_shouldClose;
}

void* CocoaWindow::getNativeHandle() const {
    return (__bridge void*)m_window;
}

void* CocoaWindow::getNativeView() const {
    return (__bridge void*)m_view;
}

int CocoaWindow::getWidth() const {
    return m_width;
}

int CocoaWindow::getHeight() const {
    return m_height;
}

void CocoaWindow::setTitle(const std::string& title) {
    @autoreleasepool {
        [m_window setTitle:[NSString stringWithUTF8String:title.c_str()]];
    }
}

void CocoaWindow::onResize(int width, int height) {
    m_width = width;
    m_height = height;
    if (m_resizeCallback) {
        m_resizeCallback(width, height);
    }
}

void CocoaWindow::onClose() {
    m_shouldClose = true;
    if (m_closeCallback) {
        m_closeCallback();
    }
}

// Factory implementation
Window* Window::create() {
    return new CocoaWindow();
}

} // namespace Pina
