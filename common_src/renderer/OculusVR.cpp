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

    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, m_eyeTextureSize.w, m_eyeTextureSize.h, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, NULL);

    // MSAA color texture and fbo setup; depth buffer is the same as non-MSAA since OpenGL doesn't support it yet with OVR
    // simply comment this line out to skip MSAA altogether
    SetupMSAA();
}

void OculusVR::OVRBuffer::SetupMSAA()
{
    glGenFramebuffers(1, &m_msaaEyeFbo);

    // create color MSAA texture
    int samples  = 4;
    int mipcount = 1;

    glGenTextures(1, &m_eyeTexMSAA);
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, m_eyeTexMSAA);

    LOG_MESSAGE_ASSERT(!glGetError(), "Could not create MSAA texture");

    glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, samples, GL_RGBA, m_eyeTextureSize.w, m_eyeTextureSize.h, false);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

    // linear filter
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_MAX_LEVEL, mipcount - 1);    

    // create MSAA depth buffer
    glGenTextures(1, &m_depthTexMSAA);
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, m_depthTexMSAA);

    glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, samples, GL_DEPTH_COMPONENT, m_eyeTextureSize.w, m_eyeTextureSize.h, false);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_MAX_LEVEL, mipcount - 1);

    LOG_MESSAGE_ASSERT(!glGetError(), "MSAA setup failed");
}

void OculusVR::OVRBuffer::OnRenderMSAA()
{
    // Increment to use next texture, just before writing
    m_swapTextureSet->CurrentIndex = (m_swapTextureSet->CurrentIndex + 1) % m_swapTextureSet->TextureCount;

    glBindFramebuffer(GL_FRAMEBUFFER, m_msaaEyeFbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, m_eyeTexMSAA, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D_MULTISAMPLE, m_depthTexMSAA, 0);

    glViewport(0, 0, m_eyeTextureSize.w, m_eyeTextureSize.h);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void OculusVR::OVRBuffer::OnRenderMSAAFinish()
{
    // blit the contents of MSAA FBO to the regular eye buffer "connected" to the HMD
    ovrGLTexture* tex = (ovrGLTexture*)&m_swapTextureSet->Textures[m_swapTextureSet->CurrentIndex];

    glBindFramebuffer(GL_READ_FRAMEBUFFER, m_msaaEyeFbo);
    glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D_MULTISAMPLE, m_eyeTexMSAA, 0);
    glFramebufferRenderbuffer(GL_READ_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, 0);

    LOG_MESSAGE_ASSERT((glCheckFramebufferStatus(GL_READ_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE), "Could not complete framebuffer operation");

    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_eyeFbo);
    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex->OGL.TexId, 0);
    glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, 0);

    LOG_MESSAGE_ASSERT((glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE), "Could not complete framebuffer operation");

    glBlitFramebuffer(0, 0, m_eyeTextureSize.w, m_eyeTextureSize.h,
                      0, 0, m_eyeTextureSize.w, m_eyeTextureSize.h, GL_COLOR_BUFFER_BIT, GL_NEAREST);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
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

    if (glIsFramebuffer(m_msaaEyeFbo))
        glDeleteFramebuffers(1, &m_msaaEyeFbo);

    if (glIsTexture(m_eyeTexMSAA))
        glDeleteTextures(1, &m_eyeTexMSAA);

    if (glIsTexture(m_depthTexMSAA))
        glDeleteTextures(1, &m_depthTexMSAA);

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

bool OculusVR::InitNonDistortMirror(int windowWidth, int windowHeight)
{
    LOG_MESSAGE_ASSERT(!glIsFramebuffer(m_nonDistortFBO), "Non-distort mirror FBO already initialized!");

    // we render per-eye only, so take only half of the target window width
    windowWidth /= 2;
    m_nonDistortViewPortWidth  = windowWidth;
    m_nonDistortViewPortHeight = windowHeight;

    // Configure non-distorted frame buffer
    glGenTextures(1, &m_nonDistortTexture);
    glBindTexture(GL_TEXTURE_2D, m_nonDistortTexture);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, windowWidth, windowHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);

    // create depth buffer
    glGenTextures(1, &m_nonDistortDepthBuffer);
    glBindTexture(GL_TEXTURE_2D, m_nonDistortDepthBuffer);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    GLenum internalFormat = GL_DEPTH_COMPONENT24;
    GLenum type = GL_UNSIGNED_INT;

    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, windowWidth, windowHeight, 0, GL_DEPTH_COMPONENT, type, NULL);

    // create FBO for non-disortion mirror
    glGenFramebuffers(1, &m_nonDistortFBO);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, m_nonDistortFBO);
    glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_nonDistortTexture, 0);
    glFramebufferRenderbuffer(GL_READ_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, 0);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        glDeleteFramebuffers(1, &m_nonDistortFBO);
        LOG_MESSAGE_ASSERT(false, "Could not initialize non-distorted mirror buffers!");
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

        m_debugData     = nullptr;
        m_cameraFrustum = nullptr;

        if (glIsFramebuffer(m_mirrorFBO))
            glDeleteFramebuffers(1, &m_mirrorFBO);

        if (glIsFramebuffer(m_nonDistortFBO))
            glDeleteFramebuffers(1, &m_nonDistortFBO);

        if (glIsTexture(m_nonDistortTexture))
            glDeleteTextures(1, &m_nonDistortTexture);

        if (glIsTexture(m_nonDistortDepthBuffer))
            glDeleteTextures(1, &m_nonDistortDepthBuffer);

        ovrHmd_DestroyMirrorTexture(m_hmd, (ovrTexture*)m_mirrorTexture);

        for (int eyeIdx = 0; eyeIdx < ovrEye_Count; eyeIdx++)
        {
            m_eyeBuffers[eyeIdx]->Destroy(m_hmd);
            delete m_eyeBuffers[eyeIdx];
            m_eyeBuffers[eyeIdx] = nullptr;
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


const OVR::Matrix4f OculusVR::OnEyeRender(int eyeIndex)
{
    int eye = m_hmd->EyeRenderOrder[eyeIndex];

    if (m_msaaEnabled)
        m_eyeBuffers[eye]->OnRenderMSAA();
    else
        m_eyeBuffers[eye]->OnRender();

    m_projectionMatrix[eye] = OVR::Matrix4f(ovrMatrix4f_Projection(m_eyeRenderDesc[eye].Fov, 0.01f, 10000.0f, ovrProjection_RightHanded));
    m_eyeViewOffset[eye]    = OVR::Matrix4f::Translation(m_hmdToEyeViewOffset[eye]);
    m_eyeOrientation[eye]   = OVR::Matrix4f(OVR::Quatf(m_eyeRenderPose[eye].Orientation).Inverted());
    m_eyePose[eye]          = OVR::Matrix4f::Translation(-OVR::Vector3f(m_eyeRenderPose[eye].Position));

    return m_projectionMatrix[eye] * m_eyeViewOffset[eye] * m_eyeOrientation[eye] * m_eyePose[eye];
}

void OculusVR::OnEyeRenderFinish(int eyeIndex)
{
    if (m_msaaEnabled)
        m_eyeBuffers[m_hmd->EyeRenderOrder[eyeIndex]]->OnRenderMSAAFinish();
    else
        m_eyeBuffers[m_hmd->EyeRenderOrder[eyeIndex]]->OnRenderFinish();
}

const OVR::Matrix4f OculusVR::GetEyeMVPMatrix(int eyeIndex) const
{
    return m_projectionMatrix[eyeIndex] * m_eyeViewOffset[eyeIndex] * m_eyeOrientation[eyeIndex] * m_eyePose[eyeIndex];
}

void OculusVR::SubmitFrame()
{
    // set up positional data
    ovrViewScaleDesc viewScaleDesc;
    viewScaleDesc.HmdSpaceToWorldScaleInMeters = 1.0f;
    viewScaleDesc.HmdToEyeViewOffset[0] = m_hmdToEyeViewOffset[0];
    viewScaleDesc.HmdToEyeViewOffset[1] = m_hmdToEyeViewOffset[1];

    // create the main eye layer
    ovrLayerEyeFov eyeLayer;
    eyeLayer.Header.Type = ovrLayerType_EyeFov;
    eyeLayer.Header.Flags = ovrLayerFlag_TextureOriginAtBottomLeft;   // Because OpenGL.

    for (int eye = 0; eye < ovrEye_Count; eye++)
    {
        eyeLayer.ColorTexture[eye] = m_eyeBuffers[eye]->m_swapTextureSet;
        eyeLayer.Viewport[eye] = OVR::Recti(m_eyeBuffers[eye]->m_eyeTextureSize);
        eyeLayer.Fov[eye] = m_hmd->DefaultEyeFov[eye];
        eyeLayer.RenderPose[eye] = m_eyeRenderPose[eye];
    }

    // append all the layers to global list
    ovrLayerHeader* layerList[1];
    layerList[0] = &eyeLayer.Header;

    ovrResult result = ovrHmd_SubmitFrame(m_hmd, 0, &viewScaleDesc, layerList, 1);
}

void OculusVR::BlitMirror(ovrEyeType numEyes, int offset)
{
    // Blit mirror texture to back buffer
    glBindFramebuffer(GL_READ_FRAMEBUFFER, m_mirrorFBO);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    GLint w = m_mirrorTexture->OGL.Header.TextureSize.w;
    GLint h = m_mirrorTexture->OGL.Header.TextureSize.h;

    switch (numEyes)
    {
    case ovrEye_Count:
        glBlitFramebuffer(0, h, w, 0, 0, 0, w, h, GL_COLOR_BUFFER_BIT, GL_NEAREST);
        break;
    case ovrEye_Left:
        glBlitFramebuffer(0, h, w / 2, 0, offset, 0, w / 2 + offset, h, GL_COLOR_BUFFER_BIT, GL_NEAREST);
        break;
    case ovrEye_Right:
        glBlitFramebuffer(w / 2, h, w, 0, offset, 0, w / 2 + offset, h, GL_COLOR_BUFFER_BIT, GL_NEAREST);
        break;
    default:
        LOG_MESSAGE_ASSERT(false, "Unrecognized ovrEyeType");
    }

    glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
}

void OculusVR::OnNonDistortMirrorStart()
{
    LOG_MESSAGE_ASSERT(glIsFramebuffer(m_nonDistortFBO), "Non-distort mirror FBO not initialized!");

    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_nonDistortFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_nonDistortTexture, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_nonDistortDepthBuffer, 0);

    glViewport(0, 0, m_nonDistortViewPortWidth, m_nonDistortViewPortHeight);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void OculusVR::BlitNonDistortMirror(int offset)
{
    LOG_MESSAGE_ASSERT(glIsFramebuffer(m_nonDistortFBO), "Non-distort mirror FBO not initialized!");

    // Blit non distorted mirror to backbuffer
    glBindFramebuffer(GL_READ_FRAMEBUFFER, m_nonDistortFBO);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    GLint dstX = 0 + offset;
    GLint dstW = m_nonDistortViewPortWidth + offset;
    glBlitFramebuffer(0, 0, m_nonDistortViewPortWidth, m_nonDistortViewPortHeight, dstX, 0, dstW, m_nonDistortViewPortHeight, GL_COLOR_BUFFER_BIT, GL_NEAREST);
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

void OculusVR::ShowPerfStats(ovrPerfHudMode statsMode)
{
    if (!IsDebugHMD())
    {
        ovrHmd_SetInt(m_hmd, "PerfHudMode", (int)statsMode);
    }
    else
    {
        LOG_MESSAGE("Performance Hud not available for debug HMD.");
    }
}