#version 410
uniform sampler2D  sTexture;
uniform vec4       vertexColor;

layout(location = 4) in vec2 TexCoord;

out vec4 fragmentColor;

void main()
{
    fragmentColor = texture2D(sTexture, TexCoord) * vertexColor; 
}
