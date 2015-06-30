#include "Application.hpp"
#include "InputHandlers.hpp"
#include "renderer/RenderContext.hpp"
#include "renderer/ShaderManager.hpp"
#include "renderer/OculusVR.hpp"
#include "renderer/CameraDirector.hpp"

// application globals
extern CameraDirector g_cameraDirector;
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

    g_renderContext.Init("Oculus Rift Minimum OpenGL", 100, 100, windowSize.w, windowSize.h);
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

    if (!g_oculusVR.InitNonDistortMirror(windowSize.w, windowSize.h))
    {
        g_renderContext.Destroy();
        g_oculusVR.DestroyVR();
        SDL_Quit();
        return 1;
    }

    ShaderManager::GetInstance()->LoadShaders();
    g_application.OnStart();

     glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
     glClearColor(0.2f, 0.2f, 0.6f, 0.0f);

    while (g_application.Running())
    {
        // handle key presses
        processEvents();
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

        Application::VRMirrorMode mirrorMode = g_application.CurrMirrorMode();

        if (mirrorMode == Application::Mirror_Regular)
        {
            g_oculusVR.BlitMirror();
        }
       
        if (mirrorMode == Application::Mirror_NonDistort)
        {
            // non distorted, dual view
            for (int eyeIndex = 0; eyeIndex < ovrEye_Count; eyeIndex++)
            {
                const OVR::Matrix4f MVPMatrix = g_oculusVR.GetEyeMVPMatrix(eyeIndex);
                const ShaderProgram &shader = ShaderManager::GetInstance()->UseShaderProgram(ShaderManager::BasicShader);
                glUniformMatrix4fv(shader.uniforms[ModelViewProjectionMatrix], 1, GL_FALSE, &MVPMatrix.Transposed().M[0][0]);

                g_oculusVR.BlitStart(windowSize.w / 2, windowSize.h);
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
                glClearColor(0.2f, 0.2f, 0.6f, 0.0f);

                g_application.OnRender();

                g_oculusVR.BlitNonDistort(windowSize.w / 2, windowSize.h, eyeIndex == 0 ? 0 : windowSize.w / 2);
            }
            // non distort end
        }

        if (mirrorMode == Application::Mirror_LeftEye)
        {
            // non distorted, centered
            const OVR::Matrix4f MVPMatrix = g_oculusVR.GetEyeMVPMatrix(ovrEye_Left);
            const ShaderProgram &shader = ShaderManager::GetInstance()->UseShaderProgram(ShaderManager::BasicShader);
            glUniformMatrix4fv(shader.uniforms[ModelViewProjectionMatrix], 1, GL_FALSE, &MVPMatrix.Transposed().M[0][0]);

            g_oculusVR.BlitStart(windowSize.w / 2, windowSize.h);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            glClearColor(0.2f, 0.2f, 0.6f, 0.0f);

            g_application.OnRender();
            g_oculusVR.BlitNonDistort(windowSize.w / 2, windowSize.h, windowSize.w / 4);
            // non distorted end
        }

        if (mirrorMode == Application::Mirror_RightEye)
        {
            // non distorted, centered
            const OVR::Matrix4f MVPMatrix = g_oculusVR.GetEyeMVPMatrix(ovrEye_Right);
            const ShaderProgram &shader = ShaderManager::GetInstance()->UseShaderProgram(ShaderManager::BasicShader);
            glUniformMatrix4fv(shader.uniforms[ModelViewProjectionMatrix], 1, GL_FALSE, &MVPMatrix.Transposed().M[0][0]);

            g_oculusVR.BlitStart(windowSize.w / 2, windowSize.h);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            glClearColor(0.2f, 0.2f, 0.6f, 0.0f);

            g_application.OnRender();
            g_oculusVR.BlitNonDistort(windowSize.w / 2, windowSize.h, windowSize.w / 4);
            // non distorted end
        }

        SDL_GL_SwapWindow(g_renderContext.window);

    }

    g_renderContext.Destroy();
    g_oculusVR.DestroyVR();

    SDL_Quit(); 

    return 0;
}