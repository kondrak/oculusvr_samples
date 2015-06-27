#include "renderer/Texture.hpp"
#include "stb_image/stb_image.h"


Texture::Texture(const char *filename) : m_texId(0), m_releaseData(true)
{
    m_textureData = stbi_load(filename, &m_width, &m_height, &m_components, 0);
    m_format = m_components == 3 ? GL_RGB : GL_RGBA;
    m_internalFormat = m_format;
}

Texture::Texture(unsigned char *data, int width, int height, int components, int format, int internalFormat) : m_width(width), 
                                                                                                               m_height(height), 
                                                                                                               m_components(components), 
                                                                                                               m_textureData(data), 
                                                                                                               m_format(format),
                                                                                                               m_internalFormat(internalFormat),
                                                                                                               m_releaseData(false),
                                                                                                               m_texId(0)
{
}


Texture::~Texture()
{
    if(m_releaseData && m_textureData != NULL)
        stbi_image_free( m_textureData );

    if (glIsTexture(m_texId))
    {
        glDeleteTextures(1, &m_texId);
    }
}

GLuint Texture::Load()
{
    // texture was already bound or could not be loaded - return texId/0
    if (m_textureData == nullptr)
        return m_texId;

    glGenTextures(1, &m_texId);
    glBindTexture(GL_TEXTURE_2D, m_texId);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    glTexImage2D(GL_TEXTURE_2D, 0, m_internalFormat, m_width, m_height, 0, m_format, GL_UNSIGNED_BYTE, m_textureData);

    if (m_releaseData)
    {
        stbi_image_free(m_textureData);
        m_textureData = NULL;
    }

    return m_texId;
}