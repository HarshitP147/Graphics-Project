#include <iostream>
#include <glad/gl.h>

inline void CheckOpenGLErrors(const char* label) {
    GLenum err;
    bool foundError = false;
    while ((err = glGetError()) != GL_NO_ERROR) {
        foundError = true;
        std::cerr << "[OpenGL Error] " << std::hex << err << " at " << label << std::endl;
    }
    // Optionally, if you want to break in a debugger on error:
    // if (foundError) __debugbreak(); // Windows/MSVC only
}