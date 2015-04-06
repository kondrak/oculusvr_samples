#include "renderer/OculusVRDebug.hpp"
#include "renderer/CameraDirector.hpp"
#include "renderer/Font.hpp"
#include "renderer/OculusVR.hpp"
#include "renderer/RenderContext.hpp"
#include "Kernel/OVR_Std.h"
#include <sstream>

extern RenderContext  g_renderContext;
extern CameraDirector g_cameraDirector;

OculusVRDebug::OculusVRDebug() : m_frameCounter(0),
                                 m_totalFrameCounter(0),
                                 m_secondsPerFrame(0.0f),
                                 m_fps(0.0f),
                                 m_lastFpsUpdate(0.0f)
{
    m_font = new Font("res/font.png");
    m_font->SetScale(Math::Vector2f(3.f * (float)g_renderContext.height / 1080.f, 3.f * (float)g_renderContext.height / 1080.f));
}

OculusVRDebug::~OculusVRDebug()
{
    delete m_font;
}

void OculusVRDebug::OnUpdate(const ovrTrackingState &trackingState)
{
    UpdateFrameRateCounter(ovr_GetTimeInSeconds());
}

void OculusVRDebug::OnRender(const ovrHmd hmd, const ovrTrackingState &trackingState, const ovrEyeRenderDesc *eyeRenderDescs, const ovrTexture *eyeTextures)
{
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glActiveTexture(GL_TEXTURE0);

    static const float xPos     = -0.3f;
    static const float ySpacing = 0.05f;
    std::stringstream statsStream;

    char buf[128];
    float hmdYaw, hmdPitch, hmdRoll;
    OVR::Quatf headOrientation(trackingState.HeadPose.ThePose.Orientation);
    headOrientation.GetEulerAngles<OVR::Axis_Y, OVR::Axis_X, OVR::Axis_Z>(&hmdYaw, &hmdPitch, &hmdRoll);

    OVR::OVR_sprintf(buf, sizeof(buf), "HMD YPR:%2.0f %2.0f %2.0f  HMD: %s", hmdYaw * PIdiv180inv, hmdPitch * PIdiv180inv, hmdRoll * PIdiv180inv, hmd->ProductName);
    m_font->drawText(buf, xPos, 0.1f, 0.f);

    OVR::OVR_sprintf(buf, sizeof(buf), "FPS: %.1f  ms/frame: %.1f  Frame: %03d %d", m_fps, m_secondsPerFrame * 1000.0f, m_frameCounter, m_totalFrameCounter % 2);
    m_font->drawText(buf, xPos, 0.1f - ySpacing, 0.f);

    OVR::OVR_sprintf(buf, sizeof(buf), "Pos: %2.2f %2.2f %2.2f  Tracking: %s", g_cameraDirector.GetActiveCamera()->Position().m_x, 
                                                                 g_cameraDirector.GetActiveCamera()->Position().m_y,
                                                                 g_cameraDirector.GetActiveCamera()->Position().m_z,
                                                                 trackingState.StatusFlags & ovrStatus_PositionTracked ? "YES" : "NO");
    m_font->drawText(buf, xPos, 0.1f - ySpacing * 2.f, 0.f);


    OVR::OVR_sprintf(buf, sizeof(buf), "EyeHeight: %2.2f IPD: %2.1fmm", ovrHmd_GetFloat(hmd, OVR_KEY_EYE_HEIGHT, 0.f), ovrHmd_GetFloat(hmd, OVR_KEY_IPD, 0.f) * 1000.f);
    m_font->drawText(buf, xPos, 0.1f - ySpacing * 3.f, 0.f);

    // Average FOVs.
    OVR::FovPort leftFov  = eyeRenderDescs[0].Fov;
    OVR::FovPort rightFov = eyeRenderDescs[1].Fov;

    // Rendered size changes based on selected options & dynamic rendering.
    int pixelSizeWidth  =  eyeTextures[0].Header.RenderViewport.Size.w + eyeTextures[1].Header.RenderViewport.Size.w;
    int pixelSizeHeight = (eyeTextures[0].Header.RenderViewport.Size.h + eyeTextures[1].Header.RenderViewport.Size.h) / 2;

    OVR::OVR_sprintf(buf, sizeof(buf), "FOV %2.1fx%2.1f, Resolution: %ix%i", (leftFov.GetHorizontalFovDegrees() + rightFov.GetHorizontalFovDegrees()) * 0.5f,
                                                                             (leftFov.GetVerticalFovDegrees() + rightFov.GetVerticalFovDegrees()) * 0.5,
                                                                             pixelSizeWidth, pixelSizeHeight);
    m_font->drawText(buf, xPos, 0.1f - ySpacing * 4.f, 0.f);

    // latency readings
    float latencies[5] = {};
    if (ovrHmd_GetFloatArray(hmd, "DK2Latency", latencies, 5) == 5)
    {
        char text[5][32];
        for (int i = 0; i < 5; ++i)
        {
            FormatLatencyReading(text[i], sizeof(text[i]), latencies[i]);
        }

        statsStream.str("");
        statsStream << "M2P Latency  Ren: " << text[0] << " TWrp: " << text[1];
        m_font->drawText(statsStream.str(), xPos, 0.1f - ySpacing * 5.f, 0.f);

        statsStream.str("");
        statsStream << "PostPresent: " << text[2] << " Err: " << text[3] << " " << text[4];
        m_font->drawText(statsStream.str(), xPos, 0.1f - ySpacing * 6.f, 0.f);
    }
}


void OculusVRDebug::UpdateFrameRateCounter(double curtime)
{
    m_frameCounter++;
    m_totalFrameCounter++;
    float secondsSinceLastMeasurement = (float)(curtime - m_lastFpsUpdate);

    if (secondsSinceLastMeasurement >= 1.f)
    {
        m_secondsPerFrame = (float)(curtime - m_lastFpsUpdate) / (float)m_frameCounter;
        m_fps = 1.0f / m_secondsPerFrame;
        m_lastFpsUpdate = curtime;
        m_frameCounter = 0;
    }
}

void OculusVRDebug::FormatLatencyReading(char* buff, size_t size, float val)
{
    if (val < 0.000001f)
        OVR::OVR_strcpy(buff, size, "N/A   ");
    else
        OVR::OVR_sprintf(buff, size, "%4.2fms", val * 1000.0f);
}