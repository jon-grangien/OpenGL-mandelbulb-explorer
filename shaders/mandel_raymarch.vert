#version 330 core

layout (location = 0) in vec3 Position;

uniform mat4 projectionMatrix;
uniform mat4 modelViewMatrix;
uniform float time;

out vec2 fragPos;
out float fragTime;

void main() {
    fragPos = Position.xy;
    fragTime = time;

    // TODO: Quad invisible
    //gl_Position = projectionMatrix * modelViewMatrix * vec4(Position.xy, 0.0, 1.0);

    gl_Position = vec4(Position.xy, 0.0, 1.0);
}
