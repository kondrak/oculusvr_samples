#include "renderer/OculusVR.hpp"
#include "renderer/ShaderManager.hpp"

void TrackerFrustum::Recalculate(ovrHmd hmd)
{
    ovrTrackingState tState = ovrHmd_GetTrackingState(hmd, 0.0f);

    ovrVector3f trackerPose = tState.CameraPose.Position;

    float trackerFar  = hmd->CameraFrustumFarZInMeters;
    float trackerNear = hmd->CameraFrustumNearZInMeters;
    float trackerHFov = hmd->CameraFrustumHFovInRadians;
    float trackerVFov = hmd->CameraFrustumVFovInRadians;

    float hScale = tanf(trackerHFov / 2.f);
    float vScale = tanf(trackerVFov / 2.f);

    // camera orientation quaternion
    OVR::Quatf trackerOrientationQuat(tState.CameraPose.Orientation.x, 
                                      tState.CameraPose.Orientation.y, 
                                      tState.CameraPose.Orientation.z, 
                                      tState.CameraPose.Orientation.w);

    // orientation indicator vector running from camera pose to near plane
    OVR::Vector3f trackerOrientationVec(0.f, 0.f, trackerNear);

    // near plane vertex positions
    OVR::Vector3f nearV1(-hScale * trackerNear,  vScale * trackerNear, trackerNear);
    OVR::Vector3f nearV2(-hScale * trackerNear, -vScale * trackerNear, trackerNear);
    OVR::Vector3f nearV3( hScale * trackerNear, -vScale * trackerNear, trackerNear);
    OVR::Vector3f nearV4( hScale * trackerNear,  vScale * trackerNear, trackerNear);

    // far plane vertex positions
    OVR::Vector3f farV1(-hScale * trackerFar,  vScale * trackerFar, trackerFar);
    OVR::Vector3f farV2(-hScale * trackerFar, -vScale * trackerFar, trackerFar);
    OVR::Vector3f farV3( hScale * trackerFar, -vScale * trackerFar, trackerFar);
    OVR::Vector3f farV4( hScale * trackerFar,  vScale * trackerFar, trackerFar);

    // reorient all vectors by current tracker camera orientation
    trackerOrientationVec = trackerOrientationQuat.Rotate(trackerOrientationVec);

    nearV1 = trackerOrientationQuat.Rotate(nearV1);
    nearV2 = trackerOrientationQuat.Rotate(nearV2);
    nearV3 = trackerOrientationQuat.Rotate(nearV3);
    nearV4 = trackerOrientationQuat.Rotate(nearV4);

    farV1 = trackerOrientationQuat.Rotate(farV1);
    farV2 = trackerOrientationQuat.Rotate(farV2);
    farV3 = trackerOrientationQuat.Rotate(farV3);
    farV4 = trackerOrientationQuat.Rotate(farV4);

    OVR::Vector3f orientationVector(trackerPose.x + trackerOrientationVec.x, 
                                    trackerPose.y + trackerOrientationVec.y, 
                                    trackerPose.z + trackerOrientationVec.z);
    // tracker camera frustum
    const GLfloat frustumVertexData[] = {
        trackerPose.x, trackerPose.y, trackerPose.z,
        trackerPose.x + farV1.x, trackerPose.y + farV1.y, trackerPose.z + farV1.z,
        trackerPose.x, trackerPose.y, trackerPose.z,
        trackerPose.x + farV2.x, trackerPose.y + farV2.y, trackerPose.z + farV2.z,
        trackerPose.x, trackerPose.y, trackerPose.z,
        trackerPose.x + farV3.x, trackerPose.y + farV3.y, trackerPose.z + farV3.z,
        trackerPose.x, trackerPose.y, trackerPose.z,
        trackerPose.x + farV4.x, trackerPose.y + farV4.y, trackerPose.z + farV4.z,
        // orientation vector (trackerPose to near plane)
        trackerPose.x, trackerPose.y, trackerPose.z,
        orientationVector.x, orientationVector.y, orientationVector.z
    };

    // near plane of the tracking camera
    const GLfloat nearPlaneVertexData[] = {
        trackerPose.x + nearV1.x, trackerPose.y + nearV1.y, trackerPose.z + nearV1.z,
        trackerPose.x + nearV2.x, trackerPose.y + nearV2.y, trackerPose.z + nearV2.z,
        trackerPose.x + nearV3.x, trackerPose.y + nearV3.y, trackerPose.z + nearV3.z,
        trackerPose.x + nearV4.x, trackerPose.y + nearV4.y, trackerPose.z + nearV4.z
    };

    // far plane of the tracking camera
    const GLfloat farPlaneVertexData[] = {
        trackerPose.x + farV1.x, trackerPose.y + farV1.y, trackerPose.z + farV1.z,
        trackerPose.x + farV2.x, trackerPose.y + farV2.y, trackerPose.z + farV2.z,
        trackerPose.x + farV3.x, trackerPose.y + farV3.y, trackerPose.z + farV3.z,
        trackerPose.x + farV4.x, trackerPose.y + farV4.y, trackerPose.z + farV4.z
    };

    if (glIsBuffer(m_vertexBuffers[0]))
        glDeleteBuffers(3, m_vertexBuffers);

    if (glIsVertexArray(m_vertexArrays[0]))
        glDeleteVertexArrays(3, m_vertexArrays);

    // create line VAO
    glGenVertexArrays(3, m_vertexArrays);
    glGenBuffers(3, m_vertexBuffers);

    glBindVertexArray(m_vertexArrays[0]);

    // Get a handle for our buffers (VBO)
    glBindBuffer(GL_ARRAY_BUFFER, m_vertexBuffers[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(frustumVertexData), frustumVertexData, GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, m_vertexBuffers[1]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(nearPlaneVertexData), nearPlaneVertexData, GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, m_vertexBuffers[2]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(farPlaneVertexData), farPlaneVertexData, GL_STATIC_DRAW);
}

void TrackerFrustum::OnRender()
{
    const ShaderProgram &shader = ShaderManager::GetInstance()->UseShaderProgram(ShaderManager::FrustumShader);
    GLuint vertexPositionAttr = glGetAttribLocation(shader.id, "inVertex");

    const float frustumColor[3]        = { 0.67f, 0.27f, 0.05f };
    const float planeColor[3]          = { 1.f, 0.f, 0.f };
    const float orientationVecColor[3] = { 0.f, 1.f, 0.f };
    

    glDisable(GL_DEPTH_TEST);
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    // render camera bounds
    glBindVertexArray(m_vertexArrays[0]);
    glEnableVertexAttribArray(vertexPositionAttr);

    glBindBuffer(GL_ARRAY_BUFFER, m_vertexBuffers[0]);
    glVertexAttribPointer(vertexPositionAttr, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

   // draw the frustum lines
    glUniform3fv(shader.uniforms[VertexColor], 1, frustumColor);
    glDrawArrays(GL_LINES, 0, 8);

    // draw the orientation vector with separate color
    glUniform3fv(shader.uniforms[VertexColor], 1, orientationVecColor);
    glDrawArrays(GL_LINES, 8, 2);

    glDisableVertexAttribArray(vertexPositionAttr);
    
    // render near and far planes
    glUniform3fv(shader.uniforms[VertexColor], 1, planeColor);

    for (int i = 1; i < 3; i++)
    {
        glBindVertexArray(m_vertexArrays[i]);
        glEnableVertexAttribArray(vertexPositionAttr);

        glBindBuffer(GL_ARRAY_BUFFER, m_vertexBuffers[i]);
        glVertexAttribPointer(vertexPositionAttr, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

        glDrawArrays(GL_LINE_LOOP, 0, 4);

        glDisableVertexAttribArray(vertexPositionAttr);
    }

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glEnable(GL_DEPTH_TEST);
}

OculusVR::~OculusVR()
{
    DestroyVR();
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

    //http://stackoverflow.com/questions/23593162/alternative-to-glframebuffertexture-for-opengl-version-3-1-and-oculus-rift
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, m_texture, 0);

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

    eyeRenderViewport[0].Pos  = OVR::Vector2i(0, 0);
    eyeRenderViewport[0].Size = OVR::Sizei(renderTargetSize.w / 2, renderTargetSize.h);
    eyeRenderViewport[1].Pos  = OVR::Vector2i((renderTargetSize.w + 1) / 2, 0);
    eyeRenderViewport[1].Size = eyeRenderViewport[0].Size;

    eyeTexture[0].OGL.Header.API = ovrRenderAPI_OpenGL;
    eyeTexture[0].OGL.Header.TextureSize = renderTargetSize;
    eyeTexture[0].OGL.Header.RenderViewport = eyeRenderViewport[0];
    eyeTexture[0].OGL.TexId = m_texture;

    eyeTexture[1] = eyeTexture[0];
    eyeTexture[1].OGL.Header.RenderViewport = eyeRenderViewport[1];

    return true;
}

void OculusVR::ConfigureRender(void *window, int w, int h)
{
    ovrGLConfig cfg;
    memset(&cfg, 0, sizeof(ovrGLConfig));

    cfg.OGL.Header.API = ovrRenderAPI_OpenGL;

    // normally it would be best to use m_hmd->Resolution.w/h here but as of SDK 0.4.4 
    // it's impossible to separate window size from framebuffer size. This applies to direct mode only.

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
    ovrHmd_ConfigureRendering(m_hmd, &cfg.Config, ovrDistortionCap_Chromatic
                                                  | ovrDistortionCap_Vignette
                                                  | ovrDistortionCap_TimeWarp
                                                  | ovrDistortionCap_Overdrive,
                                                  m_hmd->DefaultEyeFov, 
                                                  eyeRenderDesc);
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
    useHmdToEyeViewOffset[0] = eyeRenderDesc[0].HmdToEyeViewOffset;
    useHmdToEyeViewOffset[1] = eyeRenderDesc[1].HmdToEyeViewOffset;

    ovrHmd_BeginFrame(m_hmd, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, m_frameBuffer);

    // Get both eye poses simultaneously, with IPD offset already included. 
    ovrHmd_GetEyePoses(m_hmd, 0, useHmdToEyeViewOffset, eyeRenderPose, NULL);
}

const OVR::Matrix4f OculusVR::OnEyeRender(int eyeIndex) const
{
    ovrEyeType eye = m_hmd->EyeRenderOrder[eyeIndex];

    glViewport(eyeRenderViewport[eye].Pos.x, eyeRenderViewport[eye].Pos.y, eyeRenderViewport[eye].Size.w, eyeRenderViewport[eye].Size.h);

    return OVR::Matrix4f(ovrMatrix4f_Projection(eyeRenderDesc[eye].Fov, 0.01f, 10000.0f, true)) *
           OVR::Matrix4f::Translation(eyeRenderDesc[eye].HmdToEyeViewOffset) *
           OVR::Matrix4f(OVR::Quatf(eyeRenderPose[eye].Orientation).Inverted()) *
           OVR::Matrix4f::Translation(-OVR::Vector3f(eyeRenderPose[eye].Position));
}


void OculusVR::RenderTrackerFrustum()
{
    m_trackerFrustum.Recalculate(m_hmd);
    m_trackerFrustum.OnRender();
}

void OculusVR::OnRenderEnd()
{
    ovrHmd_EndFrame(m_hmd, eyeRenderPose, &eyeTexture[0].Texture);
}

void OculusVR::OnKeyPress(KeyCode key)
{
    // Dismiss the Health and Safety message by pressing any key
    ovrHmd_DismissHSWDisplay(m_hmd);

    switch (key)
    {
    case KEY_R:
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