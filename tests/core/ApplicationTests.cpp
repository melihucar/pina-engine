/// Application Tests
/// Tests for Core/Application functionality

#include <gtest/gtest.h>
#include <Pina.h>

namespace Pina {
namespace Tests {

// Test ApplicationConfig defaults
TEST(ApplicationConfigTest, DefaultValues) {
    ApplicationConfig config;

    EXPECT_EQ(config.title, "Pina Application");
    EXPECT_EQ(config.windowWidth, 1280);
    EXPECT_EQ(config.windowHeight, 720);
    EXPECT_TRUE(config.vsync);
    EXPECT_FALSE(config.fullscreen);
    EXPECT_FALSE(config.resizable);
}

// Test ApplicationConfig custom values
TEST(ApplicationConfigTest, CustomValues) {
    ApplicationConfig config;
    config.title = "Test App";
    config.windowWidth = 1920;
    config.windowHeight = 1080;
    config.vsync = false;
    config.fullscreen = true;
    config.resizable = true;

    EXPECT_EQ(config.title, "Test App");
    EXPECT_EQ(config.windowWidth, 1920);
    EXPECT_EQ(config.windowHeight, 1080);
    EXPECT_FALSE(config.vsync);
    EXPECT_TRUE(config.fullscreen);
    EXPECT_TRUE(config.resizable);
}

// Test Application subclass callbacks
class TestApplication : public Application {
public:
    bool initCalled = false;
    bool shutdownCalled = false;
    int updateCount = 0;
    int renderCount = 0;

protected:
    void onInit() override {
        initCalled = true;
    }

    void onShutdown() override {
        shutdownCalled = true;
    }

    void onUpdate(float deltaTime) override {
        (void)deltaTime;
        updateCount++;
    }

    void onRender() override {
        renderCount++;
    }
};

// Test that Application can be subclassed
TEST(ApplicationTest, Subclassing) {
    TestApplication app;

    EXPECT_FALSE(app.initCalled);
    EXPECT_FALSE(app.shutdownCalled);
    EXPECT_EQ(app.updateCount, 0);
    EXPECT_EQ(app.renderCount, 0);
}

// Note: Full application lifecycle tests require a window/graphics context
// which we'll add as integration tests later

} // namespace Tests
} // namespace Pina
