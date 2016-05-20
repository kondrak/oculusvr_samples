#include <SDL.h>
#include "Application.hpp"
#include "renderer/CameraDirector.hpp"
#include "renderer/ShaderManager.hpp"
#include "renderer/TextureManager.hpp"

CameraDirector g_cameraDirector;

Application::~Application()
{
    if (glIsBuffer(m_vertexBuffer))
        glDeleteBuffers(1, &m_vertexBuffer);

    if (glIsBuffer(m_colorBuffer))
        glDeleteBuffers(1, &m_colorBuffer);

    if (glIsBuffer(m_texcoordBuffer))
        glDeleteBuffers(1, &m_texcoordBuffer);

    if (glIsVertexArray(m_vertexArray))
        glDeleteVertexArrays(1, &m_vertexArray);

    if (glIsVertexArray(m_quadOffsetBuffer))
        glDeleteVertexArrays(1, &m_quadOffsetBuffer);
}

void Application::OnStart()
{
    glEnable(GL_MULTISAMPLE);

    g_cameraDirector.AddCamera(0.0f, 0.0f, 0.0f);

    // load block texture
    m_texture = TextureManager::GetInstance()->LoadTexture("../common_res/block_blue.png");

    const GLfloat quadBufferData[] = {
        -0.1f, 0.1f, -1.5f,
         0.1f, 0.1f, -1.5f,
        -0.1f, -0.1f, -1.5f,
         0.1f, -0.1f, -1.5f
    };

    const GLfloat vertexColorData[] = {
        1.0f, 0.0f, 0.0f, 1.0f,
        1.0f, 0.0f, 0.0f, 1.0f,
        1.0f, 0.0f, 0.0f, 1.0f,
        1.0f, 0.0f, 0.0f, 1.0f,
    };

    const GLfloat vertexTexcoordData[] = {
        0.0f, 0.0f,
        1.0f, 0.0f,
        0.0f, 1.0f,
        1.0f, 1.0f
    };

    // create quad VAO and VBOs
    glGenVertexArrays(1, &m_vertexArray);
    glBindVertexArray(m_vertexArray);

    glGenBuffers(1, &m_vertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, m_vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadBufferData), quadBufferData, GL_STATIC_DRAW);

    glGenBuffers(1, &m_colorBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, m_colorBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertexColorData), vertexColorData, GL_STATIC_DRAW);

    glGenBuffers(1, &m_texcoordBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, m_texcoordBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertexTexcoordData), vertexTexcoordData, GL_STATIC_DRAW);

    glGenBuffers(1, &m_quadOffsetBuffer);
}

void Application::OnRender(float x, float y, float z)
{
    const ShaderProgram &shader = ShaderManager::GetInstance()->UseShaderProgram(ShaderManager::BasicShader);

    GLuint vertexPosition_modelspaceID = glGetAttribLocation(shader.id, "inVertex");
    GLuint vertexColorAttr = glGetAttribLocation(shader.id, "inVertexColor");
    GLuint texCoordAttr = glGetAttribLocation(shader.id, "inTexCoord");
    GLuint offsetAttr = glGetAttribLocation(shader.id, "inOffset");

    TextureManager::GetInstance()->BindTexture(m_texture);

    // setup quad data
    glBindVertexArray(m_vertexArray);
    glEnableVertexAttribArray(vertexPosition_modelspaceID);
    glEnableVertexAttribArray(vertexColorAttr);
    glEnableVertexAttribArray(texCoordAttr);
    glEnableVertexAttribArray(offsetAttr);

    glBindBuffer(GL_ARRAY_BUFFER, m_vertexBuffer);
    glVertexAttribPointer(vertexPosition_modelspaceID, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

    glBindBuffer(GL_ARRAY_BUFFER, m_colorBuffer);
    glVertexAttribPointer(vertexColorAttr, 4, GL_FLOAT, GL_FALSE, 0, (void*)0);

    glBindBuffer(GL_ARRAY_BUFFER, m_texcoordBuffer);
    glVertexAttribPointer(texCoordAttr, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

    const GLfloat offset[] = { x, y, z,
                               x, y, z,
                               x, y, z,
                               x, y, z };

    glBindBuffer(GL_ARRAY_BUFFER, m_quadOffsetBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(offset), offset, GL_STATIC_DRAW);
    glVertexAttribPointer(offsetAttr, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

    // draw the quad!
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    glDisableVertexAttribArray(vertexPosition_modelspaceID);
    glDisableVertexAttribArray(vertexColorAttr);
    glDisableVertexAttribArray(texCoordAttr);
    glDisableVertexAttribArray(offsetAttr);
}

void Application::OnRenderInstanced(float x, float y, float z)
{
    const ShaderProgram &shader = ShaderManager::GetInstance()->UseShaderProgram(ShaderManager::BasicShaderInstanced);

    GLuint vertexPosition_modelspaceID = glGetAttribLocation(shader.id, "inVertex");
    GLuint vertexColorAttr = glGetAttribLocation(shader.id, "inVertexColor");
    GLuint texCoordAttr = glGetAttribLocation(shader.id, "inTexCoord");
    GLuint mvpAttr = glGetAttribLocation(shader.id, "ModelViewProjectionMatrix");
    GLuint offsetAttr = glGetAttribLocation(shader.id, "inOffset");

    TextureManager::GetInstance()->BindTexture(m_texture);

    // setup quad data
    glBindVertexArray(m_vertexArray);
    glEnableVertexAttribArray(vertexPosition_modelspaceID);
    glEnableVertexAttribArray(vertexColorAttr);
    glEnableVertexAttribArray(texCoordAttr);
    glEnableVertexAttribArray(mvpAttr);
    glEnableVertexAttribArray(offsetAttr);

    glBindBuffer(GL_ARRAY_BUFFER, m_vertexBuffer);
    glVertexAttribPointer(vertexPosition_modelspaceID, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

    glBindBuffer(GL_ARRAY_BUFFER, m_colorBuffer);
    glVertexAttribPointer(vertexColorAttr, 4, GL_FLOAT, GL_FALSE, 0, (void*)0);

    glBindBuffer(GL_ARRAY_BUFFER, m_texcoordBuffer);
    glVertexAttribPointer(texCoordAttr, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

    const GLfloat offset[] = { x, y, z,
                               x, y, z,
                               x, y, z,
                               x, y, z };

    glBindBuffer(GL_ARRAY_BUFFER, m_quadOffsetBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(offset), offset, GL_STATIC_DRAW);
    glVertexAttribPointer(offsetAttr, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

    // draw the scene twice using instancing
    glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, 2);

    glDisableVertexAttribArray(vertexPosition_modelspaceID);
    glDisableVertexAttribArray(vertexColorAttr);
    glDisableVertexAttribArray(texCoordAttr);
    glDisableVertexAttribArray(mvpAttr);
    glDisableVertexAttribArray(offsetAttr);
}

void Application::OnKeyPress(KeyCode key)
{
    switch(key)
    {
    case KEY_ESC:
        Terminate();
        break;
    case KEY_R:
        m_instancedRender = !m_instancedRender;
    default:
        break;
    } 
}
