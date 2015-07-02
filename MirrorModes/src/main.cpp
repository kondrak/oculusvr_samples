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

    g_renderContext.Init("Oculus Rift mirror modes (press M to cycle)", 100, 100, windowSize.w, windowSize.h);
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

    if (!g_oculusVR.InitNonDistortMirror(windowSize.w, windowSize.h))
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

            // update MVP in quad shader
            const ShaderProgram &shader = ShaderManager::GetInstance()->UseShaderProgram(ShaderManager::BasicShader);
            glUniformMatrix4fv(shader.uniforms[ModelViewProjectionMatrix], 1, GL_FALSE, &MVPMatrix.Transposed().M[0][0]);

            g_application.OnRender();    
            g_oculusVR.OnEyeRenderFinish(eyeIndex);
        }

        g_oculusVR.SubmitFrame();

        Application::VRMirrorMode mirrorMode = g_application.CurrMirrorMode();

        // Standard mirror blit (both eyes, distorted)
        if (mirrorMode == Application::Mirror_Regular)
        {
            ClearWindow(0.f, 0.f, 0.f);
            g_oculusVR.BlitMirror(ovrEye_Count, 0);
        }
       
        // Standard mirror blit (left eye, distorted)
        if (mirrorMode == Application::Mirror_RegularLeftEye)
        {
            ClearWindow(0.f, 0.f, 0.f);
            g_oculusVR.BlitMirror(ovrEye_Left, windowSize.w / 4);

            // rectangle indicating we're rendering left eye
            ShaderManager::GetInstance()->DisableShader();
            glViewport(0, 0, windowSize.w, windowSize.h);
            DrawRectangle(-0.75f, 0.f, 0.1f, 0.1f, 0.f, 1.f, 0.f);
        }

        // Standard mirror blit (right eye, distorted)
        if (mirrorMode == Application::Mirror_RegularRightEye)
        {
            ClearWindow(0.f, 0.f, 0.f);
            g_oculusVR.BlitMirror(ovrEye_Right, windowSize.w / 4);

            // rectangle indicating we're rendering right eye
            ShaderManager::GetInstance()->DisableShader();
            glViewport(0, 0, windowSize.w, windowSize.h);
            DrawRectangle(0.75f, 0.f, 0.1f, 0.1f, 0.f, 1.f, 0.f);
        }

        // Both eye mirror - no distortion (requires 2 extra renders!)
        if (mirrorMode == Application::Mirror_NonDistort)
        {
            g_oculusVR.OnNonDistortMirrorStart();

            for (int eyeIndex = 0; eyeIndex < ovrEye_Count; eyeIndex++)
            {
                const OVR::Matrix4f MVPMatrix = g_oculusVR.GetEyeMVPMatrix(eyeIndex);
                const ShaderProgram &shader = ShaderManager::GetInstance()->UseShaderProgram(ShaderManager::BasicShader);
                glUniformMatrix4fv(shader.uniforms[ModelViewProjectionMatrix], 1, GL_FALSE, &MVPMatrix.Transposed().M[0][0]);

                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
                glClearColor(0.2f, 0.2f, 0.6f, 0.0f);

                g_application.OnRender();
                g_oculusVR.BlitNonDistortMirror(eyeIndex == 0 ? 0 : windowSize.w / 2);
            }
        }

        // Left eye - no distortion (1 extra render)
        if (mirrorMode == Application::Mirror_NonDistortLeftEye)
        {
            ClearWindow(0.f, 0.f, 0.f);
            g_oculusVR.OnNonDistortMirrorStart();

            const OVR::Matrix4f MVPMatrix = g_oculusVR.GetEyeMVPMatrix(ovrEye_Left);
            const ShaderProgram &shader = ShaderManager::GetInstance()->UseShaderProgram(ShaderManager::BasicShader);
            glUniformMatrix4fv(shader.uniforms[ModelViewProjectionMatrix], 1, GL_FALSE, &MVPMatrix.Transposed().M[0][0]);

            glClearColor(0.2f, 0.2f, 0.6f, 0.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            g_application.OnRender();
            g_oculusVR.BlitNonDistortMirror(windowSize.w / 4);

            ShaderManager::GetInstance()->DisableShader();
            glViewport(0, 0, windowSize.w, windowSize.h);
            DrawRectangle(-0.75f, 0.f, 0.1f, 0.1f, 0.f, 1.f, 0.f);
        }

        //  Right eye - no distortion (1 extra render)
        if (mirrorMode == Application::Mirror_NonDistortRightEye)
        {
            ClearWindow(0.f, 0.f, 0.f);
            g_oculusVR.OnNonDistortMirrorStart();

            const OVR::Matrix4f MVPMatrix = g_oculusVR.GetEyeMVPMatrix(ovrEye_Right);
            const ShaderProgram &shader = ShaderManager::GetInstance()->UseShaderProgram(ShaderManager::BasicShader);
            glUniformMatrix4fv(shader.uniforms[ModelViewProjectionMatrix], 1, GL_FALSE, &MVPMatrix.Transposed().M[0][0]);

            glClearColor(0.2f, 0.2f, 0.6f, 0.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            g_application.OnRender();
            g_oculusVR.BlitNonDistortMirror(windowSize.w / 4);

            ShaderManager::GetInstance()->DisableShader();
            glViewport(0, 0, windowSize.w, windowSize.h);
            DrawRectangle(0.75f, 0.f, 0.1f, 0.1f, 0.f, 1.f, 0.f);
        }

        SDL_GL_SwapWindow(g_renderContext.window);
    }

    g_oculusVR.DestroyVR();
    g_renderContext.Destroy();

    SDL_Quit(); 

    return 0;
}