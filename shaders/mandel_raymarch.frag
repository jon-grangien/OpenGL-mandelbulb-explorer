#version 330 core

in vec2 fragPos;
in float fragTime;

uniform float time;
uniform float screenRatio;
uniform vec2 screenSize;

out vec4 outColor;

#define MAX_RAY_STEPS 400
#define MIN_DISTANCE 0.0001
#define MANDEL_ITERS 20
#define BAIL_LIMIT 2.5
#define POWER 8.0

#define SPHERE_R 0.9
#define SCALE 2.0

float DESphere(vec3 p) {
    return length(p) - SPHERE_R;
}

float DERecTetra(vec3 p) {
	vec3 a1 = vec3(1,1,1);
	vec3 a2 = vec3(-1,-1,1);
	vec3 a3 = vec3(1,-1,-1);
	vec3 a4 = vec3(-1,1,-1);
	vec3 c;
	int n = 0;
	float dist, d;
	vec3 z;
	while (n < MAX_RAY_STEPS) {
		 c = a1; dist = length(z-a1);
	        d = length(z-a2); if (d < dist) { c = a2; dist=d; }
		 d = length(z-a3); if (d < dist) { c = a3; dist=d; }
		 d = length(z-a4); if (d < dist) { c = a4; dist=d; }
		z = SCALE*z-c*(SCALE-1.0);
		n++;
	}

	return length(z) * pow(SCALE, float(-n));
}

float DEMandelBulb(vec3 pos) {
	vec3 z = pos;
	float dr = 1.0;
	float r = 0.0;
	for (int i = 0; i < MANDEL_ITERS ; i++) {
		r = length(z);
		if (r > BAIL_LIMIT) break;

		// convert to polar coordinates
		float theta = acos(z.z/r);
		float phi = atan(z.y,z.x);
		dr = pow( r, POWER-1.0)*POWER*dr + 1.0;

		// scale and rotate the point
		float zr = pow( r,POWER);
		theta = theta*POWER;
		phi = phi*POWER;

		// convert back to cartesian coordinates
		z = zr*vec3(sin(theta)*cos(phi), sin(phi)*sin(theta), cos(theta));
		z += pos;
	}
	return 0.5*log(r)*r/dr;
}

// March with distance estimate and return grayscale value
float simpleMarch(vec3 from, vec3 dir) {
	float totalDistance = 0.0;
	int steps;

	for (steps=0; steps < MAX_RAY_STEPS; steps++) {
		vec3 p = from + totalDistance * dir;
		float distance = DEMandelBulb(p);
		totalDistance += distance;

		if (distance < MIN_DISTANCE)
            break;
	}
	return (1.0 - float(steps) / float(MAX_RAY_STEPS)); // greyscale val based on amount steps
}

void main() {
    // Middle circle unadjusted, incorrect uv
    vec2 uv = fragPos.xy;
    //uv *= screenRatio;

    // Correct uv but circle cornered?
    //vec2 uv = fragPos.xy + vec2(0.5); // depends on quad input data
    //uv *= screenRatio;

    // Correct uv but no circle
    //vec2 uv = fragPos.xy / screenSize.xy - vec2(0.5);

    // Estimate normal
    //vec3 n = normalize(vec3(DE(pos+xDir)-DE(pos-xDir),
    //                        DE(pos+yDir)-DE(pos-yDir),
    //                        DE(pos+zDir)-DE(pos-zDir)));

    vec3 from = vec3(uv.x, uv.y, 0.0);
    vec3 dir = vec3(0.0, 0.0, 1.0);
    float gsColor = simpleMarch(from, dir);

    //outColor = vec4(uv.x * sin(fragTime), 0.5, uv.y * sin(fragTime), 1.0);
    outColor = vec4(vec3(gsColor), 1.0);

    // Test frag pos
    //if (uv.y <= 0.5) {
    //    outColor = vec4(1.0);
    //} else if (uv.y > 0.5) {
    //    outColor = vec4(vec3(0.0), 1.0);
    //}

}
