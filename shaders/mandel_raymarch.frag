#version 330 core

in vec2 fragPos;
in float fragTime;

out vec4 out_color;

void main() {
    out_color = vec4(fragPos.x * sin(fragTime), 0.5, fragPos.y * cos(fragTime), 1.0);
}
