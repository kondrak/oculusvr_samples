#include "Application.hpp"
#include "InputHandlers.hpp"
#include "renderer/RenderContext.hpp"
#include "renderer/ShaderManager.hpp"
#include "renderer/OculusVR.hpp"

// application globals
RenderContext g_renderContext;
Application   g_application;
OculusVR      g_oculusVR;


int main(int argc, char **argv)
{
    // initialize everything
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) < 0)
    {
        return 1;
    }

    if (!g_oculusVR.InitVR())
    {
        SDL_Quit();
        return 1;
    }

    ovrSizei hmdResolution = g_oculusVR.GetResolution();
    ovrSizei windowSize = { hmdResolution.w / 2, hmdResolution.h / 2 };

    g_renderContext.Init("Oculus Rift Vive-style tracker chaperone", 100, 100, windowSize.w, windowSize.h);
    SDL_ShowCursor(SDL_DISABLE);

    if (glewInit() != GLEW_OK)
    {
        g_oculusVR.DestroyVR();
        g_renderContext.Destroy();
        SDL_Quit();
        return 1;
    }

    if (!g_oculusVR.InitVRBuffers(windowSize.w, windowSize.h))
    {
        g_oculusVR.DestroyVR();
        g_renderContext.Destroy();
        SDL_Quit();
        return 1;
    }

    ShaderManager::GetInstance()->LoadShaders();
    g_application.OnStart();

    while (g_application.Running())
    {
        // handle key presses
        processEvents();

        glClearColor(0.2f, 0.2f, 0.6f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        g_oculusVR.OnRenderStart();

        for (int eyeIndex = 0; eyeIndex < ovrEye_Count; eyeIndex++)
        {
            OVR::Matrix4f MVPMatrix = g_oculusVR.OnEyeRender(eyeIndex);

            // update MVP in both shaders
            const ShaderProgram &shader = ShaderManager::GetInstance()->UseShaderProgram(ShaderManager::BasicShader);
            glUniformMatrix4fv(shader.uniforms[ModelViewProjectionMatrix], 1, GL_FALSE, &MVPMatrix.Transposed().M[0][0]);
            const ShaderProgram &shader2 = ShaderManager::GetInstance()->UseShaderProgram(ShaderManager::OVRFrustumShader);
            glUniformMatrix4fv(shader2.uniforms[ModelViewProjectionMatrix], 1, GL_FALSE, &MVPMatrix.Transposed().M[0][0]);

            g_application.OnRender();
            g_oculusVR.RenderTrackerChaperone();
            g_oculusVR.OnEyeRenderFinish(eyeIndex);
        }

        g_oculusVR.SubmitFrame();
        g_oculusVR.BlitMirror();
        SDL_GL_SwapWindow(g_renderContext.window);
    }

    g_oculusVR.DestroyVR();
    g_renderContext.Destroy();

    SDL_Quit(); 

    return 0;
}