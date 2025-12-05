# Pina Editor - Scene graph editor with ImGui

add_executable(pina-editor
    src/main.cpp
    src/Editor.cpp
    src/Selection.cpp
    src/EditorCamera.cpp
    src/Panels/HierarchyPanel.cpp
    src/Panels/InspectorPanel.cpp
    src/Panels/ViewportPanel.cpp
    src/Gizmos/Gizmo.cpp
    src/Gizmos/GizmoRenderer.cpp
    src/Gizmos/TranslateGizmo.cpp
    src/Gizmos/RotateGizmo.cpp
    src/Gizmos/ScaleGizmo.cpp
)

target_include_directories(pina-editor PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

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
