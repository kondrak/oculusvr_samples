#ifndef OPENGL_INCLUDED
#define OPENGL_INCLUDED

#ifdef _WIN32
    #include <Windows.h>
    #include "glew-1.11.0/include/GL/glew.h"
#else
    #error "Only Windows platform supported for now."
#endif

#ifdef _DEBUG
#include <sstream>
#define LOG_MESSAGE(msg) { \
    std::stringstream msgStr; \
    msgStr << "[LOG]: " << msg << "\n"; \
    LogError(msgStr.str().c_str()); \
}

#define LOG_MESSAGE_ASSERT(x, msg) \
    if(!x) { \
    std::stringstream msgStr; \
    msgStr << "[!ASSERT!]: " << msg << "\n"; \
    LogError(msgStr.str().c_str()); \
    __debugbreak(); \
}
#else
#define LOG_MESSAGE(msg)
#define LOG_MESSAGE_ASSERT(x, msg)
#endif

#include "Math.hpp"

namespace Renderer
{
    void MakePerspective(Math::Matrix4f &matrix, float fov, float scrRatio, float nearPlane, float farPlane);
    void MakeOrthogonal(Math::Matrix4f &matrix, float left, float right, float bottom, float top, float nearPlane, float farPlane);
    void MakeView(Math::Matrix4f &matrix, const Math::Vector3f &eye, const Math::Vector3f &target, const Math::Vector3f &up);
}

void LogError(const char *msg);

#endif