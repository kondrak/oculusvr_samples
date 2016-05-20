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

    g_renderContext.Init("Oculus Rift OpenGL instanced rendering", 100, 100, windowSize.w, windowSize.h);
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

    // for instanced rendering we need to store each MVP 
    GLuint mvpInstanceVBO;
    glGenBuffers(1, &mvpInstanceVBO);

    while (g_application.Running())
    {
        // handle key presses
        processEvents();
        glClearColor(0.2f, 0.2f, 0.6f, 0.0f);

        g_oculusVR.OnRenderStart();

        if(g_application.InstancedRender())
            RenderInstanced(mvpInstanceVBO);
        else
            Render();

        g_oculusVR.OnRenderFinish();

        g_oculusVR.SubmitFrame();
        g_oculusVR.BlitMirror();
        SDL_GL_SwapWindow(g_renderContext.window);
    }

    glDeleteBuffers(1, &mvpInstanceVBO);

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

// instanced rendering: draw the scene once using OpenGL instances
void RenderInstanced(GLuint &instanceVBO)
{
    // MVP matrices for left and right eye
    GLfloat mvps[32];

    // fetch MVP matrices for both eyes
    for (int i = 0; i < 2; i++)
    {
        OVR::Matrix4f MVPMatrix = g_oculusVR.OnEyeRender(i);
        memcpy(&mvps[i * 16], &MVPMatrix.Transposed().M[0][0], sizeof(GLfloat) * 16);
    }

    // update instanceVBO with new matrix values
    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
    glBufferData(GL_ARRAY_BUFFER, 2 * sizeof(GLfloat) * 16, mvps, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // a 4x4 mat4 shader attribute is broken down into 4 vec4s taking up consecutive locations
    glEnableVertexAttribArray(5);
    glEnableVertexAttribArray(6);
    glEnableVertexAttribArray(7);
    glEnableVertexAttribArray(8);
    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
    glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 16, (GLvoid*)0);
    glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 16, (GLvoid*)(4  * sizeof(GLfloat)));
    glVertexAttribPointer(7, 4, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 16, (GLvoid*)(8  * sizeof(GLfloat)));
    glVertexAttribPointer(8, 4, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 16, (GLvoid*)(12 * sizeof(GLfloat)));
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    // fetch next MVP after rendering a single instance
    glVertexAttribDivisor(5, 1);
    glVertexAttribDivisor(6, 1);
    glVertexAttribDivisor(7, 1);
    glVertexAttribDivisor(8, 1);

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