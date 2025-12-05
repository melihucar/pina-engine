#import "CocoaWindow.h"
#import "CocoaInput.h"
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

- (void)windowDidBecomeKey:(NSNotification*)notification {
    (void)notification;
    if (self.pinaWindow) {
        self.pinaWindow->onFocusChange(true);
    }
}

- (void)windowDidResignKey:(NSNotification*)notification {
    (void)notification;
    if (self.pinaWindow) {
        self.pinaWindow->onFocusChange(false);
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

            // Track if we handled this event (to prevent system beep for keys)
            bool handled = false;

            // Route input events to handler
            if (m_inputHandler) {
                switch ([event type]) {
                    case NSEventTypeKeyDown:
                        m_inputHandler->processKeyDown([event keyCode]);
                        m_inputHandler->processModifiersChanged((unsigned int)[event modifierFlags]);
                        handled = true;  // Don't forward to system (prevents beep)
                        break;

                    case NSEventTypeKeyUp:
                        m_inputHandler->processKeyUp([event keyCode]);
                        m_inputHandler->processModifiersChanged((unsigned int)[event modifierFlags]);
                        handled = true;  // Don't forward to system
                        break;

                    case NSEventTypeFlagsChanged:
                        m_inputHandler->processModifiersChanged((unsigned int)[event modifierFlags]);
                        break;

                    case NSEventTypeLeftMouseDown:
                        m_inputHandler->processMouseDown(MouseButton::Left);
                        break;

                    case NSEventTypeLeftMouseUp:
                        m_inputHandler->processMouseUp(MouseButton::Left);
                        break;

                    case NSEventTypeRightMouseDown:
                        m_inputHandler->processMouseDown(MouseButton::Right);
                        break;

                    case NSEventTypeRightMouseUp:
                        m_inputHandler->processMouseUp(MouseButton::Right);
                        break;

                    case NSEventTypeOtherMouseDown:
                        if ([event buttonNumber] == 2) {
                            m_inputHandler->processMouseDown(MouseButton::Middle);
                        } else if ([event buttonNumber] == 3) {
                            m_inputHandler->processMouseDown(MouseButton::X1);
                        } else if ([event buttonNumber] == 4) {
                            m_inputHandler->processMouseDown(MouseButton::X2);
                        }
                        break;

                    case NSEventTypeOtherMouseUp:
                        if ([event buttonNumber] == 2) {
                            m_inputHandler->processMouseUp(MouseButton::Middle);
                        } else if ([event buttonNumber] == 3) {
                            m_inputHandler->processMouseUp(MouseButton::X1);
                        } else if ([event buttonNumber] == 4) {
                            m_inputHandler->processMouseUp(MouseButton::X2);
                        }
                        break;

                    case NSEventTypeMouseMoved:
                    case NSEventTypeLeftMouseDragged:
                    case NSEventTypeRightMouseDragged:
                    case NSEventTypeOtherMouseDragged: {
                        NSPoint location = [event locationInWindow];
                        NSRect viewBounds = [m_view bounds];
                        // Convert from Cocoa coordinates (origin bottom-left) to top-left
                        m_inputHandler->processMouseMove(
                            static_cast<float>(location.x),
                            static_cast<float>(viewBounds.size.height - location.y)
                        );
                        break;
                    }

                    case NSEventTypeScrollWheel:
                        m_inputHandler->processScroll(
                            static_cast<float>([event scrollingDeltaX]),
                            static_cast<float>([event scrollingDeltaY])
                        );
                        break;

                    default:
                        break;
                }
            }

            // Only forward non-handled events to the system
            // (Key events cause beep if forwarded after handling)
            if (!handled) {
                [NSApp sendEvent:event];
            }
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

void CocoaWindow::onFocusChange(bool hasFocus) {
    if (m_inputHandler) {
        m_inputHandler->processFocusChange(hasFocus);
    }
}

// Factory implementation
Window* Window::createDefault() {
    return new CocoaWindow();
}

} // namespace Pina
