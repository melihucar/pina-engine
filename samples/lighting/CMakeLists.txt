# Lighting Sample - Demonstrates all light types

add_executable(sample-lighting main.cpp)

target_link_libraries(sample-lighting PRIVATE pina-engine)

# macOS: Link OpenGL and silence deprecation warnings
if(APPLE)
    find_library(OPENGL_LIBRARY OpenGL REQUIRED)
    target_link_libraries(sample-lighting PRIVATE ${OPENGL_LIBRARY})
    target_compile_definitions(sample-lighting PRIVATE GL_SILENCE_DEPRECATION)

    set_target_properties(sample-lighting PROPERTIES
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_GUI_IDENTIFIER "com.pina.sample.lighting"
        MACOSX_BUNDLE_BUNDLE_NAME "Lighting Sample"
    )
endif()
