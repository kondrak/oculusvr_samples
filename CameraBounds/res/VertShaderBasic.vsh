#version 100

attribute highp   vec3 inVertex;
attribute mediump vec4 inVertexColor;
attribute mediump vec2 inTexCoord;

uniform highp   mat4 ModelViewProjectionMatrix;
varying mediump vec4 vertexColor;
varying mediump vec2  TexCoord;

void main()
{
    gl_Position = ModelViewProjectionMatrix * vec4(inVertex, 1.0);
    
    vertexColor = inVertexColor;
	TexCoord    = inTexCoord; 
}
 