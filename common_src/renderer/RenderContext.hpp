#ifndef RENDERCONTEXT_INCLUDED
#define RENDERCONTEXT_INCLUDED

#include "Math.hpp"
#include "renderer/OpenGL.hpp"
#include <SDL.h>
#include <SDL_syswm.h>

/*
 * SDL-based OpenGL render context
 */

class RenderContext
{
public:
    RenderContext() : window(NULL), 
                      fov(75.f * PIdiv180),
                      nearPlane(0.1f), 
                      farPlane(1000.f), 
                      scrRatio(0.0f),
                      width(0), 
                      height(0), 
                      halfWidth(0), 
                      halfHeight(0),
                      left(0.0f),
                      right(0.0f),
                      bottom(0.0f),
                      top(0.0f)
    {
    }

    void Init(const char *title, int x, int y, int w, int h);
    void Destroy();

    SDL_Window *window;
    SDL_GLContext context;

    float fov;
    float nearPlane;
    float farPlane;
    float scrRatio;
    int   width;
    int   height;
    int   halfWidth;
    int   halfHeight;

    // ortho parameters
    float left;
    float right;
    float bottom;
    float top;

    Math::Matrix4f ModelViewProjectionMatrix; // global MVP used to orient the entire world
};

#endif