#version 400 core

layout (location = 0) in vec3 Position;

uniform mat4 u_inverseVP;
uniform float u_nearPlane;
uniform float u_farPlane;

out float fragTime;

out vec3 vertRayOrigin;
out vec3 vertRayDirection;

void main() {

    // Correct setup with artifacts
    //vec4 farPlane = u_inverseVP * vec4(Position.xy, u_farPlane, 1.0);
    //vec4 nearPlane = u_inverseVP * vec4(Position.xy, u_nearPlane, 1.0);

    // Incorrect setup with less artifacts
    vec4 farPlane = u_inverseVP * vec4(Position.xy, u_nearPlane, 1.0);
    vec4 nearPlane = u_inverseVP * vec4(Position.xy, u_farPlane, 1.0);

    farPlane /= farPlane.w;
    nearPlane /= nearPlane.w;

    vertRayOrigin = nearPlane.xyz;
    vertRayDirection = (farPlane.xyz - nearPlane.xyz);

    gl_Position = vec4(Position.xy, 0.0, 1.0);
}
