#include <SDL.h>
#include "Application.hpp"
#include "renderer/CameraDirector.hpp"
#include "renderer/ShaderManager.hpp"
#include "renderer/TextureManager.hpp"
#include "renderer/OculusVR.hpp"

CameraDirector  g_cameraDirector;
extern OculusVR g_oculusVR;

Application::~Application()
{
    if (glIsBuffer(m_vertexBuffer))
        glDeleteBuffers(1, &m_vertexBuffer);

    if (glIsBuffer(m_colorBuffer))
        glDeleteBuffers(1, &m_colorBuffer);

    if (glIsVertexArray(m_vertexArray))
        glDeleteVertexArrays(1, &m_vertexArray);
}

void Application::OnStart()
{
    //glEnable(GL_MULTISAMPLE);

    g_cameraDirector.AddCamera(0.0f, 0.0f, 0.0f);

    const GLfloat triangleBufferData[] = {
        -1.0f, -1.0f, -15.f,
        0.0f, 1.0f, -15.f,
        1.0f, -1.0f, -15.f
    };

    const GLfloat vertexColorData[] = {
        1.0f, 1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f, 1.0f
    };

    // create quad VAO and VBOs
    glGenVertexArrays(1, &m_vertexArray);
    glBindVertexArray(m_vertexArray);

    glGenBuffers(1, &m_vertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, m_vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(triangleBufferData), triangleBufferData, GL_STATIC_DRAW);

    glGenBuffers(1, &m_colorBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, m_colorBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertexColorData), vertexColorData, GL_STATIC_DRAW);
}

void Application::OnRender()
{
    const ShaderProgram &shader = ShaderManager::GetInstance()->UseShaderProgram(ShaderManager::BasicShaderNoTex);

    GLuint vertexPosition_modelspaceID = glGetAttribLocation(shader.id, "inVertex");
    GLuint vertexColorAttr = glGetAttribLocation(shader.id, "inVertexColor");

    // setup quad data
    glBindVertexArray(m_vertexArray);
    glEnableVertexAttribArray(vertexPosition_modelspaceID);
    glEnableVertexAttribArray(vertexColorAttr);

    glBindBuffer(GL_ARRAY_BUFFER, m_vertexBuffer);
    glVertexAttribPointer(vertexPosition_modelspaceID, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

    glBindBuffer(GL_ARRAY_BUFFER, m_colorBuffer);
    glVertexAttribPointer(vertexColorAttr, 4, GL_FLOAT, GL_FALSE, 0, (void*)0);

    // draw the quad!
    glDrawArrays(GL_TRIANGLES, 0, 3);

    glDisableVertexAttribArray(vertexPosition_modelspaceID);
    glDisableVertexAttribArray(vertexColorAttr);
}

void Application::OnKeyPress(KeyCode key)
{
    switch(key)
    {
    case KEY_ESC:
        Terminate();
        break;
    case KEY_M:
        // MSAA toggle
        g_oculusVR.SetMSAA(!g_oculusVR.MSAAEnabled());
    default:
        break;
    } 
}
