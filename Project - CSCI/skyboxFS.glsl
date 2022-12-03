#version 430

out vec4 Color;

in vec3 TexCoords;

uniform samplerCube skybox;
uniform samplerCube skybox2;
uniform float intensity;

void main()
{    
    Color = (1- intensity) * texture(skybox, TexCoords) + intensity*texture(skybox2, TexCoords);
}