#version 330 core

in vec2 fragPos;
in float fragTime;

uniform float time;
uniform float screenRatio;
uniform vec2 screenSize;

out vec4 outColor;

#define MAX_RAY_STEPS 200
#define MIN_DISTANCE 0.05

// March with distance estimate and return grayscale value
//float simpleMarch(vec3 from, vec3 dir) {
//	float totalDistance = 0.0;
//	int steps;
//	for (steps=0; steps < MAX_RAY_STEPS; steps++) {
//		vec3 p = from + totalDistance * dir;
//		float distance = DistanceEstimator(p);
//		totalDistance += distance;
//		if (distance < MIN_DISTANCE) break;
//	}
//	return (1.0 - float(steps) / float(MAX_RAY_STEPS)); // greyscale val based on amount steps
//}

void main() {
    vec2 uv = fragPos + 0.5; // depends on quad input data

    // Estimate normal
    //vec3 n = normalize(vec3(DE(pos+xDir)-DE(pos-xDir),
    //                        DE(pos+yDir)-DE(pos-yDir),
    //                        DE(pos+zDir)-DE(pos-zDir)));

    outColor = vec4(uv.x * sin(fragTime), 0.5, uv.y * cos(fragTime), 1.0);
}
