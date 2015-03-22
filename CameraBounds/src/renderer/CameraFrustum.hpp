#ifndef CAMERA_FRUSTUM_HPP
#define CAMERA_FRUSTUM_HPP
#include "renderer/OpenGL.hpp"
#include "OVR.h"
#include "OVR_CAPI.h"
#include "OVR_CAPI_GL.h"
#include "Kernel/OVR_Math.h"

/*
 * Rendering class for IR tracking camera bounds (frustum)
 */
class CameraFrustum
{
public:
    void Recalculate(ovrHmd hmd);
    void OnRender();
private:
    GLuint m_vertexArrays[3];
    GLuint m_vertexBuffers[3];
};

#endif