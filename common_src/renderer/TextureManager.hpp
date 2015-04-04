#ifndef TEXTUREMANAGER_HPP
#define TEXTUREMANAGER_HPP

#include "renderer/OpenGL.hpp"
#include "renderer/Texture.hpp"
#include <map>

class TextureManager
{
public:
    static TextureManager* GetInstance();

    Texture *LoadTexture(const char *textureName);
    void BindTexture(Texture *t);
    void UnBindTexture(); // set current texture to 0;
    void ReleaseTextures();
private:
    TextureManager() : m_currentTexture(0)
    {
    }

    ~TextureManager();

    std::map<std::string, Texture *> m_textures;
    GLuint m_currentTexture;
};

#endif
