#version 330 core

in vec3 vertRayOrigin;
in vec3 vertRayDirection;

uniform float u_time;
uniform float u_screenRatio;
uniform vec2 u_screenSize;

uniform float u_maxRaySteps;
uniform float u_minDistance;
uniform float u_mandelIters;
uniform float u_bailLimit;
uniform float u_power;

out vec4 outColor;

#define SPHERE_R 0.9
#define SCALE 0.5

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
	while (n < u_maxRaySteps) {
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
	for (int i = 0; i < u_mandelIters ; i++) {
		r = length(z);
		if (r > u_bailLimit) break;

		// convert to polar coordinates
		float theta = acos(z.z/r);
		float phi = atan(z.y,z.x);
		dr = pow(r, u_power-1.0)*u_power*dr + 1.0;

		// scale and rotate the point
		float zr = pow(r,u_power);
		theta = theta*u_power;
		phi = phi*u_power;

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

	for (steps=0; steps < u_maxRaySteps; steps++) {
		vec3 p = from + totalDistance * dir;
		float distance = DEMandelBulb(p);
		totalDistance += distance;

		if (distance < u_minDistance)
            break;
	}
	return (1.0 - float(steps) / float(u_maxRaySteps)); // greyscale val based on amount steps
}

void main() {
    // vec2 uv = fragPos.xy / screenSize.xy - vec2(0.5);

    // Estimate normal
    //vec3 n = normalize(vec3(DE(pos+xDir)-DE(pos-xDir),
    //                        DE(pos+yDir)-DE(pos-yDir),
    //                        DE(pos+zDir)-DE(pos-zDir)));

    float gsColor = simpleMarch(vertRayOrigin, vertRayDirection);

    // DEBUG: Check frag pos
    //if (uv.x <= 0.5) {
    //    outColor = vec4(1.0);
    //    return;
    //} else if (uv.x > 0.5) {
    //    outColor = vec4(0.0);
    //    return;
    //}

    outColor = vec4(vec3(gsColor), 1.0);
}
