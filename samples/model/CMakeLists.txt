# Model Sample - Demonstrates 3D model loading

add_executable(sample-model main.cpp)

target_link_libraries(sample-model PRIVATE pina-engine)

# macOS: Link OpenGL and silence deprecation warnings
if(APPLE)
    find_library(OPENGL_LIBRARY OpenGL REQUIRED)
    target_link_libraries(sample-model PRIVATE ${OPENGL_LIBRARY})
    target_compile_definitions(sample-model PRIVATE GL_SILENCE_DEPRECATION)

    set_target_properties(sample-model PROPERTIES
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_GUI_IDENTIFIER "com.pina.sample.model"
        MACOSX_BUNDLE_BUNDLE_NAME "Model Sample"
    )
endif()

# Copy assets to build directory (recursive)
file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/assets/" DESTINATION "${CMAKE_BINARY_DIR}/bin/assets/")
