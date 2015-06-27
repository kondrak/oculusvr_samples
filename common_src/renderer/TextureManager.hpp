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
    Texture *LoadUnmanagedTexture(unsigned char *data, int width, int height, int components, int format, int internalFormat);
    void BindTexture(Texture *t);
    void UnBindTexture(); // set current texture to 0;
    void ReleaseUnmanagedTexture(Texture *t);
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
