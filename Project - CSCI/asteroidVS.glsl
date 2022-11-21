#version 430

in layout(location=0) vec3 position;
in layout(location=1) vec2 vertexUV;
in layout(location=2) vec3 normal;

out vec2 UV;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model[500];

void main()
{
    gl_Position = projection * view * model[gl_InstanceID] * vec4(position, 1.0); 
    UV = vertexUV;
}

