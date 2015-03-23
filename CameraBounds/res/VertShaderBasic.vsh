#version 330

uniform mat4 ModelViewProjectionMatrix;

in vec3 inVertex;
in vec4 inVertexColor;
in vec2 inTexCoord;

out vec4 vertexColor;
out vec2  TexCoord;

void main()
{
    gl_Position = ModelViewProjectionMatrix * vec4(inVertex, 1.0);
    
    vertexColor = inVertexColor;
	TexCoord    = inTexCoord; 
}
 