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
#define LOW_P_ZERO 0.00001

vec3 glowColor = vec3(0.75, 0.9, 1.0);
vec3 divColorA = vec3(0.69, 0.55, 0.76);
vec3 divColorB = vec3(0.5, 0.75, 0.48);

//
// Description : Array and textureless GLSL 2D/3D/4D simplex
//               noise functions.
//      Author : Ian McEwan, Ashima Arts.
//  Maintainer : stegu
//     Lastmod : 20110822 (ijm)
//     License : Copyright (C) 2011 Ashima Arts. All rights reserved.
//               Distributed under the MIT License. See LICENSE file.
//               https://github.com/ashima/webgl-noise
//               https://github.com/stegu/webgl-noise
//

vec3 mod289(vec3 x) {
  return x - floor(x * (1.0 / 289.0)) * 289.0;
}

vec4 mod289(vec4 x) {
  return x - floor(x * (1.0 / 289.0)) * 289.0;
}

vec4 permute(vec4 x) {
     return mod289(((x*34.0)+1.0)*x);
}

vec4 taylorInvSqrt(vec4 r)
{
  return 1.79284291400159 - 0.85373472095314 * r;
}

float snoise(vec3 v)
  {
  const vec2  C = vec2(1.0/6.0, 1.0/3.0) ;
  const vec4  D = vec4(0.0, 0.5, 1.0, 2.0);

// First corner
  vec3 i  = floor(v + dot(v, C.yyy) );
  vec3 x0 =   v - i + dot(i, C.xxx) ;

// Other corners
  vec3 g = step(x0.yzx, x0.xyz);
  vec3 l = 1.0 - g;
  vec3 i1 = min( g.xyz, l.zxy );
  vec3 i2 = max( g.xyz, l.zxy );

  //   x0 = x0 - 0.0 + 0.0 * C.xxx;
  //   x1 = x0 - i1  + 1.0 * C.xxx;
  //   x2 = x0 - i2  + 2.0 * C.xxx;
  //   x3 = x0 - 1.0 + 3.0 * C.xxx;
  vec3 x1 = x0 - i1 + C.xxx;
  vec3 x2 = x0 - i2 + C.yyy; // 2.0*C.x = 1/3 = C.y
  vec3 x3 = x0 - D.yyy;      // -1.0+3.0*C.x = -0.5 = -D.y

// Permutations
  i = mod289(i);
  vec4 p = permute( permute( permute(
             i.z + vec4(0.0, i1.z, i2.z, 1.0 ))
           + i.y + vec4(0.0, i1.y, i2.y, 1.0 ))
           + i.x + vec4(0.0, i1.x, i2.x, 1.0 ));

// Gradients: 7x7 points over a square, mapped onto an octahedron.
// The ring size 17*17 = 289 is close to a multiple of 49 (49*6 = 294)
  float n_ = 0.142857142857; // 1.0/7.0
  vec3  ns = n_ * D.wyz - D.xzx;

  vec4 j = p - 49.0 * floor(p * ns.z * ns.z);  //  mod(p,7*7)

  vec4 x_ = floor(j * ns.z);
  vec4 y_ = floor(j - 7.0 * x_ );    // mod(j,N)

  vec4 x = x_ *ns.x + ns.yyyy;
  vec4 y = y_ *ns.x + ns.yyyy;
  vec4 h = 1.0 - abs(x) - abs(y);

  vec4 b0 = vec4( x.xy, y.xy );
  vec4 b1 = vec4( x.zw, y.zw );

  //vec4 s0 = vec4(lessThan(b0,0.0))*2.0 - 1.0;
  //vec4 s1 = vec4(lessThan(b1,0.0))*2.0 - 1.0;
  vec4 s0 = floor(b0)*2.0 + 1.0;
  vec4 s1 = floor(b1)*2.0 + 1.0;
  vec4 sh = -step(h, vec4(0.0));

  vec4 a0 = b0.xzyw + s0.xzyw*sh.xxyy ;
  vec4 a1 = b1.xzyw + s1.xzyw*sh.zzww ;

  vec3 p0 = vec3(a0.xy,h.x);
  vec3 p1 = vec3(a0.zw,h.y);
  vec3 p2 = vec3(a1.xy,h.z);
  vec3 p3 = vec3(a1.zw,h.w);

//Normalise gradients
  vec4 norm = taylorInvSqrt(vec4(dot(p0,p0), dot(p1,p1), dot(p2, p2), dot(p3,p3)));
  p0 *= norm.x;
  p1 *= norm.y;
  p2 *= norm.z;
  p3 *= norm.w;

// Mix final noise value
  vec4 m = max(0.6 - vec4(dot(x0,x0), dot(x1,x1), dot(x2,x2), dot(x3,x3)), 0.0);
  m = m * m;
  return 42.0 * dot( m*m, vec4( dot(p0,x0), dot(p1,x1),
                                dot(p2,x2), dot(p3,x3) ) );
  }

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

    // Alternate method to spherical
		// float theta = asin( z.z/r );
    // float phi = atan( z.y,z.x );

		dr = pow(r, u_power-1.0)*u_power*dr + 1.0;

		// scale and rotate the point
		float zr = pow(r,u_power);
		theta = theta*u_power;
		phi = phi*u_power;

		// convert back to cartesian coordinates
		z = zr*vec3(sin(theta)*cos(phi), sin(phi)*sin(theta), cos(theta));

    // Alternate method to spherical
		// z = zr*vec3( cos(theta)*cos(phi), cos(theta)*sin(phi), sin(theta) );

		z += pos;
	}
	return 0.5*log(r)*r/dr;
}

// March with distance estimate and return grayscale value
float simpleMarch(vec3 from, vec3 dir, out int stepsTaken, out vec3 pos) {
	float totalDistance = 0.0;
	int steps;
	vec3 p;

	for (steps=0; steps < u_maxRaySteps; steps++) {
		p = from + totalDistance * dir;
		float distance = DEMandelBulb(p);
		totalDistance += distance;

		if (distance < u_minDistance)
            break;
	}

	stepsTaken = steps;
	pos = p;
	return (1.0 - float(steps) / float(u_maxRaySteps)); // greyscale val based on amount steps
}

void main() {
    vec2 uv = gl_FragCoord.xy / u_screenSize.xy;

    // Estimate normal
    //vec3 n = normalize(vec3(DE(pos+xDir)-DE(pos-xDir),
    //                        DE(pos+yDir)-DE(pos-yDir),
    //                        DE(pos+zDir)-DE(pos-zDir)));

    int stepsTaken = 0;
    vec3 color;
    vec3 mandelPos;
    float gsColor = simpleMarch(vertRayOrigin, vertRayDirection, stepsTaken, mandelPos);

    // Ray miss; bg plane color
    if (gsColor < LOW_P_ZERO) {
      color = mix(divColorA, divColorA*0.7, uv.y);

    // Ray hit, mix some blue
    } else if (gsColor > 0.1) {
      float noise = snoise(5.0 * mandelPos);
      noise += 0.5 * snoise(10.0 * mandelPos);
      noise += 0.25 * snoise(20.0 * mandelPos);

      vec3 c1 = vec3(0.4+0.3*noise*sin(u_time), 0.2*noise, 1.0);
      vec3 c2 = vec3(1.0) - 0.05 * noise;
      color = mix(c1, c2, gsColor);

    // Fog applied
    } else {
      color = mix(glowColor, vec3(gsColor), gsColor);
    }

    outColor = vec4(color, 1.0);
}
