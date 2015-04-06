#version 410

uniform mat4 ModelViewProjectionMatrix;

layout(location = 0) in vec3 inVertex;
layout(location = 1) in vec4 inVertexColor;
layout(location = 2) in vec2 inTexCoord;

layout(location = 3) out vec4 vertexColor;
layout(location = 4) out vec2  TexCoord;

void main()
{
    gl_Position = ModelViewProjectionMatrix * vec4(inVertex, 1.0);
    
    vertexColor = inVertexColor;
	TexCoord    = inTexCoord; 
}
 