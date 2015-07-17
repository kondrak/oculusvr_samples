#version 410

uniform mat4 ModelViewProjectionMatrix;

layout(location = 0) in vec3 inVertex;
layout(location = 1) in vec4 inVertexColor;

layout(location = 3) out vec4 vertexColor;

void main()
{
    gl_Position = ModelViewProjectionMatrix * vec4(inVertex, 1.0);
    
    vertexColor = inVertexColor;
}
 