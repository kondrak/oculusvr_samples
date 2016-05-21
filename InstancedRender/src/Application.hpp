#ifndef APPLICATION_INCLUDED
#define APPLICATION_INCLUDED

#include "InputHandlers.hpp"
#include "renderer/OpenGL.hpp"
#include "renderer/Texture.hpp"

/*
 * main application 
 */
class Application
{
public:
    Application::Application() : m_running(true)
    {
    }

    ~Application();

    void OnStart();
    void OnRender();
    void OnRenderInstanced();

    inline bool Running() const  { return m_running; }
    inline void Terminate()      { m_running = false; }
    inline bool InstancedRender() const { return m_instancedRender; }

    void OnKeyPress(KeyCode key);
private:
    bool m_running;
    bool m_instancedRender;

    // rendered quad data
    GLuint m_vertexBuffer;
    GLuint m_colorBuffer;
    GLuint m_texcoordBuffer;
    GLuint m_quadOffsetBuffer;
    GLuint m_vertexArray;
    Texture *m_texture;

};

#endif
