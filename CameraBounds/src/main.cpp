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

    g_oculusVR.InitVR();
    OVR::Vector4i windowDim = g_oculusVR.RenderDimensions();

    g_renderContext.Init("Oculus Rift IR Camera Bounds Renderer", windowDim.x, windowDim.y, windowDim.z, windowDim.w);
    SDL_ShowCursor( SDL_DISABLE );

    if (glewInit() != GLEW_OK)
    {
        g_renderContext.Destroy();
        g_oculusVR.DestroyVR();
        SDL_Quit();
        return 1;
   }
    
    if (!g_oculusVR.InitVRBuffers())
    {
        g_renderContext.Destroy();
        g_oculusVR.DestroyVR();
        SDL_Quit();
        return 1;
    }

    ShaderManager::GetInstance()->LoadShaders();

    SDL_SysWMinfo info;
    memset(&info, 0, sizeof(SDL_SysWMinfo));
    SDL_VERSION(&info.version);
    SDL_GetWindowWMInfo(g_renderContext.window, &info);

    g_oculusVR.ConfigureRender(info.info.win.window, windowDim.z, windowDim.w);

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

            // update MVP in both shaders
            const ShaderProgram &shader = ShaderManager::GetInstance()->UseShaderProgram(ShaderManager::BasicShader);
            glUniformMatrix4fv(shader.uniforms[ModelViewProjectionMatrix], 1, GL_FALSE, &MVPMatrix.Transposed().M[0][0]);
            const ShaderProgram &shader2 = ShaderManager::GetInstance()->UseShaderProgram(ShaderManager::OVRFrustumShader);
            glUniformMatrix4fv(shader2.uniforms[ModelViewProjectionMatrix], 1, GL_FALSE, &MVPMatrix.Transposed().M[0][0]);

            g_application.OnRender();     
            g_oculusVR.RenderTrackerFrustum();
        }

        g_oculusVR.OnRenderEnd();
    }

    g_renderContext.Destroy();
    g_oculusVR.DestroyVR();

    SDL_Quit(); 

    return 0;
}