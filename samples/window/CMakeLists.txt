# Window Sample - Basic window creation

add_executable(sample-window main.cpp)

target_link_libraries(sample-window PRIVATE pina-engine)

# macOS: Link OpenGL and silence deprecation warnings
if(APPLE)
    find_library(OPENGL_LIBRARY OpenGL REQUIRED)
    target_link_libraries(sample-window PRIVATE ${OPENGL_LIBRARY})
    target_compile_definitions(sample-window PRIVATE GL_SILENCE_DEPRECATION)

    set_target_properties(sample-window PROPERTIES
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_GUI_IDENTIFIER "com.pina.sample.window"
        MACOSX_BUNDLE_BUNDLE_NAME "Window Sample"
    )
endif()
