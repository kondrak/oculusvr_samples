#include <SDL.h>
#include "Application.hpp"
#include "renderer/ShaderManager.hpp"
#include "renderer/TextureManager.hpp"

Application::~Application()
{
    if (glIsBuffer(m_vertexBuffer))
        glDeleteBuffers(1, &m_vertexBuffer);

    if (glIsBuffer(m_colorBuffer))
        glDeleteBuffers(1, &m_colorBuffer);

    if (glIsBuffer(m_texcoordBuffer))
        glDeleteBuffers(1, &m_texcoordBuffer);

    if (glIsBuffer(m_vertexArray))
        glDeleteBuffers(1, &m_vertexArray);
}

void Application::OnStart()
{
    glEnable(GL_DEPTH_TEST);
    glClientActiveTextureARB(GL_TEXTURE1_ARB);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);

    glClientActiveTextureARB(GL_TEXTURE0_ARB);
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);

    // load shaders and block texture
    ShaderManager::GetInstance()->LoadShaders();
    m_texture = TextureManager::GetInstance()->LoadTexture("../common_res/block_blue.png");

    const GLfloat quadBufferData[] = {
        -1.0f, 1.0f, -1.5f,
        1.0f, 1.0f, -1.5f,
        -1.0f, -1.0f, -1.5f,
        1.0f, -1.0f, -1.5f
    };

    const GLfloat vertexColorData[] = {
        1.0f, 0.0f, 0.0f, 1.0f,
        0.0f, 1.0f, 0.0f, 1.0f,
        0.0f, 0.0f, 1.0f, 1.0f,
        1.0f, 0.0f, 1.0f, 1.0f,
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
}

void Application::OnRender()
{
    const ShaderProgram &shader = ShaderManager::GetInstance()->UseShaderProgram(ShaderManager::BasicShader);

    GLuint vertexPosition_modelspaceID = glGetAttribLocation(shader.id, "inVertex");
    GLuint vertexColorAttr = glGetAttribLocation(shader.id, "inVertexColor");
    GLuint texCoordAttr = glGetAttribLocation(shader.id, "inTexCoord");

    TextureManager::GetInstance()->BindTexture(m_texture);

    // setup quad data
   glBindVertexArray(m_vertexArray);
   glEnableVertexAttribArray(vertexPosition_modelspaceID);
   glEnableVertexAttribArray(vertexColorAttr);
   glEnableVertexAttribArray(texCoordAttr);

    glBindBuffer(GL_ARRAY_BUFFER, m_vertexBuffer);
    glVertexAttribPointer(vertexPosition_modelspaceID, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

    glBindBuffer(GL_ARRAY_BUFFER, m_colorBuffer);
    glVertexAttribPointer(vertexColorAttr, 4, GL_FLOAT, GL_FALSE, 0, (void*)0);

    glBindBuffer(GL_ARRAY_BUFFER, m_texcoordBuffer);
    glVertexAttribPointer(texCoordAttr, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

    // draw the quad!
    glBindVertexArray(m_vertexArray);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4); 

    glDisableVertexAttribArray(vertexPosition_modelspaceID);
    glDisableVertexAttribArray(vertexColorAttr);
    glDisableVertexAttribArray(texCoordAttr);
}

void Application::OnKeyPress(KeyCode key)
{
    switch(key)
    {
    case KEY_ESC:
        Terminate();
        break;
    default:
        break;
    } 
}
