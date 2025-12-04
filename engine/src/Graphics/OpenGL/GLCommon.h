#pragma once

/// Pina Engine - OpenGL Common Header
/// Platform-specific OpenGL includes and utilities

// Platform-specific OpenGL headers
#ifdef __APPLE__
    #ifndef GL_SILENCE_DEPRECATION
        #define GL_SILENCE_DEPRECATION
    #endif
    #include <OpenGL/gl3.h>
#elif defined(_WIN32)
    // TODO: Use glad on Windows
    #include <GL/gl.h>
#else
    // TODO: Use glad on Linux
    #include <GL/gl.h>
#endif

#include <string>
#include <iostream>

namespace Pina {

/// Check for OpenGL errors and log them
inline void GLCheckError(const char* file, int line) {
#ifdef PINA_DEBUG
    GLenum error = glGetError();
    while (error != GL_NO_ERROR) {
        std::string errorStr;
        switch (error) {
            case GL_INVALID_ENUM:      errorStr = "GL_INVALID_ENUM"; break;
            case GL_INVALID_VALUE:     errorStr = "GL_INVALID_VALUE"; break;
            case GL_INVALID_OPERATION: errorStr = "GL_INVALID_OPERATION"; break;
            case GL_OUT_OF_MEMORY:     errorStr = "GL_OUT_OF_MEMORY"; break;
            default:                   errorStr = "Unknown"; break;
        }
        std::cerr << "OpenGL Error: " << errorStr << " at " << file << ":" << line << std::endl;
        error = glGetError();
    }
#else
    (void)file;
    (void)line;
#endif
}

#define GL_CHECK_ERROR() Pina::GLCheckError(__FILE__, __LINE__)

} // namespace Pina
