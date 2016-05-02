#include "renderer/OculusVR.hpp"
#include "renderer/ShaderManager.hpp"
//#include <GL/CAPI_GLE.h>

OculusVR::OVRBuffer::OVRBuffer(const ovrSession &session, int eyeIdx)
{
    ovrHmdDesc hmdDesc = ovr_GetHmdDesc(session);
    m_eyeTextureSize   = ovr_GetFovTextureSize(session, (ovrEyeType)eyeIdx, hmdDesc.DefaultEyeFov[eyeIdx], 1.0f);

    ovrTextureSwapChainDesc desc = {};
    desc.Type = ovrTexture_2D;
    desc.ArraySize = 1;
    desc.Width = m_eyeTextureSize.w;
    desc.Height = m_eyeTextureSize.h;
    desc.MipLevels = 1;
    desc.Format = OVR_FORMAT_R8G8B8A8_UNORM_SRGB;
    desc.SampleCount = 1;
    desc.StaticImage = ovrFalse;

    ovrResult result = ovr_CreateTextureSwapChainGL(session, &desc, &m_swapTextureChain);

    int textureCount = 0;
    ovr_GetTextureSwapChainLength(session, m_swapTextureChain, &textureCount);

    for (int j = 0; j < textureCount; ++j)
    {
        GLuint chainTexId;
        ovr_GetTextureSwapChainBufferGL(session, m_swapTextureChain, j, &chainTexId);
        glBindTexture(GL_TEXTURE_2D, chainTexId);

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

    // MSAA color texture and fbo setup
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
    glBindFramebuffer(GL_FRAMEBUFFER, m_msaaEyeFbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, m_eyeTexMSAA, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D_MULTISAMPLE, m_depthTexMSAA, 0);

    glViewport(0, 0, m_eyeTextureSize.w, m_eyeTextureSize.h);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void OculusVR::OVRBuffer::OnRenderMSAAFinish()
{
    // blit the contents of MSAA FBO to the regular eye buffer "connected" to the HMD
    glBindFramebuffer(GL_READ_FRAMEBUFFER, m_msaaEyeFbo);
    glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D_MULTISAMPLE, m_eyeTexMSAA, 0);
    glFramebufferRenderbuffer(GL_READ_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, 0);

    LOG_MESSAGE_ASSERT((glCheckFramebufferStatus(GL_READ_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE), "Could not complete framebuffer operation");

    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_eyeFbo);
    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_eyeTexId, 0);
    glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, 0);

    LOG_MESSAGE_ASSERT((glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE), "Could not complete framebuffer operation");

    glBlitFramebuffer(0, 0, m_eyeTextureSize.w, m_eyeTextureSize.h,
                      0, 0, m_eyeTextureSize.w, m_eyeTextureSize.h, GL_COLOR_BUFFER_BIT, GL_NEAREST);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}


void OculusVR::OVRBuffer::OnRender()
{
    // Switch to eye render target
    glBindFramebuffer(GL_FRAMEBUFFER, m_eyeFbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_eyeTexId, 0);
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

void OculusVR::OVRBuffer::Destroy(const ovrSession &session)
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

    ovr_DestroyTextureSwapChain(session, m_swapTextureChain);
}

OculusVR::~OculusVR()
{
    ovr_Destroy(m_hmdSession);
    ovr_Shutdown();
    m_hmdSession = nullptr;
}

bool OculusVR::InitVR()
{
    ovrResult result = ovr_Initialize(nullptr);
    ovrGraphicsLuid luid; // as of SDK 0.7.0.0 luid is not supported with OpenGL

    if (result != ovrSuccess)
    {
        LOG_MESSAGE_ASSERT(false, "Failed to initialize LibOVR");
        return false;
    }

    result = ovr_Create(&m_hmdSession, &luid);

    if (result != ovrSuccess)
    {
        LOG_MESSAGE_ASSERT(result == ovrSuccess, "Failed to create OVR device");
    }

    m_hmdDesc = ovr_GetHmdDesc(m_hmdSession);

    // debug camera tracking frustum
    m_cameraFrustum    = new OVRCameraFrustum;
    // debug Vive-style camera tracking chaperone
    m_trackerChaperone = new OVRTrackerChaperone;

    return result == ovrSuccess;
}

bool OculusVR::InitVRBuffers(int windowWidth, int windowHeight)
{
    for (int eyeIdx = 0; eyeIdx < ovrEye_Count; eyeIdx++)
    {
        m_eyeBuffers[eyeIdx]    = new OVRBuffer(m_hmdSession, eyeIdx);
        m_eyeRenderDesc[eyeIdx] = ovr_GetRenderDesc(m_hmdSession, (ovrEyeType)eyeIdx, m_hmdDesc.DefaultEyeFov[eyeIdx]);
    }

    memset(&m_mirrorDesc, 0, sizeof(m_mirrorDesc));
    m_mirrorDesc.Width  = windowWidth;
    m_mirrorDesc.Height = windowHeight;
    m_mirrorDesc.Format = OVR_FORMAT_R8G8B8A8_UNORM_SRGB;

    // since SDK 0.6 we're using a mirror texture + FBO which in turn copies contents of mirror to back buffer
    ovr_CreateMirrorTextureGL(m_hmdSession, &m_mirrorDesc, &m_mirrorTexture);

    // Configure the mirror read buffer
    GLuint texId;
    ovr_GetMirrorTextureBufferGL(m_hmdSession, m_mirrorTexture, &texId);
    glGenFramebuffers(1, &m_mirrorFBO);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, m_mirrorFBO);
    glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texId, 0);
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
    if (m_hmdSession)
    {
        delete m_debugData;
        delete m_cameraFrustum;
        delete m_trackerChaperone;

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

        ovr_DestroyMirrorTexture(m_hmdSession, m_mirrorTexture);

        for (int eyeIdx = 0; eyeIdx < ovrEye_Count; eyeIdx++)
        {
            m_eyeBuffers[eyeIdx]->Destroy(m_hmdSession);
            delete m_eyeBuffers[eyeIdx];
            m_eyeBuffers[eyeIdx] = nullptr;
        }
    }
}

const ovrSizei OculusVR::GetResolution() const
{
    ovrSizei resolution = { m_hmdDesc.Resolution.w, m_hmdDesc.Resolution.h };
    return resolution;
}

void OculusVR::OnRenderStart()
{
    m_hmdToEyeOffset[0] = m_eyeRenderDesc[0].HmdToEyeOffset;
    m_hmdToEyeOffset[1] = m_eyeRenderDesc[1].HmdToEyeOffset;

    // this data is fetched only for the debug display, no need to do this to just get the rendering work
    m_frameTiming   = ovr_GetPredictedDisplayTime(m_hmdSession, 0);
    m_trackingState = ovr_GetTrackingState(m_hmdSession, m_frameTiming, ovrTrue);

    // Get both eye poses simultaneously, with IPD offset already included.
    ovr_GetEyePoses(m_hmdSession, m_frameIndex, ovrTrue, m_hmdToEyeOffset, m_eyeRenderPose, &m_sensorSampleTime);    
}


const OVR::Matrix4f OculusVR::OnEyeRender(int eyeIndex)
{
    // set the current eye texture in swap chain
    int curIndex;
    ovr_GetTextureSwapChainCurrentIndex(m_hmdSession, m_eyeBuffers[eyeIndex]->m_swapTextureChain, &curIndex);
    ovr_GetTextureSwapChainBufferGL(m_hmdSession, m_eyeBuffers[eyeIndex]->m_swapTextureChain, curIndex, &m_eyeBuffers[eyeIndex]->m_eyeTexId);

    if (m_msaaEnabled)
        m_eyeBuffers[eyeIndex]->OnRenderMSAA();
    else
        m_eyeBuffers[eyeIndex]->OnRender();

    m_projectionMatrix[eyeIndex] = OVR::Matrix4f(ovrMatrix4f_Projection(m_eyeRenderDesc[eyeIndex].Fov, 0.01f, 10000.0f, ovrProjection_None));
    m_eyeOrientation[eyeIndex] = OVR::Matrix4f(OVR::Quatf(m_eyeRenderPose[eyeIndex].Orientation).Inverted());
    m_eyePose[eyeIndex]        = OVR::Matrix4f::Translation(-OVR::Vector3f(m_eyeRenderPose[eyeIndex].Position));

    return m_projectionMatrix[eyeIndex] * m_eyeOrientation[eyeIndex] * m_eyePose[eyeIndex];
}

void OculusVR::OnEyeRenderFinish(int eyeIndex)
{
    if (m_msaaEnabled)
        m_eyeBuffers[eyeIndex]->OnRenderMSAAFinish();
    else
        m_eyeBuffers[eyeIndex]->OnRenderFinish();

    ovr_CommitTextureSwapChain(m_hmdSession, m_eyeBuffers[eyeIndex]->m_swapTextureChain);
}

const OVR::Matrix4f OculusVR::GetEyeMVPMatrix(int eyeIndex) const
{
    return m_projectionMatrix[eyeIndex] * m_eyeOrientation[eyeIndex] * m_eyePose[eyeIndex];
}

void OculusVR::SubmitFrame()
{
    // set up positional data
    ovrViewScaleDesc viewScaleDesc;
    viewScaleDesc.HmdSpaceToWorldScaleInMeters = 1.0f;
    viewScaleDesc.HmdToEyeOffset[0] = m_hmdToEyeOffset[0];
    viewScaleDesc.HmdToEyeOffset[1] = m_hmdToEyeOffset[1];

    // create the main eye layer
    ovrLayerEyeFov eyeLayer;
    eyeLayer.Header.Type  = ovrLayerType_EyeFov;
    eyeLayer.Header.Flags = ovrLayerFlag_TextureOriginAtBottomLeft;   // Because OpenGL.

    for (int eye = 0; eye < ovrEye_Count; eye++)
    {
        eyeLayer.ColorTexture[eye] = m_eyeBuffers[eye]->m_swapTextureChain;
        eyeLayer.Viewport[eye]     = OVR::Recti(m_eyeBuffers[eye]->m_eyeTextureSize);
        eyeLayer.Fov[eye]          = m_hmdDesc.DefaultEyeFov[eye];
        eyeLayer.RenderPose[eye]   = m_eyeRenderPose[eye];
        eyeLayer.SensorSampleTime  = m_sensorSampleTime;
    }

    // append all the layers to global list
    ovrLayerHeader* layerList = &eyeLayer.Header;

    ovrResult result = ovr_SubmitFrame(m_hmdSession, m_frameIndex, nullptr, &layerList, 1);
}

void OculusVR::BlitMirror(ovrEyeType numEyes, int offset)
{
    // Blit mirror texture to back buffer
    glBindFramebuffer(GL_READ_FRAMEBUFFER, m_mirrorFBO);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    GLint w = m_mirrorDesc.Width;
    GLint h = m_mirrorDesc.Height;

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
        ovr_RecenterTrackingOrigin(m_hmdSession);
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
    m_debugData->OnRender(m_hmdSession, m_trackingState, m_eyeRenderDesc, texSize);
}

void OculusVR::RenderTrackerFrustum()
{
    if (!IsDebugHMD() && m_cameraFrustum)
    {
        m_cameraFrustum->Recalculate(m_hmdSession);
        m_cameraFrustum->OnRender();
    }
}

void OculusVR::RenderTrackerChaperone()
{
    if (!IsDebugHMD() && m_trackerChaperone)
    {
        m_trackerChaperone->Recalculate(m_hmdSession, m_trackingState.HeadPose.ThePose.Position);
        m_trackerChaperone->OnRender();
    }
}

void OculusVR::ShowPerfStats(ovrPerfHudMode statsMode)
{
    if (!IsDebugHMD())
    {
        ovr_SetInt(m_hmdSession, "PerfHudMode", (int)statsMode);
    }
    else
    {
        LOG_MESSAGE("Performance Hud not available for debug HMD.");
    }
}