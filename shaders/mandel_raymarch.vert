#version 330 core

layout (location = 0) in vec3 Position;

uniform mat4 projectionMatrix;
uniform mat4 viewMatrix;
uniform mat4 modelViewMatrix;
uniform mat4 inverseMVP;
uniform float u_nearPlane;
uniform float u_farPlane;
uniform float time;

out vec2 fragPos;
out float fragTime;

out vec3 vertRayOrigin;
out vec3 vertRayDirection;

void main() {
    fragPos = Position.xy;
    fragTime = time;

    // Flip values for near and far until we figure out otherwise
    vec4 farPlane = inverseMVP * vec4(Position.xy, u_nearPlane, 1.0);
    vec4 nearPlane = inverseMVP * vec4(Position.xy, u_farPlane, 1.0);
    farPlane /= farPlane.w;
    nearPlane /= nearPlane.w;

    vertRayOrigin = nearPlane.xyz;
    vertRayDirection = (farPlane.xyz - nearPlane.xyz);

    gl_Position = vec4(Position.xy, 0.0, 1.0);
}
