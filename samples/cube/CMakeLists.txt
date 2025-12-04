# Cube Sample - 3D rotating cube with camera

add_executable(sample-cube main.cpp)

target_link_libraries(sample-cube PRIVATE pina-engine)

# macOS: Link OpenGL and silence deprecation warnings
if(APPLE)
    find_library(OPENGL_LIBRARY OpenGL REQUIRED)
    target_link_libraries(sample-cube PRIVATE ${OPENGL_LIBRARY})
    target_compile_definitions(sample-cube PRIVATE GL_SILENCE_DEPRECATION)

    set_target_properties(sample-cube PROPERTIES
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_GUI_IDENTIFIER "com.pina.sample.cube"
        MACOSX_BUNDLE_BUNDLE_NAME "Cube Sample"
    )
endif()
