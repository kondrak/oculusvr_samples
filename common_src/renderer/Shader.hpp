#ifndef SHADER_HPP
#define SHADER_HPP

#include "renderer/OpenGL.hpp"

// Group shader programs and their uniform locations together
enum UniformId
{
    ModelViewProjectionMatrix,
    VertexColor,
    NUM_UNIFORMS
};

struct ShaderProgram
{
    GLuint id;
    GLuint vertShader;
    GLuint fragShader;

    GLint uniforms[NUM_UNIFORMS];

    ShaderProgram() : id(0), vertShader(0), fragShader(0)
    {
        for (int i = 0; i < NUM_UNIFORMS; i++)
        {
            uniforms[i] = -1;
        }
    }
};

#endif