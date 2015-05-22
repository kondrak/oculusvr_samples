#ifndef OCULUSVRDEBUG_HPP
#define OCULUSVRDEBUG_HPP
#include "OVR_CAPI_0_6_0.h"

class Font;

class OculusVRDebug
{
public:
    OculusVRDebug();
    ~OculusVRDebug();

    void OnUpdate(const ovrTrackingState &trackingState);
    void OnRender(const ovrHmd hmd, const ovrTrackingState &trackingState, const ovrEyeRenderDesc *eyeRenderDescs, const ovrSizei &eyeTextureSize);

private:   
    void UpdateFrameRateCounter(double curtime);
    void FormatLatencyReading(char* buff, size_t size, float val);

    // Frame timing logic.
    int    m_frameCounter;
    int	   m_totalFrameCounter;
    float  m_secondsPerFrame;
    float  m_fps;
    double m_lastFpsUpdate;

    Font *m_font;
};

#endif