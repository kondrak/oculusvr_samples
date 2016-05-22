#include "Application.hpp"
#include "InputHandlers.hpp"
#include "renderer/RenderContext.hpp"
#include "renderer/ShaderManager.hpp"
#include "OculusVRInstanced.hpp"

// application globals
RenderContext g_renderContext;
Application   g_application;
OculusVR      g_oculusVR;

void Render();
void RenderInstanced(GLuint &instanceVBO);

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

    g_renderContext.Init("Oculus Rift OpenGL instanced rendering (press R to toggle)", 100, 100, windowSize.w, windowSize.h);
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
    g_oculusVR.ShowPerfStats(ovrPerfHud_AppRenderTiming);

    // for instanced rendering, store both eyes' MVPs in UBO
    GLuint mvpUBO;
    glGenBuffers(1, &mvpUBO);
    glBindBuffer(GL_UNIFORM_BUFFER, mvpUBO);
    glBufferData(GL_UNIFORM_BUFFER, 2 * sizeof(GLfloat) * 16, 0, GL_STREAM_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    while (g_application.Running())
    {
        // handle key presses
        processEvents();
        glClearColor(0.2f, 0.2f, 0.6f, 0.0f);

        g_oculusVR.OnRenderStart();

        if(g_application.InstancedRender())
            RenderInstanced(mvpUBO);
        else
            Render();

        g_oculusVR.OnRenderFinish();

        g_oculusVR.SubmitFrame();
        g_oculusVR.BlitMirror();
        SDL_GL_SwapWindow(g_renderContext.window);
    }

    glDeleteBuffers(1, &mvpUBO);

    g_oculusVR.ShowPerfStats(ovrPerfHud_Off);
    g_oculusVR.DestroyVR();
    g_renderContext.Destroy();

    SDL_Quit(); 

    return 0;
}


// standard render: draw scene twice per eye
void Render()
{
    for (int eyeIndex = 0; eyeIndex < ovrEye_Count; eyeIndex++)
    {
        OVR::Matrix4f MVPMatrix = g_oculusVR.OnEyeRender(eyeIndex);

        // update MVP in quad shader
        const ShaderProgram &shader = ShaderManager::GetInstance()->UseShaderProgram(ShaderManager::BasicShader);
        glUniformMatrix4fv(shader.uniforms[ModelViewProjectionMatrix], 1, GL_FALSE, &MVPMatrix.Transposed().M[0][0]);

        g_application.OnRender();
    }
}

// instanced rendering: draw the scene once using OpenGL instancing
void RenderInstanced(GLuint &ubo)
{
    // MVP matrices for left and right eye
    GLfloat mvps[32];

    const ShaderProgram &shader = ShaderManager::GetInstance()->GetShaderProgram(ShaderManager::BasicShaderInstanced);

    // fetch location of MVP UBO in shader
    GLuint mvpBinding = 0;
    GLint blockIdx = glGetUniformBlockIndex(shader.id, "EyeMVPs");
    glUniformBlockBinding(shader.id, blockIdx, mvpBinding);

    // fetch MVP matrices for both eyes
    for (int i = 0; i < 2; i++)
    {
        OVR::Matrix4f MVPMatrix = g_oculusVR.OnEyeRender(i);
        memcpy(&mvps[i * 16], &MVPMatrix.Transposed().M[0][0], sizeof(GLfloat) * 16);
    }

    // update MVP UBO with new eye matrices
    glBindBuffer(GL_UNIFORM_BUFFER, ubo);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, 2 * sizeof(GLfloat) * 16, mvps);
    glBindBufferRange(GL_UNIFORM_BUFFER, mvpBinding, ubo, 0, 2 * sizeof(GLfloat) * 16);

    ovrRecti viewPortL = g_oculusVR.GetEyeViewport(0);
    ovrRecti viewPortR = g_oculusVR.GetEyeViewport(1);

    // create viewport array for geometry shader
    GLfloat viewports[] = { (GLfloat)viewPortL.Pos.x, (GLfloat)viewPortL.Pos.y, 
                            (GLfloat)viewPortL.Size.w, (GLfloat)viewPortL.Size.h,
                            (GLfloat)viewPortR.Pos.x, (GLfloat)viewPortR.Pos.y, 
                            (GLfloat)viewPortR.Size.w, (GLfloat)viewPortR.Size.h };
    glViewportArrayv(0, 2, viewports);

    // perform instanced render - half the drawcalls compared to "standard" rendering!
    g_application.OnRenderInstanced();
}