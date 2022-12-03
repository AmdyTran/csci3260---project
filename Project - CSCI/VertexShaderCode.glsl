#version 430

layout(location=0) in vec3 position;
layout(location=1) in vec2 vertexUV;
layout(location=2) in vec3 normal;


uniform mat4 rotationMatrix;
uniform mat4 scalingMatrix;
uniform mat4 transformMatrix;
uniform mat4 projectionMatrix;
uniform mat4 viewMatrix;

out vec2 UV;

// for lighting
out vec3 normalWorld;
out vec3 vertexPositionWorld;
out mat4 viewMVP;
out mat4 view;

void main()
{
	mat4 MVP = transformMatrix * rotationMatrix * scalingMatrix;
	gl_Position = projectionMatrix*viewMatrix * MVP *vec4(position, 1.0);
	UV = vertexUV;
    
    viewMVP = viewMatrix * MVP;
    view = viewMatrix; 
        
    // lighting
    vec4 v = vec4(position,1.0);
    normalWorld = (viewMatrix *  MVP  * vec4(normal, 0)).xyz;
    vertexPositionWorld = (viewMatrix *  MVP * v).xyz;
}
