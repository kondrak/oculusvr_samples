#ifndef TEXTURE_INCLUDED
#define TEXTURE_INCLUDED

#include "renderer/OpenGL.hpp"

/*
 *  Basic texture
 */

class Texture
{
public:
    friend class TextureManager;

    const int Width()      const { return m_width; }
    const int Height()     const { return m_height; }
    const int Components() const { return m_components; }
    const GLuint Id()      const { return m_texId; }
    
private:
    Texture(const char *filename);
    ~Texture();

    GLuint Load();

    int    m_width;
    int    m_height;
    int    m_components;
    GLuint m_texId;
    unsigned char *m_textureData;
};

#endif