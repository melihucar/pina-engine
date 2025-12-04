# Triangle Sample - Basic rendering with GraphicsDevice

add_executable(sample-triangle main.cpp)

target_link_libraries(sample-triangle PRIVATE pina-engine)

# macOS: Link OpenGL and silence deprecation warnings
if(APPLE)
    find_library(OPENGL_LIBRARY OpenGL REQUIRED)
    target_link_libraries(sample-triangle PRIVATE ${OPENGL_LIBRARY})
    target_compile_definitions(sample-triangle PRIVATE GL_SILENCE_DEPRECATION)

    set_target_properties(sample-triangle PROPERTIES
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_GUI_IDENTIFIER "com.pina.sample.triangle"
        MACOSX_BUNDLE_BUNDLE_NAME "Triangle Sample"
    )
endif()
