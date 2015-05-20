#include "renderer/Texture.hpp"
#include "stb_image/stb_image.h"


Texture::Texture( const char *filename ) : m_texId(0)
{
    m_textureData = stbi_load(filename, &m_width, &m_height, &m_components, 0);
}


Texture::~Texture()
{
    if(m_textureData != NULL)
        stbi_image_free( m_textureData );

    if (glIsTexture(m_texId))
    {
        glDeleteTextures(1, &m_texId);
    }
}

GLuint Texture::Load()
{
    // texture was already bound or could not be loaded - return texId/0
    if(m_textureData == NULL)
        return m_texId;

    glGenTextures(1, &m_texId);
    glBindTexture(GL_TEXTURE_2D, m_texId);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    glTexImage2D(GL_TEXTURE_2D, 0, m_components, m_width, m_height, 0, m_components == 3 ? GL_RGB : GL_RGBA, GL_UNSIGNED_BYTE, m_textureData);

    stbi_image_free( m_textureData );

    m_textureData = NULL;

    return m_texId;
}