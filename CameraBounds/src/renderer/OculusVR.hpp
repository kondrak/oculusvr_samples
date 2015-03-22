#ifndef OCULUSVR_INCLUDED
#define OCULUSVR_INCLUDED

#include "InputHandlers.hpp"
#include "renderer/OpenGL.hpp"
#include "OVR.h"
#include "OVR_CAPI.h"
#include "OVR_CAPI_GL.h"
#include "Kernel/OVR_Math.h"

class TrackerFrustum
{
public:
    void Recalculate(ovrHmd hmd);
    void OnRender();
private:
    GLuint m_vertexArrays[3];
    GLuint m_vertexBuffers[3];
};

class OculusVR
{
public:
    OculusVR() : m_hmd(nullptr),
                 m_frameBuffer(0),
                 m_renderBuffer(0),
                 m_texture(0)
    {
    }

    ~OculusVR();
    bool InitVR();
    bool InitVRBuffers();
    void ConfigureRender(void *window, int w, int h);
    void DestroyVR();

    void OnRenderStart();
    const OVR::Matrix4f OnEyeRender(int eyeIndex) const;
    void OnRenderEnd();

    void RenderTrackerFrustum();

    void OnKeyPress(KeyCode key);
    const OVR::Vector4i RenderDimensions() const;
    const bool IsDirectMode() const;
private:
    ovrHmd m_hmd;
    ovrEyeRenderDesc eyeRenderDesc[2];
    ovrRecti         eyeRenderViewport[2];
    ovrGLTexture     eyeTexture[2];
    ovrPosef         eyeRenderPose[2];
    ovrVector3f      useHmdToEyeViewOffset[2];

    GLuint m_frameBuffer;
    GLuint m_renderBuffer;
    GLuint m_texture;

    TrackerFrustum m_trackerFrustum;
};


#endif