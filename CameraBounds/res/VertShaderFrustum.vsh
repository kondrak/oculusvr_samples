#version 100

attribute highp   vec3 inVertex;
uniform highp     mat4 ModelViewProjectionMatrix;

void main()
{
    gl_Position = ModelViewProjectionMatrix * vec4(inVertex, 1.0);    
}
 