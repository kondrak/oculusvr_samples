#include "Utils.hpp"
#ifdef _WIN32
#include <Windows.h>
#endif

#include "renderer/OpenGL.hpp"

void LogError(const char *msg)
{
    // basic error logging for VS debugger
#ifdef _WIN32
    OutputDebugStringA(msg);
#else
    printf("%s\n", msg);
#endif
}

void ClearWindow(float r, float g, float b)
{
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glClearColor(r, g, b, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void DrawRectangle(float x, float y, float w, float h, float r, float g, float b)
{
    GLboolean texOn   = glIsEnabled(GL_TEXTURE_2D);
    GLboolean depthOn = glIsEnabled(GL_DEPTH_TEST);
    GLboolean blendOn = glIsEnabled(GL_BLEND);

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glActiveTexture(GL_TEXTURE0);
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();  
    glOrtho(-1.f, 1.f, 1.f, -1.0f, 0.1f, 10.0f);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glBegin(GL_QUADS);
    glColor3f(r, g, b);
    glVertex3f(x - w / 2.f, y - w / 2.f, -1.f);
    glVertex3f(x + w / 2.f, y - w / 2.f, -1.f);
    glVertex3f(x + w / 2.f, y + w / 2.f, -1.f);
    glVertex3f(x - w / 2.f, y + w / 2.f, -1.f);
    glEnd();

    if (texOn)
        glEnable(GL_TEXTURE_2D);

    if (depthOn)
        glEnable(GL_DEPTH_TEST);

    if (blendOn)
        glEnable(GL_BLEND);
}