/// GraphicsContext Tests
/// Tests for Platform/GraphicsContext functionality

#include <gtest/gtest.h>
#include <Pina.h>

namespace Pina {
namespace Tests {

// Test ContextConfig defaults
TEST(ContextConfigTest, DefaultValues) {
    ContextConfig config;

    EXPECT_EQ(config.majorVersion, 4);
    EXPECT_EQ(config.minorVersion, 1);
    EXPECT_TRUE(config.coreProfile);
    EXPECT_TRUE(config.vsync);
    EXPECT_EQ(config.samples, 0);
    EXPECT_EQ(config.depthBits, 24);
    EXPECT_EQ(config.stencilBits, 8);
}

// Test ContextConfig custom values
TEST(ContextConfigTest, CustomValues) {
    ContextConfig config;
    config.majorVersion = 3;
    config.minorVersion = 3;
    config.coreProfile = false;
    config.vsync = false;
    config.samples = 4;
    config.depthBits = 32;
    config.stencilBits = 0;

    EXPECT_EQ(config.majorVersion, 3);
    EXPECT_EQ(config.minorVersion, 3);
    EXPECT_FALSE(config.coreProfile);
    EXPECT_FALSE(config.vsync);
    EXPECT_EQ(config.samples, 4);
    EXPECT_EQ(config.depthBits, 32);
    EXPECT_EQ(config.stencilBits, 0);
}

// Test GraphicsContext factory creates OpenGL context by default
TEST(GraphicsContextTest, FactoryCreatesOpenGL) {
    GraphicsContext* context = GraphicsContext::create(GraphicsBackend::OpenGL);
    ASSERT_NE(context, nullptr);

    EXPECT_EQ(context->getBackend(), GraphicsBackend::OpenGL);

    delete context;
}

// Test GraphicsBackend enum values
TEST(GraphicsBackendTest, EnumValues) {
    EXPECT_NE(static_cast<int>(GraphicsBackend::OpenGL),
              static_cast<int>(GraphicsBackend::Metal));
    EXPECT_NE(static_cast<int>(GraphicsBackend::OpenGL),
              static_cast<int>(GraphicsBackend::Vulkan));
    EXPECT_NE(static_cast<int>(GraphicsBackend::OpenGL),
              static_cast<int>(GraphicsBackend::DirectX12));
}

// Note: Actual context creation tests require a window
// These will be integration tests

} // namespace Tests
} // namespace Pina
