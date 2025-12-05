/// Window Tests
/// Tests for Platform/Window functionality

#include <gtest/gtest.h>
#include <Pina.h>

namespace Pina {
namespace Tests {

// Test WindowConfig defaults
TEST(WindowConfigTest, DefaultValues) {
    WindowConfig config;

    EXPECT_EQ(config.width, 1280);
    EXPECT_EQ(config.height, 720);
    EXPECT_EQ(config.title, "Pina Window");
    EXPECT_FALSE(config.fullscreen);
    EXPECT_FALSE(config.resizable);
}

// Test WindowConfig custom values
TEST(WindowConfigTest, CustomValues) {
    WindowConfig config;
    config.width = 1920;
    config.height = 1080;
    config.title = "Custom Window";
    config.fullscreen = true;
    config.resizable = true;

    EXPECT_EQ(config.width, 1920);
    EXPECT_EQ(config.height, 1080);
    EXPECT_EQ(config.title, "Custom Window");
    EXPECT_TRUE(config.fullscreen);
    EXPECT_TRUE(config.resizable);
}

// Test Window factory creates platform-specific implementation
TEST(WindowTest, FactoryCreation) {
    Window* window = Window::createDefault();
    ASSERT_NE(window, nullptr);

    // Window should not be created until create() is called
    EXPECT_EQ(window->getNativeHandle(), nullptr);

    delete window;
}

// Note: Actual window creation tests require a display server
// These will be integration tests that run in a CI environment with display

} // namespace Tests
} // namespace Pina
