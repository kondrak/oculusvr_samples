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
    // Oculus VR mirror modes
    enum VRMirrorMode
    {
        Mirror_Regular,
        Mirror_NonDistort,
        Mirror_LeftEye,
        Mirror_RightEye,
        MM_Count
    };


    Application::Application() : m_running(true), m_mirrorMode(Mirror_Regular)
    {
    }

    ~Application();

    void OnStart();
    void OnRender();

    inline bool Running() const  { return m_running; }
    inline void Terminate()      { m_running = false; }

    void OnKeyPress(KeyCode key);

    const VRMirrorMode CurrMirrorMode() const { return (VRMirrorMode)m_mirrorMode; }
private:  
    bool m_running;
    int m_mirrorMode;

    // rendered quad data
    GLuint m_vertexBuffer;
    GLuint m_colorBuffer;
    GLuint m_texcoordBuffer;
    GLuint m_vertexArray;
    Texture *m_texture;

};

#endif
