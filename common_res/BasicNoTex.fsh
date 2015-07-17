#version 410

layout(location = 3) in vec4 vertexColor;

out vec4 fragmentColor;

void main()
{
    fragmentColor = vertexColor; 
}
