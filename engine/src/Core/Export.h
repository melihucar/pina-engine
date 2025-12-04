#pragma once

/// Export/Import macros for shared library
/// PINA_API marks symbols for DLL export/import

#if defined(PINA_SHARED)
    #if defined(_WIN32) || defined(__CYGWIN__)
        #if defined(PINA_EXPORTS)
            #define PINA_API __declspec(dllexport)
        #else
            #define PINA_API __declspec(dllimport)
        #endif
    #elif defined(__GNUC__) || defined(__clang__)
        #if defined(PINA_EXPORTS)
            #define PINA_API __attribute__((visibility("default")))
        #else
            #define PINA_API
        #endif
    #else
        #define PINA_API
    #endif
#else
    #define PINA_API
#endif
