#version 330 core

out vec4 Color;

in vec3 TexCoords;

uniform samplerCube skybox;

void main()
{    
    Color = texture(skybox, TexCoords);
}
