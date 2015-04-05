#version 410

uniform mat4 ModelViewProjectionMatrix;

layout(location = 0) in vec3 inVertex;

void main()
{
    gl_Position = ModelViewProjectionMatrix * vec4(inVertex, 1.0);    
}
 