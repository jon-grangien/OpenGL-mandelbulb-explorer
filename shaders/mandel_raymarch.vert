#version 330 core

layout (location = 0) in vec3 Position;

uniform mat4 projectionMatrix;
uniform mat4 modelViewMatrix;
uniform mat4 inverseMVP;
uniform float time;

out vec2 fragPos;
out float fragTime;

out vec3 vertRayDirection;

void main() {
    fragPos = Position.xy;
    fragTime = time;

    vec4 farPlane = inverseMVP * vec4(Position.xy, 2.0, 1.0);
    farPlane /= farPlane.w;

    vec4 nearPlane = inverseMVP * vec4(Position.xy, -2.0, 1.0);
    nearPlane /= nearPlane.w;

    //vertRayOrigin = nearPlane.xyz;
    vertRayDirection = (farPlane.xyz - nearPlane.xyz);

    // TODO: Quad invisible
    //gl_Position = projectionMatrix * modelViewMatrix * vec4(Position.xy, 0.0, 1.0);

    gl_Position = vec4(Position.xy, 0.0, 1.0);
}
