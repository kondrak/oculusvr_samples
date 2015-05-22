#ifndef OCULUSVR_INCLUDED
#define OCULUSVR_INCLUDED

#include "InputHandlers.hpp"
#include "renderer/OpenGL.hpp"
#include "renderer/OculusVRDebug.hpp"
#include "renderer/OVRCameraFrustum.hpp"
#include "Extras/OVR_Math.h"
#include "OVR_CAPI_0_6_0.h"

/*
 * Oculus Rift DK2 setup class (as of SDK 0.6.0.0)
 */
class OculusVR
{
public:
    OculusVR() : m_hmd(nullptr),
                 m_debugData(nullptr),
                 m_cameraFrustum(nullptr)
    {
    }

    ~OculusVR();
    bool  InitVR();
    bool InitVRBuffers(int windowWidth, int windowHeight);
    void  DestroyVR();
    const ovrSizei GetResolution() const;

    void  OnRenderStart();
    const OVR::Matrix4f OnEyeRender(int eyeIndex) const;
    void  OnEyeRenderFinish(int eyeIndex);
    void  SubmitFrame();
    void  BlitMirror();

    void  OnKeyPress(KeyCode key);
    void  CreateDebug();
    void  UpdateDebug();
    void  RenderDebug();
    void  RenderTrackerFrustum();   
    bool  IsDebugHMD() const { return (m_hmd->HmdCaps & ovrHmdCap_DebugDevice) != 0; }
    bool  IsDK2() const { return m_hmd->Type == ovrHmd_DK2; }
private:
    // A buffer struct used to store eye textures and framebuffers.
    // We create one instance for the left eye, one for the right eye.
    // Final endering is done via blitting two separate frame buffers into one render target.
    struct OVRBuffer
    {  
        OVRBuffer(const ovrHmd &hmd, int eyeIdx);
        void OnRender(int eyeIdx);
        void OnRenderFinish(int eyeIdx);
        void Destroy(const ovrHmd &hmd, int eyeIdx);

        ovrTexture m_eyeTexture;
        ovrSizei   m_eyeTextureSize;
        GLuint     m_eyeFbo      = 0;
        GLuint     m_depthBuffer = 0;
        ovrSwapTextureSet *m_swapTextureSet = nullptr;
    };

    ovrHmd            m_hmd;
    ovrEyeRenderDesc  m_eyeRenderDesc[2];

    ovrPosef          m_eyeRenderPose[2];
    ovrVector3f       m_hmdToEyeViewOffset[2];
    ovrGLTexture     *m_mirrorTexture;

    ovrFrameTiming    m_frameTiming;
    ovrTrackingState  m_trackingState;

    OVRBuffer        *m_eyeBuffers[ovrEye_Count];
    GLuint            m_mirrorFBO;

    OculusVRDebug    *m_debugData;
    OVRCameraFrustum *m_cameraFrustum;
};


#endif