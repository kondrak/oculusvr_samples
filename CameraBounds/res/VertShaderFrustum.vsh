#version 330

uniform mat4 ModelViewProjectionMatrix;

in vec3 inVertex;

void main()
{
    gl_Position = ModelViewProjectionMatrix * vec4(inVertex, 1.0);    
}
 