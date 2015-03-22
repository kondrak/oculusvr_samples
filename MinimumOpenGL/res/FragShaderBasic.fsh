#version 100
uniform sampler2D  sTexture;

varying mediump vec2   TexCoord;
varying mediump vec4 vertexColor;

void main()
{
    gl_FragColor = texture2D(sTexture, TexCoord) * vertexColor; 
}
