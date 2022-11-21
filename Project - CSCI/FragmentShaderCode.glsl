#version 430

in vec2 UV;

uniform sampler2D myTextureSampler0;
uniform sampler2D myTextureSampler1;

out vec4 Color;

void main()
{
	Color = texture(myTextureSampler0, UV).rgba;
}