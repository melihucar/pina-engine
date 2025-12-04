#pragma once

#include "../Window.h"

#ifdef __OBJC__
@class NSWindow;
@class NSView;
@class PinaWindowDelegate;
#else
typedef void NSWindow;
typedef void NSView;
typedef void PinaWindowDelegate;
#endif

namespace Pina {

/// macOS Cocoa window implementation
class CocoaWindow : public Window {
public:
    CocoaWindow();
    ~CocoaWindow() override;

    bool create(const WindowConfig& config) override;
    void destroy() override;
    void pollEvents() override;
    bool shouldClose() const override;
    void* getNativeHandle() const override;
    void* getNativeView() const override;
    int getWidth() const override;
    int getHeight() const override;
    void setTitle(const std::string& title) override;

    // Called by delegate
    void onResize(int width, int height);
    void onClose();

private:
    NSWindow* m_window = nullptr;
    NSView* m_view = nullptr;
    PinaWindowDelegate* m_delegate = nullptr;
    int m_width = 0;
    int m_height = 0;
    bool m_shouldClose = false;
};

} // namespace Pina
