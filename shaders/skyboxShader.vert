#version 410 core

layout (location = 0) in vec3 vertexPosition;
out vec3 textureCoordinates;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model; // Make sure this matrix includes the scaling transformation

void main()
{
    // Apply the scaling transformation to the vertex position
    vec4 scaledPosition = vec4(vertexPosition, 1.0) * model;

    // Calculate the final position
    vec4 tempPos = projection * view * scaledPosition;

    // Set gl_Position and pass textureCoordinates
   gl_Position = projection * view * vec4(vertexPosition, 1.0);

    textureCoordinates = vertexPosition;
}
