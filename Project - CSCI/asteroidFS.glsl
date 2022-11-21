#version 430

out vec4 Color;

in vec3 UV;

uniform samplerCube textureAsteroid;

void main()
{    
    Color = texture(textureAsteroid, UV);
}