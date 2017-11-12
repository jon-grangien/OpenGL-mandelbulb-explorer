#version 330 core

in vec3 Position;
in vec3 Normal;
in vec2 TexCoord;

uniform mat4 modelViewMatrix;
uniform mat4 projectionMatrix;

out vec2 outTexCoord;

void main() {
    outTexCoord = TexCoord;
    gl_Position = projectionMatrix * modelViewMatrix * vec4(Position, 1.0);
}
