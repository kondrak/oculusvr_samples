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
            return NULL;
        }

        m_textures[textureName] = newTex;       
    }

    return m_textures[textureName];
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

