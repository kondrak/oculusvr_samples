#include "renderer/RenderContext.hpp"

void RenderContext::Init(const char *title, int x, int y, int w, int h)
{
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 2);
    SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);

    window = SDL_CreateWindow(title, x, y, w, h, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_SHOWN);

    context = SDL_GL_CreateContext(window); 

    SDL_GetWindowSize(window, &width, &height);

    // VSync control
    SDL_GL_SetSwapInterval(0);

    halfWidth  = width  >> 1;
    halfHeight = height >> 1;

    scrRatio = (float)width / (float)height;

/*
 *       ----(1.0)---
 *       |          |
 *    -ratio      ratio
 *       |          |
 *       ---(-1.0)---
 */
    left   = -scrRatio;
    right  = scrRatio;
    bottom = -1.0f;
    top    = 1.0f;
}

void RenderContext::Destroy()
{
    if (window)
    {
        SDL_GL_DeleteContext(context);
        SDL_DestroyWindow(window);
        window = NULL;
    }
}