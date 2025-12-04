# Pina Engine Tests
# Using Google Test framework

enable_testing()

# Test executable
add_executable(pina-tests
    main.cpp
    core/ApplicationTests.cpp
    core/MemoryTests.cpp
    platform/WindowTests.cpp
    platform/GraphicsContextTests.cpp
)

target_link_libraries(pina-tests
    PRIVATE
        pina-engine
        GTest::gtest
        GTest::gtest_main
)

target_include_directories(pina-tests
    PRIVATE
        ${CMAKE_SOURCE_DIR}/engine/src
)

# Register tests with CTest
include(GoogleTest)
gtest_discover_tests(pina-tests)
