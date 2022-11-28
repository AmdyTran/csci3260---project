#version 430

out vec4 Color;

in vec3 TexCoords;

uniform samplerCube skybox;

void main()
{    
    Color = texture(skybox, TexCoords);
}