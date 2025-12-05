# Input Sample - Demonstrates keyboard and mouse input

add_executable(sample-input main.cpp)

target_link_libraries(sample-input PRIVATE pina-engine)

# macOS: Link OpenGL and silence deprecation warnings
if(APPLE)
    find_library(OPENGL_LIBRARY OpenGL REQUIRED)
    target_link_libraries(sample-input PRIVATE ${OPENGL_LIBRARY})
    target_compile_definitions(sample-input PRIVATE GL_SILENCE_DEPRECATION)

    set_target_properties(sample-input PROPERTIES
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_GUI_IDENTIFIER "com.pina.sample.input"
        MACOSX_BUNDLE_BUNDLE_NAME "Input Sample"
    )
endif()
