#version 420 core

layout (location = 0) in vec3 Position;

uniform mat4 modelViewMatrix;
uniform mat4 projectionMatrix;

const vec2 quadVertices[4] = { vec2(-1.0, -1.0), vec2(1.0, -1.0), vec2(-1.0, 1.0), vec2(1.0, 1.0) };

void main() {
    //gl_Position = projectionMatrix * modelViewMatrix * vec4(Position, 1.0);
    //gl_Position = projectionMatrix * modelViewMatrix * vec4(quadVertices[gl_VertexID], 0.0, 1.0);
    //gl_Position = vec4(Position.xy, 0.0, 1.0);
    gl_Position = vec4(quadVertices[gl_VertexID].xy, 0.0, 1.0);
}
