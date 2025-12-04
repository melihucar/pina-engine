# Pina Editor - Dear ImGui based editor
# TODO: Implement Dear ImGui integration

add_executable(pina-editor src/main.cpp)

target_link_libraries(pina-editor
    PRIVATE
        pina-engine
        imgui
)

# macOS: Link OpenGL and silence deprecation warnings
if(APPLE)
    find_library(OPENGL_LIBRARY OpenGL REQUIRED)
    target_link_libraries(pina-editor PRIVATE ${OPENGL_LIBRARY})
    target_compile_definitions(pina-editor PRIVATE GL_SILENCE_DEPRECATION)

    set_target_properties(pina-editor PROPERTIES
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_GUI_IDENTIFIER "com.pina.editor"
        MACOSX_BUNDLE_BUNDLE_NAME "Pina Editor"
    )
endif()
