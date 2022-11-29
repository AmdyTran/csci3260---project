#version 330 core

out vec4 Color;

in vec2 UV;

uniform sampler2D textureAsteroid;

// for diffuse lighting
in vec3 normalWorld;
in vec3 vertexPositionWorld;
in mat4 viewMVP;

uniform vec3 lightPositionWorld;
uniform vec3 eyePosition;

void main()
{    
    Color = texture(textureAsteroid, UV);
    
    vec3 normal = normalize(normalWorld);
    
    // diffuse
    vec3 finalLightPosition = (viewMVP * vec4(lightPositionWorld, 1.0)).xyz;
    
    vec3 lightVectorWorld = normalize(vertexPositionWorld - finalLightPosition);
    float brightness = dot(lightVectorWorld, normal);
    vec4 diffuseLight = vec4(brightness * 230/255.0, brightness * 180/255.0, brightness * 60/255.0, 1.0);
    // fix the light color
    
    // specular
    // calc direction
    vec3 reflectedVector = reflect(-lightVectorWorld, normal);
    //calc direction from eye to obj
    vec3 eyeObj = normalize(eyePosition - vertexPositionWorld);
    // calc brightness
    float s = clamp(dot(reflectedVector, eyeObj), 0, 1);
    // control lobe
    s = pow(s, 5);
    vec4 specularLight = vec4(s, s * 245/255.0, s * 240/255.0, 1.0);
    
    vec4 factor = vec4(0.5f, 0.5f, 0.5f, 1);
    
    // add together
    
    Color = 0.3 * clamp(diffuseLight, 0, 1.0)
    + vec4(0.3f, 0.3f, 0.3f, 1) * specularLight
    + factor * Color;
}
