cmake_minimum_required(VERSION 3.20)
project(PinaEngine VERSION 0.1.0 LANGUAGES CXX C OBJCXX)

# C++ Standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Build options
option(PINA_BUILD_EDITOR "Build the Pina Editor" ON)
option(PINA_BUILD_SAMPLES "Build sample projects" ON)
option(PINA_BUILD_RUNTIME "Build the runtime (hot-reload host)" ON)
option(PINA_BUILD_TESTS "Build unit tests" ON)
option(PINA_DEV_MODE "Development mode with hot-reload support" ON)

# Output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Platform detection
if(APPLE AND NOT IOS)
    set(PINA_PLATFORM "macos")
    set(PINA_GRAPHICS_BACKENDS "opengl")
    message(STATUS "Platform: macOS")
    message(STATUS "Graphics backends: ${PINA_GRAPHICS_BACKENDS}")
elseif(WIN32)
    set(PINA_PLATFORM "windows")
    set(PINA_GRAPHICS_BACKENDS "opengl")
    message(STATUS "Platform: Windows")
elseif(UNIX)
    set(PINA_PLATFORM "linux")
    set(PINA_GRAPHICS_BACKENDS "opengl")
    message(STATUS "Platform: Linux")
endif()

# Dependencies
include(cmake/Dependencies.cmake)

# Engine library
add_subdirectory(engine)

# Runtime (hot-reload host)
if(PINA_BUILD_RUNTIME)
    add_subdirectory(runtime)
endif()

# Editor
if(PINA_BUILD_EDITOR)
    add_subdirectory(editor)
endif()

# Samples
if(PINA_BUILD_SAMPLES)
    add_subdirectory(samples)
endif()

# Tests
if(PINA_BUILD_TESTS)
    add_subdirectory(tests)
endif()

# Print configuration summary
message(STATUS "")
message(STATUS "=== Pina Engine Configuration ===")
message(STATUS "Version: ${PROJECT_VERSION}")
message(STATUS "Platform: ${PINA_PLATFORM}")
message(STATUS "Dev Mode: ${PINA_DEV_MODE}")
message(STATUS "Build Editor: ${PINA_BUILD_EDITOR}")
message(STATUS "Build Samples: ${PINA_BUILD_SAMPLES}")
message(STATUS "Build Runtime: ${PINA_BUILD_RUNTIME}")
message(STATUS "Build Tests: ${PINA_BUILD_TESTS}")
message(STATUS "=================================")
message(STATUS "")
