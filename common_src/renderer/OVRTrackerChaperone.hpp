#ifndef OVRTRACKERCHAPERONE_HPP
#define OVRTRACKERCHAPERONE_HPP
#include "renderer/OpenGL.hpp"
#include "OVR_CAPI.h"
#include "OVR_CAPI_GL.h"
#include "Extras/OVR_Math.h"

/*
 * Rendering class for Vive-style tracker chaperone
 */
class OVRTrackerChaperone
{
public:
    ~OVRTrackerChaperone();
    void Recalculate(ovrSession session, const OVR::Vector3f &headPos);
    void OnRender();
private:
    GLuint m_vertexArray;
    GLuint m_vertexBuffers[4];
    float  m_planeDistances[4];
};

#endif