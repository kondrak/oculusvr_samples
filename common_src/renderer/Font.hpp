#ifndef FONT_HPP
#define FONT_HPP

#include "renderer/OpenGL.hpp"
#include <string>

class Texture;

class Font
{
public:
    Font(const char *texture);
    ~Font();
    void SetColor(const Math::Vector4f &color) { m_color = color; }
    void SetPosition(const Math::Vector3f &position) { m_position = position; }
    void SetScale(const Math::Vector2f &scale) { m_scale = scale; }
    void drawText(const std::string &text);
    void drawText(const std::string &text, float x, float y, float z, float r, float g, float b, float a);
    void drawText(const std::string &text, const Math::Vector3f &position, const Math::Vector4f &color=Math::Vector4f(1.f, 1.f, 1.f, 1.f));
    void drawText(const std::string &text, float x, float y, float z=-1.0f);
private:
    void renderAt(const Math::Vector3f &pos, int w, int h, int uo, int vo, const Math::Vector4f &color);
    Texture*        m_texture;
    Math::Vector2f  m_scale;
    Math::Vector3f  m_position;
    Math::Vector4f  m_color;

    // OpenGL thingamabobs
    GLuint m_fontVertexArray; // VAO

    GLuint m_vertexBuffer;    // VBO
    GLuint m_indexBuffer;     // IBO

    GLuint m_vertexPosAttr;   // attribute location
};

#endif