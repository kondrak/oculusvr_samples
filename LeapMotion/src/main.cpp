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
    if( SDL_Init( SDL_INIT_VIDEO | SDL_INIT_EVENTS ) < 0 ) 
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

    g_renderContext.Init("Oculus Rift Leap Motion integration test", 100, 100, windowSize.w, windowSize.h);
    SDL_ShowCursor(SDL_DISABLE);

    if (glewInit() != GLEW_OK)
    {
        g_renderContext.Destroy();
        g_oculusVR.DestroyVR();
        SDL_Quit();
        return 1;
    }

    if (!g_oculusVR.InitVRBuffers(windowSize.w, windowSize.h))
    {
        g_renderContext.Destroy();
        g_oculusVR.DestroyVR();
        SDL_Quit();
        return 1;
    }

    ShaderManager::GetInstance()->LoadShaders();
    g_application.OnStart();

    while (g_application.Running())
    {
        // handle key presses
        processEvents();

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glClearColor(0.2f, 0.2f, 0.6f, 0.0f);

        g_oculusVR.OnRenderStart();

        for (int eyeIndex = 0; eyeIndex < ovrEye_Count; eyeIndex++)
        {
            OVR::Matrix4f MVPMatrix = g_oculusVR.OnEyeRender(eyeIndex);

            // update MVP in quad shader
            const ShaderProgram &shader = ShaderManager::GetInstance()->UseShaderProgram(ShaderManager::BasicShader);
            glUniformMatrix4fv(shader.uniforms[ModelViewProjectionMatrix], 1, GL_FALSE, &MVPMatrix.Transposed().M[0][0]);

            g_application.OnRender();    
            g_oculusVR.OnEyeRenderFinish(eyeIndex);
        }

        g_oculusVR.SubmitFrame();
        g_oculusVR.BlitMirror();
        SDL_GL_SwapWindow(g_renderContext.window);
    }

    g_renderContext.Destroy();
    g_oculusVR.DestroyVR();

    SDL_Quit(); 

    return 0;
}