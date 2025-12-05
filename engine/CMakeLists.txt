# Pina Engine Library

set(ENGINE_NAME pina-engine)

# Collect source files
file(GLOB_RECURSE ENGINE_SOURCES
    "src/*.cpp"
    "src/*.c"
)

# Collect Objective-C++ files for macOS
if(PINA_PLATFORM STREQUAL "macos")
    file(GLOB_RECURSE ENGINE_OBJCXX_SOURCES
        "src/*.mm"
    )
    list(APPEND ENGINE_SOURCES ${ENGINE_OBJCXX_SOURCES})
endif()

# Collect header files
file(GLOB_RECURSE ENGINE_HEADERS
    "src/*.h"
    "src/*.hpp"
)

# Create library (shared for dev mode, static for release)
if(PINA_DEV_MODE)
    add_library(${ENGINE_NAME} SHARED ${ENGINE_SOURCES} ${ENGINE_HEADERS})
    target_compile_definitions(${ENGINE_NAME} PRIVATE PINA_EXPORTS)
    target_compile_definitions(${ENGINE_NAME} PUBLIC PINA_SHARED)
else()
    add_library(${ENGINE_NAME} STATIC ${ENGINE_SOURCES} ${ENGINE_HEADERS})
endif()

# Include directories
target_include_directories(${ENGINE_NAME}
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
        $<INSTALL_INTERFACE:include>
)

# Link dependencies
target_link_libraries(${ENGINE_NAME}
    PUBLIC
        glm::glm
        spdlog::spdlog
        stb
    PRIVATE
        imgui
)

# Platform-specific settings
if(PINA_PLATFORM STREQUAL "macos")
    # Find and link macOS frameworks
    find_library(COCOA_LIBRARY Cocoa REQUIRED)
    find_library(OPENGL_LIBRARY OpenGL REQUIRED)
    find_library(IOKIT_LIBRARY IOKit REQUIRED)
    find_library(COREVIDEO_LIBRARY CoreVideo REQUIRED)

    target_link_libraries(${ENGINE_NAME}
        PRIVATE
            ${COCOA_LIBRARY}
            ${OPENGL_LIBRARY}
            ${IOKIT_LIBRARY}
            ${COREVIDEO_LIBRARY}
    )

    # OpenGL deprecation warning suppression (Apple deprecated OpenGL)
    target_compile_definitions(${ENGINE_NAME} PRIVATE GL_SILENCE_DEPRECATION)
endif()

# Compiler warnings
if(MSVC)
    target_compile_options(${ENGINE_NAME} PRIVATE /W4)
else()
    target_compile_options(${ENGINE_NAME} PRIVATE -Wall -Wextra -Wpedantic)
endif()

# Export symbols for shared library
if(PINA_DEV_MODE)
    set_target_properties(${ENGINE_NAME} PROPERTIES
        CXX_VISIBILITY_PRESET hidden
        VISIBILITY_INLINES_HIDDEN YES
    )
endif()

# Copy fonts to build output (next to executables)
file(GLOB ENGINE_FONTS "${CMAKE_CURRENT_SOURCE_DIR}/resources/fonts/*.ttf")
foreach(FONT ${ENGINE_FONTS})
    get_filename_component(FONT_NAME ${FONT} NAME)
    configure_file(${FONT} ${CMAKE_BINARY_DIR}/bin/fonts/${FONT_NAME} COPYONLY)
endforeach()
