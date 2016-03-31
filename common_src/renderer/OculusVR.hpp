#ifndef OCULUSVR_INCLUDED
#define OCULUSVR_INCLUDED

#include "InputHandlers.hpp"
#include "renderer/OpenGL.hpp"
#include "renderer/OculusVRDebug.hpp"
#include "renderer/OVRCameraFrustum.hpp"
#include "Extras/OVR_Math.h"
#include "OVR_CAPI.h"

/*
 * Oculus Rift setup class (as of SDK 1.3.0)
 */
class OculusVR
{
public:
    OculusVR() : m_hmdSession(nullptr),
                 m_debugData(nullptr),
                 m_cameraFrustum(nullptr),
                 m_msaaEnabled(false),
                 m_frameIndex(0),
                 m_sensorSampleTime(0)
    {
    }

    ~OculusVR();
    bool  InitVR();
    bool  InitVRBuffers(int windowWidth, int windowHeight);
    bool  InitNonDistortMirror(int windowWidth, int windowHeight); // create non-distorted mirror if necessary (debug purposes)
    void  DestroyVR();
    const ovrSizei GetResolution() const;
    void  OnRenderStart();
    const OVR::Matrix4f OnEyeRender(int eyeIndex);
    void  OnEyeRenderFinish(int eyeIndex);
    const OVR::Matrix4f GetEyeMVPMatrix(int eyeIdx) const;
    void  SubmitFrame();

    void  BlitMirror(ovrEyeType numEyes=ovrEye_Count, int offset = 0);   // regular OculusVR mirror view
    void  OnNonDistortMirrorStart();        // non-distorted mirror rendering start (debug purposes)
    void  BlitNonDistortMirror(int offset); // non-distorted mirror rendering (debug purposes)

    void  OnKeyPress(KeyCode key);
    void  CreateDebug();
    void  UpdateDebug();
    void  RenderDebug();
    void  RenderTrackerFrustum();   
    bool  IsDebugHMD() const { return (m_hmdDesc.AvailableHmdCaps & ovrHmdCap_DebugDevice) != 0; }
    void  ShowPerfStats(ovrPerfHudMode statsMode);
    void  SetMSAA(bool val) { m_msaaEnabled = val; }
    bool  MSAAEnabled() const { return m_msaaEnabled; }
private:
    // A buffer struct used to store eye textures and framebuffers.
    // We create one instance for the left eye, one for the right eye.
    // Final rendering is done via blitting two separate frame buffers into one render target.
    struct OVRBuffer
    {  
        OVRBuffer(const ovrSession &session, int eyeIdx);
        void OnRender();
        void OnRenderFinish();
        void SetupMSAA(); 
        void OnRenderMSAA();
        void OnRenderMSAAFinish();
        void Destroy(const ovrSession &session);

        ovrSizei   m_eyeTextureSize;
        GLuint     m_eyeFbo      = 0;
        GLuint     m_eyeTexId    = 0;
        GLuint     m_depthBuffer = 0;

        GLuint m_msaaEyeFbo   = 0;   // framebuffer for MSAA texture
        GLuint m_eyeTexMSAA   = 0;   // color texture for MSAA
        GLuint m_depthTexMSAA = 0;   // depth texture for MSAA

        ovrTextureSwapChain m_swapTextureChain = nullptr;
    };

    // data and buffers used to render to HMD
    ovrSession        m_hmdSession;
    ovrHmdDesc        m_hmdDesc;
    ovrEyeRenderDesc  m_eyeRenderDesc[ovrEye_Count];
    ovrPosef          m_eyeRenderPose[ovrEye_Count];
    ovrVector3f       m_hmdToEyeOffset[ovrEye_Count];
    OVRBuffer        *m_eyeBuffers[ovrEye_Count];

    OVR::Matrix4f     m_projectionMatrix[ovrEye_Count];
    OVR::Matrix4f     m_eyeViewOffset[ovrEye_Count];
    OVR::Matrix4f     m_eyeOrientation[ovrEye_Count];
    OVR::Matrix4f     m_eyePose[ovrEye_Count];

    // frame timing data and tracking info
    double            m_frameTiming;
    ovrTrackingState  m_trackingState;

    // mirror texture used to render HMD view to OpenGL window
    ovrMirrorTexture     m_mirrorTexture;
    ovrMirrorTextureDesc m_mirrorDesc;

    // debug non-distorted mirror texture data
    GLuint            m_nonDistortTexture;
    GLuint            m_nonDistortDepthBuffer;
    GLuint            m_mirrorFBO;
    GLuint            m_nonDistortFBO;
    int               m_nonDistortViewPortWidth;
    int               m_nonDistortViewPortHeight;
    bool              m_msaaEnabled;
    long long         m_frameIndex;
    double            m_sensorSampleTime;

    // debug hardware output data
    OculusVRDebug    *m_debugData;

    // debug camera frustum renderer
    OVRCameraFrustum *m_cameraFrustum;
};


#endif