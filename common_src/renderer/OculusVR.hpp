#ifndef OCULUSVR_INCLUDED
#define OCULUSVR_INCLUDED

#include "InputHandlers.hpp"
#include "renderer/OpenGL.hpp"
#include "renderer/OculusVRDebug.hpp"
#include "renderer/OVRCameraFrustum.hpp"
#include "Extras/OVR_Math.h"
#include "OVR_CAPI_0_5_0.h"

/*
 * Oculus Rift DK2 setup class (as of SDK 0.5.0.1)
 */
class OculusVR
{
public:
    OculusVR() : m_hmd(nullptr),
                 m_frameBuffer(0),
                 m_renderBuffer(0),
                 m_texture(0),
                 m_debugData(nullptr),
                 m_cameraFrustum(nullptr)
    {
    }

    ~OculusVR();
    bool  InitVR();
    bool  InitVRBuffers();
    void  ConfigureRender(void *window, int w, int h);
    void  DestroyVR();

    void  OnRenderStart();
    const OVR::Matrix4f OnEyeRender(int eyeIndex) const;
    void  OnRenderEnd();

    void  OnKeyPress(KeyCode key);
    const OVR::Vector4i RenderDimensions() const;
    const bool IsDirectMode() const;
    void  CreateDebug();
    void  UpdateDebug();
    void  RenderDebug();
    void  RenderTrackerFrustum();
    bool  IsDebugHMD() const { return (m_hmd->HmdCaps & ovrHmdCap_DebugDevice) != 0; }
    bool  IsDK2() const { return m_hmd->Type == ovrHmd_DK2; }
private:
    ovrHmd           m_hmd;
    ovrEyeRenderDesc m_eyeRenderDesc[2];
    ovrRecti         m_eyeRenderViewport[2];
    ovrTexture       m_eyeTexture[2];
    ovrPosef         m_eyeRenderPose[2];
    ovrVector3f      m_useHmdToEyeViewOffset[2];

    ovrFrameTiming   m_frameTiming;
    ovrTrackingState m_trackingState;

    GLuint m_frameBuffer;
    GLuint m_renderBuffer;
    GLuint m_texture;

    OculusVRDebug    *m_debugData;
    OVRCameraFrustum *m_cameraFrustum;
};


#endif