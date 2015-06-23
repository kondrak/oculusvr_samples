#ifndef OPENGL_INCLUDED
#define OPENGL_INCLUDED

#ifdef _WIN32
    #include <Windows.h>
    #include "glew-1.11.0/include/GL/glew.h"
#else
    #error "Only Windows platform supported for now."
#endif

#include "Math.hpp"
#include "Utils.hpp"

namespace Renderer
{
    void MakePerspective(Math::Matrix4f &matrix, float fov, float scrRatio, float nearPlane, float farPlane);
    void MakeOrthogonal(Math::Matrix4f &matrix, float left, float right, float bottom, float top, float nearPlane, float farPlane);
    void MakeView(Math::Matrix4f &matrix, const Math::Vector3f &eye, const Math::Vector3f &target, const Math::Vector3f &up);
}

#endif