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
    Texture(unsigned char *data, int width, int height, int components, int format, int internalFormat);
    ~Texture();

    GLuint Load();

    int    m_width;
    int    m_height;
    int    m_components;
    int    m_format;
    int    m_internalFormat;
    bool   m_releaseData;     // should data release be handled automatically?
    GLuint m_texId;
    unsigned char *m_textureData;    
};

#endif