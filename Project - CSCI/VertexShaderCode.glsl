#version 430

in layout(location=0) vec3 position;
in layout(location=1) vec2 vertexUV;
in layout(location=2) vec3 normal;


uniform mat4 rotationMatrix;
uniform mat4 scalingMatrix;
uniform mat4 transformMatrix;
uniform mat4 projectionMatrix;
uniform mat4 viewMatrix;



out vec2 UV;

void main()
{
	mat4 MVP = transformMatrix * rotationMatrix * scalingMatrix;
	gl_Position = projectionMatrix*viewMatrix * MVP *vec4(position, 1.0);
	UV = vertexUV;
}