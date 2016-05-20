#version 410

uniform sampler2D sTexture;

layout(location = 9)  in vec4 vertexColor;
layout(location = 10) in vec2 TexCoord;

out vec4 fragmentColor;

void main()
{
    fragmentColor = texture2D(sTexture, TexCoord) * vertexColor; 
}
