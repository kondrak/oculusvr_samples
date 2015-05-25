#include "renderer/OculusVR.hpp"
#include "renderer/ShaderManager.hpp"
//#include <GL/CAPI_GLE.h>

OculusVR::OVRBuffer::OVRBuffer(const ovrHmd &hmd, int eyeIdx)
{
    m_eyeTextureSize = ovrHmd_GetFovTextureSize(hmd, (ovrEyeType)eyeIdx, hmd->DefaultEyeFov[eyeIdx], 1.0f);

    ovrHmd_CreateSwapTextureSetGL(hmd, GL_RGBA, m_eyeTextureSize.w, m_eyeTextureSize.h, &m_swapTextureSet);

    for (int j = 0; j < m_swapTextureSet->TextureCount; ++j)
    {
        ovrGLTexture* tex = (ovrGLTexture*)&m_swapTextureSet->Textures[j];
        glBindTexture(GL_TEXTURE_2D, tex->OGL.TexId);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    }

    glGenFramebuffers(1, &m_eyeFbo);

    // create depth buffer
    glGenTextures(1, &m_depthBuffer);
    glBindTexture(GL_TEXTURE_2D, m_depthBuffer);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    GLenum internalFormat = GL_DEPTH_COMPONENT24;
    GLenum type = GL_UNSIGNED_INT;

    /*if (GLE_ARB_depth_buffer_float)
    {
    internalFormat = GL_DEPTH_COMPONENT32F;
    type = GL_FLOAT;
    } */

    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, m_eyeTextureSize.w, m_eyeTextureSize.h, 0, GL_DEPTH_COMPONENT, type, NULL);
}

void OculusVR::OVRBuffer::OnRender()
{
    // Increment to use next texture, just before writing
    m_swapTextureSet->CurrentIndex = (m_swapTextureSet->CurrentIndex + 1) % m_swapTextureSet->TextureCount;

    // Switch to eye render target
    ovrGLTexture* tex = (ovrGLTexture*)&m_swapTextureSet->Textures[m_swapTextureSet->CurrentIndex];

    glBindFramebuffer(GL_FRAMEBUFFER, m_eyeFbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex->OGL.TexId, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_depthBuffer, 0);

    glViewport(0, 0, m_eyeTextureSize.w, m_eyeTextureSize.h);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void OculusVR::OVRBuffer::OnRenderFinish()
{
    glBindFramebuffer(GL_FRAMEBUFFER, m_eyeFbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, 0, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, 0, 0);
}

void OculusVR::OVRBuffer::Destroy(const ovrHmd &hmd)
{
    if (glIsFramebuffer(m_eyeFbo))
        glDeleteFramebuffers(1, &m_eyeFbo);  

    if (glIsTexture(m_depthBuffer))
        glDeleteTextures(1, &m_depthBuffer);

    ovrHmd_DestroySwapTextureSet(hmd, m_swapTextureSet);
}

OculusVR::~OculusVR()
{
    ovrHmd_Destroy(m_hmd);
    ovr_Shutdown();
    m_hmd = nullptr;
}

bool OculusVR::InitVR()
{
    ovrResult result = ovr_Initialize(nullptr);

    if (result != ovrSuccess)
    {
        LOG_MESSAGE_ASSERT(false, "Failed to initialize LibOVR");
        return false;
    }

    result = ovrHmd_Create(0, &m_hmd);

    if (result != ovrSuccess)
    {
        result = ovrHmd_CreateDebug(ovrHmd_DK2, &m_hmd);
        LOG_MESSAGE_ASSERT(result == ovrSuccess, "Failed to create OVR device");
    }

    ovrHmd_SetEnabledCaps(m_hmd, ovrHmdCap_LowPersistence | ovrHmdCap_DynamicPrediction);
    ovrHmd_ConfigureTracking(m_hmd, ovrTrackingCap_Orientation |
                                    ovrTrackingCap_MagYawCorrection |
                                    ovrTrackingCap_Position, 0);
                                    
    m_cameraFrustum = new OVRCameraFrustum;

    return result == ovrSuccess;
}

bool OculusVR::InitVRBuffers(int windowWidth, int windowHeight)
{
    for (int eyeIdx = 0; eyeIdx < ovrEye_Count; eyeIdx++)
    {
        m_eyeBuffers[eyeIdx]    = new OVRBuffer(m_hmd, eyeIdx);
        m_eyeRenderDesc[eyeIdx] = ovrHmd_GetRenderDesc(m_hmd, (ovrEyeType)eyeIdx, m_hmd->DefaultEyeFov[eyeIdx]);
    }

    // since SDK 0.6 we're using a mirror texture + FBO which in turn copies contents of mirror to back buffer
    ovrHmd_CreateMirrorTextureGL(m_hmd, GL_RGBA, windowWidth, windowHeight, (ovrTexture**)&m_mirrorTexture);

    // Configure the mirror read buffer
    glGenFramebuffers(1, &m_mirrorFBO);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, m_mirrorFBO);
    glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_mirrorTexture->OGL.TexId, 0);
    glFramebufferRenderbuffer(GL_READ_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, 0);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        glDeleteFramebuffers(1, &m_mirrorFBO);
        LOG_MESSAGE_ASSERT(false, "Could not initialize VR buffers!");
        return false;
    }

    return true;
}

void OculusVR::DestroyVR()
{
    if (m_hmd)
    {
        delete m_debugData;
        delete m_cameraFrustum;

        m_debugData = nullptr;
        m_cameraFrustum = nullptr;

        if (glIsFramebuffer(m_mirrorFBO))
            glDeleteFramebuffers(1, &m_mirrorFBO);

        ovrHmd_DestroyMirrorTexture(m_hmd, (ovrTexture*)m_mirrorTexture);

        for (int eyeIdx = 0; eyeIdx < ovrEye_Count; eyeIdx++)
        {
            m_eyeBuffers[eyeIdx]->Destroy(m_hmd);
        }
    }
}

const ovrSizei OculusVR::GetResolution() const
{
    ovrSizei resolution = { m_hmd->Resolution.w, m_hmd->Resolution.h };
    return resolution;
}

void OculusVR::OnRenderStart()
{
    m_hmdToEyeViewOffset[0] = m_eyeRenderDesc[0].HmdToEyeViewOffset;
    m_hmdToEyeViewOffset[1] = m_eyeRenderDesc[1].HmdToEyeViewOffset;

    m_frameTiming   = ovrHmd_GetFrameTiming(m_hmd, 0);
    m_trackingState = ovrHmd_GetTrackingState(m_hmd, m_frameTiming.DisplayMidpointSeconds);

    // Get both eye poses simultaneously, with IPD offset already included. 
    ovr_CalcEyePoses(m_trackingState.HeadPose.ThePose, m_hmdToEyeViewOffset, m_eyeRenderPose);
}


const OVR::Matrix4f OculusVR::OnEyeRender(int eyeIndex) const
{
    int eye = m_hmd->EyeRenderOrder[eyeIndex];
    m_eyeBuffers[eye]->OnRender();

    return OVR::Matrix4f(ovrMatrix4f_Projection(m_eyeRenderDesc[eye].Fov, 0.01f, 10000.0f, ovrProjection_RightHanded)) *
           OVR::Matrix4f::Translation(m_hmdToEyeViewOffset[eye]) *
           OVR::Matrix4f(OVR::Quatf(m_eyeRenderPose[eye].Orientation).Inverted()) *
           OVR::Matrix4f::Translation(-OVR::Vector3f(m_eyeRenderPose[eye].Position));
}

void OculusVR::OnEyeRenderFinish(int eyeIndex)
{
    m_eyeBuffers[ m_hmd->EyeRenderOrder[eyeIndex] ]->OnRenderFinish();
}

void OculusVR::SubmitFrame()
{

    // Set up positional data.
    ovrViewScaleDesc viewScaleDesc;
    viewScaleDesc.HmdSpaceToWorldScaleInMeters = 1.0f;
    viewScaleDesc.HmdToEyeViewOffset[0] = m_hmdToEyeViewOffset[0];
    viewScaleDesc.HmdToEyeViewOffset[1] = m_hmdToEyeViewOffset[1];

    ovrLayerEyeFov ld;
    ld.Header.Type = ovrLayerType_EyeFov;
    ld.Header.Flags = ovrLayerFlag_TextureOriginAtBottomLeft;   // Because OpenGL.

    for (int eye = 0; eye < ovrEye_Count; eye++)
    {
        ld.ColorTexture[eye] = m_eyeBuffers[eye]->m_swapTextureSet;
        ld.Viewport[eye] = OVR::Recti(m_eyeBuffers[eye]->m_eyeTextureSize);
        ld.Fov[eye] = m_hmd->DefaultEyeFov[eye];
        ld.RenderPose[eye] = m_eyeRenderPose[eye];
    }

    ovrLayerHeader* layers = &ld.Header;
    ovrResult result = ovrHmd_SubmitFrame(m_hmd, 0, &viewScaleDesc, &layers, 1);
}

void OculusVR::BlitMirror()
{
    // Blit mirror texture to back buffer
    glBindFramebuffer(GL_READ_FRAMEBUFFER, m_mirrorFBO);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    GLint w = m_mirrorTexture->OGL.Header.TextureSize.w;
    GLint h = m_mirrorTexture->OGL.Header.TextureSize.h;
    glBlitFramebuffer(0, h, w, 0, 0, 0, w, h, GL_COLOR_BUFFER_BIT, GL_NEAREST);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
}

void OculusVR::OnKeyPress(KeyCode key)
{
    switch (key)
    {
    case KEY_SPACE:
        ovrHmd_RecenterPose(m_hmd);
        break;
    }
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

    // Rendered size changes based on selected options & dynamic rendering.
    int pixelSizeWidth  = m_eyeBuffers[0]->m_eyeTextureSize.w + m_eyeBuffers[1]->m_eyeTextureSize.w;
    int pixelSizeHeight = (m_eyeBuffers[0]->m_eyeTextureSize.h + m_eyeBuffers[1]->m_eyeTextureSize.h) / 2;

    ovrSizei texSize = { pixelSizeWidth, pixelSizeHeight };
    m_debugData->OnRender(m_hmd, m_trackingState, m_eyeRenderDesc, texSize);
}

void OculusVR::RenderTrackerFrustum()
{
    if (!IsDebugHMD() && IsDK2() && m_cameraFrustum)
    {
        m_cameraFrustum->Recalculate(m_hmd);
        m_cameraFrustum->OnRender();
    }
}