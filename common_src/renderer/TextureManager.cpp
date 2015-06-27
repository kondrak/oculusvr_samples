#include "renderer/TextureManager.hpp"


TextureManager* TextureManager::GetInstance()
{
    static TextureManager instance;

    return &instance;
}

TextureManager::~TextureManager()
{
    ReleaseTextures();
}

void TextureManager::ReleaseUnmanagedTexture(Texture *t)
{
    delete t;
    t = nullptr;
}

void TextureManager::ReleaseTextures()
{
    for (std::map<std::string, Texture*>::iterator it = m_textures.begin(); it != m_textures.end(); ++it)
    {
        delete it->second;
    }

    m_currentTexture = 0;
}

Texture *TextureManager::LoadTexture(const char *textureName)
{
    if (m_textures.count(textureName) == 0)
    {
        LOG_MESSAGE("[TextureManager] Loading texture: " << textureName);
        Texture *newTex = new Texture(textureName);   

        // failed to load texture/file doesn't exist
        if (newTex->Load() == 0)
        {
            delete newTex;
            return nullptr;
        }

        m_textures[textureName] = newTex;       
    }

    return m_textures[textureName];
}

Texture *TextureManager::LoadUnmanagedTexture(unsigned char *data, int width, int height, int components, int format, int internalFormat)
{
        LOG_MESSAGE("[TextureManager] Loading unmanaged texture: " << width << "x" << height << "x" << components);
        Texture *newTex = new Texture(data, width, height, components, format, internalFormat);

        if (newTex->Load() == 0)
        {
            delete newTex;
            return nullptr;
        }

        return newTex;
}

void TextureManager::BindTexture(Texture *t)
{
    if (m_currentTexture != t->Id())
    {
        m_currentTexture = t->Id();
        glBindTexture(GL_TEXTURE_2D, t->Id());
    }
}

void TextureManager::UnBindTexture()
{
    if (m_currentTexture != 0)
    {
        glBindTexture(GL_TEXTURE_2D, 0);
        m_currentTexture = 0;
    }
}

