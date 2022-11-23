#version 330 core

out vec4 Color;

in vec2 UV;

uniform sampler2D textureSpacecraft;

void main()
{    
    Color = texture(textureSpacecraft, UV);
}
