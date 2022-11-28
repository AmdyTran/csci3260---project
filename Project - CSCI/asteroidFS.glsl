#version 430

out vec4 Color;

in vec2 UV;

uniform sampler2D textureAsteroid;

void main()
{    
    Color = texture(textureAsteroid, UV);
}