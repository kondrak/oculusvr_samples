#version 330

uniform sampler2D sTexture;

in vec2 TexCoord;
in vec4 vertexColor;

void main()
{
    gl_FragColor = texture2D(sTexture, TexCoord) * vertexColor; 
}
