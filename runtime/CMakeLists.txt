# Pina Runtime - Hot-reload host executable
# TODO: Implement DLL loading and file watching

add_executable(pina-runtime src/main.cpp)

target_link_libraries(pina-runtime PRIVATE pina-engine)

# macOS: Link OpenGL and silence deprecation warnings
if(APPLE)
    find_library(OPENGL_LIBRARY OpenGL REQUIRED)
    target_link_libraries(pina-runtime PRIVATE ${OPENGL_LIBRARY})
    target_compile_definitions(pina-runtime PRIVATE GL_SILENCE_DEPRECATION)

    set_target_properties(pina-runtime PROPERTIES
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_GUI_IDENTIFIER "com.pina.runtime"
        MACOSX_BUNDLE_BUNDLE_NAME "Pina Runtime"
    )
endif()
