#version 430

out vec4 Color;

in vec2 UV;

uniform sampler2D textureAsteroid;

// for diffuse lighting
in vec3 normalWorld;
in vec3 vertexPositionWorld;
in mat4 viewMVP;
in mat4 viewM;

// lighting
uniform int spotOn;

uniform vec3 lightPositionWorld;
uniform vec3 eyePosition;
uniform vec3 spotDirection;
uniform float innerCutoff;
uniform float outerCutoff;

void main()
{    
    float theta;
    
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
    
    // spotlight
    
    if (spotOn == 1) {
        vec3 spotVec = vec3(viewM * vec4(eyePosition, 1.0)) - vertexPositionWorld;
        theta = dot(normalize(spotVec), normalize(spotDirection));
        
        float intensity = clamp((theta - outerCutoff) / (innerCutoff - outerCutoff), 0.0, 0.5);
        
        Color += vec4(intensity/1.6, intensity/1.7, intensity * 1.5, 1) * clamp(diffuseLight, 0, 1.0)
        + vec4(intensity/1.2, intensity/1.2, intensity * 1.5, 1) * specularLight;
    }
}
