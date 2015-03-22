#ifndef RENDERCONTEXT_INCLUDED
#define RENDERCONTEXT_INCLUDED

#include "renderer/OpenGL.hpp"
#include <SDL.h>
#include <SDL_syswm.h>

/*
 * SDL-based OpenGL render context
 */
class RenderContext
{
public:
    RenderContext() : window(NULL)
    {
    }

    void Init(const char *title, int x, int y, int w, int h);
    void Destroy();

    SDL_Window *window;
    SDL_GLContext context;
};

#endif