#version 410

layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;

layout(location = 3) in vec2 tCoord[];
layout(location = 4) in vec4 vColor[];
layout(location = 9)  out vec2 texCoord;
layout(location = 10) out vec4 vertexColor;
layout(location = 20) in int instanceID[];

void main()
{
    gl_ViewportIndex = instanceID[0];
    gl_Position = gl_in[0].gl_Position;
    texCoord = tCoord[0];
    vertexColor = vColor[0];
    EmitVertex();
    gl_Position = gl_in[1].gl_Position;
    texCoord = tCoord[1];
    vertexColor = vColor[1];
    EmitVertex();
    gl_Position = gl_in[2].gl_Position;
    texCoord = tCoord[2];
    vertexColor = vColor[2];

    EmitVertex();
    EndPrimitive();
}