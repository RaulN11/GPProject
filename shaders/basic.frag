#version 410 core

in vec3 fPosition;
in vec3 fNormal;
in vec2 fTexCoords;

out vec4 fColor;

// Matrices
uniform mat4 model;
uniform mat4 view;
uniform mat3 normalMatrix;

// Directional light properties (global light)
uniform vec3 lightDir;    // Direction of the global light
uniform vec3 lightColor;  // Color of the global light (yellow)

// Point light properties (camera light)
uniform vec3 cameraPos;   // Camera position as light source position
uniform vec3 pointLightColor; // Color of the point light (red)

// Textures
uniform sampler2D diffuseTexture;
uniform sampler2D specularTexture;

// Material properties
float ambientStrength = 0.2f;
float specularStrength = 0.5f;

// Fog properties
uniform bool useFog;
uniform vec3 fogColor;
uniform float fogDensity;

// Function to calculate directional (global) light contribution
vec3 calculateDirLight() {
    vec3 norm = normalize(normalMatrix * fNormal);
    vec3 lightDirNormalized = normalize(-lightDir);
    float diff = max(dot(norm, lightDirNormalized), 0.0);
    vec3 diffuse = diff * lightColor;

    vec3 viewDir = normalize(-fPosition);
    vec3 reflectDir = reflect(-lightDirNormalized, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = specularStrength * spec * lightColor;

    return (ambientStrength + diffuse + specular) * texture(diffuseTexture, fTexCoords).rgb;
}

// Function to calculate point light (camera light) contribution
vec3 calculatePointLight() {
    vec3 norm = normalize(normalMatrix * fNormal);
    vec3 lightDir = normalize(cameraPos - fPosition);
    float distance = length(cameraPos - fPosition);
    float attenuation = 1.0 / (1.0 + 0.09 * distance + 0.032 * distance * distance);

    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * pointLightColor * attenuation;

    vec3 viewDir = normalize(-fPosition);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = specularStrength * spec * pointLightColor * attenuation;

    return (diffuse + specular) * texture(diffuseTexture, fTexCoords).rgb;
}

// Function to calculate fog factor
float calculateFogFactor() {
    float distance = length(cameraPos - vec3(view * model * vec4(fPosition, 1.0f)));
    float fogFactor = exp(-pow(distance * fogDensity, 2.0));
    return clamp(fogFactor, 0.0, 1.0);
}

void main() {
    vec4 texColor = texture(diffuseTexture, fTexCoords);

    if(texColor.a < 0.1) {
        discard;
    }
    vec3 dirLightEffect = calculateDirLight();
    vec3 pointLightEffect = calculatePointLight();

    vec3 objectColor = min(dirLightEffect + pointLightEffect, 1.0f);

    if (useFog) {
        float fogFactor = calculateFogFactor();
        vec3 finalColor = mix(objectColor, fogColor, 1.0 - fogFactor);
        fColor = vec4(finalColor, 1.0);
    } else {
        fColor = vec4(objectColor, 1.0);
    }
}
