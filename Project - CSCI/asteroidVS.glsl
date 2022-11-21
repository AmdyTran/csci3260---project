#version 430 core
layout (location = 0) in vec3 position;
layout (location = 2) in vec2 vertexUV
layout (location = 3) in mat4 instanceMatrix;

out vec2 UV;

uniform mat4 projection;
uniform mat4 view;

void main()
{
    gl_Position = projection * view * instanceMatrix * vec4(aPos, 1.0); 
    UV = vertexUV;
}