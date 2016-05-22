#version 410

layout(location = 0) in vec3 inVertex;
layout(location = 1) in vec4 inVertexColor;
layout(location = 2) in vec2 inTexCoord;

layout(location = 3) out vec4 vertexColor;
layout(location = 4) out vec2 texCoord;
layout(location = 15) in  vec3 inOffset;
layout(location = 20) out int instanceID;

// store MVP for each eye in a separate matrix UBO
// rendering will be done determined by gl_InstanceID (0 - left, 1 - right)
layout(std140) uniform EyeMVPs
{
    mat4 ModelViewProjectionMatrix[2];
};

void main()
{
    gl_Position = ModelViewProjectionMatrix[gl_InstanceID] * vec4(inVertex + inOffset, 1.0);
    
    vertexColor = inVertexColor;
    texCoord    = inTexCoord; 
    instanceID  = gl_InstanceID;
}
 