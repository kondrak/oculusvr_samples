#include "renderer/OculusVR.hpp"
#include "renderer/ShaderManager.hpp"
#include "OVR_CAPI_GL.h"

OculusVR::~OculusVR()
{
    DestroyVR();
    delete m_debugData;
}

bool OculusVR::InitVR()
{
    ovr_Initialize();

    m_hmd = ovrHmd_Create(0);

    if (!m_hmd)
    {
        m_hmd = ovrHmd_CreateDebug(ovrHmd_DK2);
    }

    ovrHmd_SetEnabledCaps(m_hmd, ovrHmdCap_LowPersistence | ovrHmdCap_DynamicPrediction);
    ovrHmd_ConfigureTracking(m_hmd, ovrTrackingCap_Orientation |
                                    ovrTrackingCap_MagYawCorrection |
                                    ovrTrackingCap_Position, 0);

    return m_hmd != nullptr;
}

bool OculusVR::InitVRBuffers()
{
    OVR::Sizei recommendedTex0Size = ovrHmd_GetFovTextureSize(m_hmd, ovrEye_Left, m_hmd->DefaultEyeFov[0], 1.0f);
    OVR::Sizei recommendedTex1Size = ovrHmd_GetFovTextureSize(m_hmd, ovrEye_Right, m_hmd->DefaultEyeFov[1], 1.0f);
    OVR::Sizei renderTargetSize;
    renderTargetSize.w = recommendedTex0Size.w + recommendedTex1Size.w;
    renderTargetSize.h = max(recommendedTex0Size.h, recommendedTex1Size.h);

    glGenFramebuffers(1, &m_frameBuffer);
    glGenTextures(1, &m_texture);

    glBindFramebuffer(GL_FRAMEBUFFER, m_frameBuffer);
    glBindTexture(GL_TEXTURE_2D, m_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, renderTargetSize.w, renderTargetSize.h, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_texture, 0);

    glGenRenderbuffers(1, &m_renderBuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, m_renderBuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, renderTargetSize.w, renderTargetSize.h);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_renderBuffer);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        glDeleteFramebuffers(1, &m_frameBuffer);
        glDeleteTextures(1, &m_texture);
        glDeleteRenderbuffers(1, &m_renderBuffer);

        LOG_MESSAGE_ASSERT(false, "Could not initialize VR buffers!");
        return false;
    }

    m_eyeRenderViewport[0].Pos  = OVR::Vector2i(0, 0);
    m_eyeRenderViewport[0].Size = OVR::Sizei(renderTargetSize.w / 2, renderTargetSize.h);
    m_eyeRenderViewport[1].Pos  = OVR::Vector2i((renderTargetSize.w + 1) / 2, 0);
    m_eyeRenderViewport[1].Size = m_eyeRenderViewport[0].Size;

    m_eyeTexture[0].Header.API = ovrRenderAPI_OpenGL;
    m_eyeTexture[0].Header.TextureSize = renderTargetSize;
    m_eyeTexture[0].Header.RenderViewport = m_eyeRenderViewport[0];   
    ((ovrGLTextureData*)&m_eyeTexture[0])->TexId = m_texture;

    m_eyeTexture[1] = m_eyeTexture[0];
    m_eyeTexture[1].Header.RenderViewport = m_eyeRenderViewport[1];

    return true;
}

void OculusVR::ConfigureRender(void *window, int w, int h)
{
    ovrGLConfig cfg;
    memset(&cfg, 0, sizeof(ovrGLConfig));

    cfg.OGL.Header.API = ovrRenderAPI_OpenGL;

    // Normally it would be best to use m_hmd->Resolution.w/h here but with OVR + OpenGL it's impossible to separate window size from framebuffer size.
    // This applies to direct mode only.
    if (IsDirectMode())
    {
        cfg.OGL.Header.BackBufferSize = OVR::Sizei(w, h);
    }
    else
    {
        cfg.OGL.Header.BackBufferSize = OVR::Sizei(m_hmd->Resolution.w, m_hmd->Resolution.h);
    }

    cfg.OGL.Header.Multisample = 1;
    cfg.OGL.Window = (HWND)window;
    cfg.OGL.DC = nullptr;

    if (IsDirectMode())
        ovrHmd_AttachToWindow(m_hmd, window, NULL, NULL);

    // Set the configuration and receive eye render descriptors in return.
    ovrHmd_ConfigureRendering(m_hmd, &cfg.Config, ovrDistortionCap_Vignette
                                                  | ovrDistortionCap_TimeWarp
                                                  | ovrDistortionCap_Overdrive
                                                  | ovrDistortionCap_HqDistortion,
                                                  m_hmd->DefaultEyeFov, 
                                                  m_eyeRenderDesc);
}

void OculusVR::DestroyVR()
{
    if (m_hmd)
    {
        glDeleteFramebuffers(1, &m_frameBuffer);
        glDeleteTextures(1, &m_texture);
        glDeleteRenderbuffers(1, &m_renderBuffer);

        ovrHmd_Destroy(m_hmd);
        ovr_Shutdown();
        m_hmd = nullptr;
    }
}


void OculusVR::OnRenderStart()
{
    m_useHmdToEyeViewOffset[0] = m_eyeRenderDesc[0].HmdToEyeViewOffset;
    m_useHmdToEyeViewOffset[1] = m_eyeRenderDesc[1].HmdToEyeViewOffset;

    m_frameTiming   = ovrHmd_BeginFrame(m_hmd, 0);
    m_trackingState = ovrHmd_GetTrackingState(m_hmd, m_frameTiming.ScanoutMidpointSeconds);

    glBindFramebuffer(GL_FRAMEBUFFER, m_frameBuffer);

    // Get both eye poses simultaneously, with IPD offset already included. 
    ovrHmd_GetEyePoses(m_hmd, 0, m_useHmdToEyeViewOffset, m_eyeRenderPose, NULL);
}


const OVR::Matrix4f OculusVR::OnEyeRender(int eyeIndex) const
{
    ovrEyeType eye = m_hmd->EyeRenderOrder[eyeIndex];

    glViewport(m_eyeRenderViewport[eye].Pos.x, m_eyeRenderViewport[eye].Pos.y, m_eyeRenderViewport[eye].Size.w, m_eyeRenderViewport[eye].Size.h);

    return OVR::Matrix4f(ovrMatrix4f_Projection(m_eyeRenderDesc[eye].Fov, 0.01f, 10000.0f, true)) *
           OVR::Matrix4f::Translation(m_eyeRenderDesc[eye].HmdToEyeViewOffset) *
           OVR::Matrix4f(OVR::Quatf(m_eyeRenderPose[eye].Orientation).Inverted()) *
           OVR::Matrix4f::Translation(-OVR::Vector3f(m_eyeRenderPose[eye].Position));
}


void OculusVR::OnRenderEnd()
{
    ovrHmd_EndFrame(m_hmd, m_eyeRenderPose, &m_eyeTexture[0]);
}

void OculusVR::OnKeyPress(KeyCode key)
{
    // Dismiss the Health and Safety message by pressing any key
    ovrHmd_DismissHSWDisplay(m_hmd);

    switch (key)
    {
    case KEY_SPACE:
        ovrHmd_RecenterPose(m_hmd);
        break;
    }
}

const OVR::Vector4i OculusVR::RenderDimensions() const
{
    return OVR::Vector4i(m_hmd->WindowsPos.x, m_hmd->WindowsPos.y, m_hmd->Resolution.w, m_hmd->Resolution.h);
}

const bool OculusVR::IsDirectMode() const
{
    return (m_hmd->HmdCaps & ovrHmdCap_ExtendDesktop) == 0;
}

void OculusVR::CreateDebug()
{
    if (!m_debugData)
        m_debugData = new OculusVRDebug();
}

void OculusVR::UpdateDebug()
{
    LOG_MESSAGE_ASSERT(m_debugData, "Debug data not created!");
    m_debugData->OnUpdate(m_trackingState);
}

void OculusVR::RenderDebug()
{
    LOG_MESSAGE_ASSERT(m_debugData, "Debug data not created!");
    m_debugData->OnRender(m_hmd, m_trackingState, m_eyeRenderDesc, m_eyeTexture);
}

void OculusVR::RenderTrackerFrustum()
{
    if (!IsDebugHMD() && IsDK2())
    {
        m_cameraFrustum.Recalculate(m_hmd);
        m_cameraFrustum.OnRender();
    }
}