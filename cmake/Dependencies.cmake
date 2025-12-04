# Pina Engine Dependencies
# Using CMake FetchContent for all dependencies

include(FetchContent)

# glad - OpenGL loader
FetchContent_Declare(
    glad
    GIT_REPOSITORY https://github.com/Dav1dde/glad.git
    GIT_TAG        v2.0.6
)

# glm - Math library
FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG        1.0.1
)

# spdlog - Logging
FetchContent_Declare(
    spdlog
    GIT_REPOSITORY https://github.com/gabime/spdlog.git
    GIT_TAG        v1.14.1
)

# stb - Image loading (header-only)
FetchContent_Declare(
    stb
    GIT_REPOSITORY https://github.com/nothings/stb.git
    GIT_TAG        master
)

# Dear ImGui - Editor UI (docking branch)
FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG        docking
)

# Google Test - Testing framework
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG        v1.14.0
)

# Make dependencies available
message(STATUS "Fetching dependencies...")

# glm
FetchContent_MakeAvailable(glm)
message(STATUS "  - glm: OK")

# spdlog
FetchContent_MakeAvailable(spdlog)
message(STATUS "  - spdlog: OK")

# stb (header-only, just populate)
FetchContent_GetProperties(stb)
if(NOT stb_POPULATED)
    FetchContent_Populate(stb)
endif()
message(STATUS "  - stb: OK")

# imgui (needs manual setup as it's not CMake-based)
FetchContent_GetProperties(imgui)
if(NOT imgui_POPULATED)
    FetchContent_Populate(imgui)
endif()
message(STATUS "  - imgui: OK")

# glad (needs to be configured for OpenGL 4.1)
FetchContent_GetProperties(glad)
if(NOT glad_POPULATED)
    FetchContent_Populate(glad)
endif()
message(STATUS "  - glad: OK")

# googletest
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)
message(STATUS "  - googletest: OK")

# Create stb library target
add_library(stb INTERFACE)
target_include_directories(stb INTERFACE ${stb_SOURCE_DIR})

# Create imgui library target
set(IMGUI_SOURCES
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_demo.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
)
add_library(imgui STATIC ${IMGUI_SOURCES})
target_include_directories(imgui PUBLIC ${imgui_SOURCE_DIR})

message(STATUS "Dependencies configured successfully")
