#version 330 core

layout(location=0) in vec3 position;
layout(location=1) in vec2 vertexUV;
layout(location=2) in vec3 normal;

out vec2 UV;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 transform;
uniform mat4 rotation;
uniform mat4 scaling;
uniform mat4 invTrans;

void main()
{
    gl_Position = projection * view * transform * rotation * scaling * vec4(position, 1.0);

    UV = vertexUV;
}

