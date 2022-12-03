#version 430

layout(location=0) in vec3 position;
layout(location=1) in vec2 vertexUV;
layout(location=2) in vec3 normal;

out vec2 UV;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 transform;
uniform mat4 rotation;
uniform mat4 model[500];

// for lighting
out vec3 normalWorld;
out vec3 vertexPositionWorld;
out mat4 viewMVP;
out mat4 viewM;

void main()
{
    gl_Position = projection * view * transform * rotation * model[gl_InstanceID] * vec4(position, 1.0); 
    UV = vertexUV;
    
    viewMVP = view * transform * rotation * model[gl_InstanceID];
    viewM = view;
        
    // lighting
    vec4 v = vec4(position,1.0);
    normalWorld = (viewMVP  * vec4(normal, 0)).xyz;
    vertexPositionWorld = (viewMVP * v).xyz;
}

