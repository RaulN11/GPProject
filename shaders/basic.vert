#version 410 core

layout(location = 0) in vec3 vPosition; // Vertex position
layout(location = 1) in vec3 vNormal;   // Vertex normal
layout(location = 2) in vec2 vTexCoords; // Vertex texture coordinates

out vec3 fPosition;
out vec3 fNormal;
out vec2 fTexCoords;
out vec3 worldPos; // Pass world position to fragment shader

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {
    fPosition = vPosition;
    fNormal = vNormal;
    fTexCoords = vTexCoords;

    vec4 worldPosition = model * vec4(vPosition, 1.0);
    worldPos = vec3(worldPosition); // Compute world position of the vertex

    gl_Position = projection * view * worldPosition;
}
