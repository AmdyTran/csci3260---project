#version 330 core

out vec4 Color;

in vec2 UV;

uniform sampler2D textureSpacecraft;

// for diffuse lighting
in vec3 normalWorld;
in vec3 vertexPositionWorld;

uniform vec3 lightPositionWorld;
uniform vec3 eyePosition;

void main()
{    
    Color = texture(textureSpacecraft, UV);
    
    // diffuse
    vec3 lightVectorWorld = normalize(lightPositionWorld - vertexPositionWorld);
    float brightness = dot(lightVectorWorld, normalize(normalWorld));
    vec4 diffuseLight = vec4(brightness * 230/255.0, brightness * 180/255.0, brightness * 60/255.0, 1.0);
    // fix the light color
    
    // specular
    // calc direction
    vec3 reflectedVector = reflect(-lightVectorWorld, normalWorld);
    //calc direction from eye to obj
    vec3 eyeObj = normalize(eyePosition - vertexPositionWorld);
    // calc brightness
    float s = clamp(dot(reflectedVector, eyeObj), 0, 1);
    // control lobe
    s = pow(s, 200);
    vec4 specularLight = vec4(s, s * 245/255.0, s * 200/255.0, 1.0);
    
    vec4 factor = vec4(0.5f, 0.5f, 0.5f, 1);
    
    // add together
    
    Color = factor * clamp(diffuseLight, 0, 1.0)
    + factor * specularLight
    + vec4(0.7f, 0.7f, 0.7f, 1) * Color;
    
}
