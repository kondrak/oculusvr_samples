#include "renderer/RenderContext.hpp"

void RenderContext::Init(const char *title, int x, int y, int w, int h)
{
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    window  = SDL_CreateWindow(title, x, y, w, h, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_SHOWN);
    context = SDL_GL_CreateContext(window); 
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