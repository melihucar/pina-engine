# Texture Sample - Demonstrates texture loading and rendering

add_executable(sample-texture main.cpp)

target_link_libraries(sample-texture PRIVATE pina-engine)

# macOS: Link OpenGL and silence deprecation warnings
if(APPLE)
    find_library(OPENGL_LIBRARY OpenGL REQUIRED)
    target_link_libraries(sample-texture PRIVATE ${OPENGL_LIBRARY})
    target_compile_definitions(sample-texture PRIVATE GL_SILENCE_DEPRECATION)

    set_target_properties(sample-texture PROPERTIES
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_GUI_IDENTIFIER "com.pina.sample.texture"
        MACOSX_BUNDLE_BUNDLE_NAME "Texture Sample"
    )
endif()
