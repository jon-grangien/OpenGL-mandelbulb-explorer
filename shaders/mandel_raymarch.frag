#version 330 core

in vec2 fragPos;
in float fragTime;

uniform float time;
uniform float screenRatio;
uniform vec2 screenSize;

out vec4 outColor;

void main() {
    vec2 uv = fragPos + 0.5; //?
    outColor = vec4(uv.x * sin(fragTime), 0.5, uv.y * cos(fragTime), 1.0);
}
