#version 430 core

layout(local_size_x = 100) in;

// Structures

struct Vertex {
	float positionX;
	float positionY;
	float positionZ;
	float normalX;
	float normalY;
	float normalZ;
};

// ----------------------------------------------------------------------------
//
// uniforms
//
// ----------------------------------------------------------------------------



layout (location = 0) uniform int size;
layout (location = 1) uniform float simpexScale;
layout (location = 2) uniform float worleyScale;
layout (location = 3) uniform float simpexAmplitude;
layout (location = 4) uniform float worleyAmplitude;
layout (location = 5) uniform float ratio;
layout (location = 6) uniform vec3 translation;

layout(std430, binding = 0) buffer VertexBuffer {

	Vertex vertices[];
};

layout(std430, binding = 1) buffer VertexFollowBuffer {

	vec3 vertexFollow;
};



// ----------------------------------------------------------------------------
//
// functions
//
// ----------------------------------------------------------------------------
#define pi 3.14159265
// Generate randomness
vec2 randomGradient(int ix, int iy) {
	// No precomputed gradients mean this works for any number of grid coordinates
	int w = 8 * 8;
	int s = w / 2;
	int a = ix, b = iy;
	a *= 3284157443;

	b ^= a << s | a >> w - s;
	b *= 1911520717;

	a ^= b << s | b >> w - s;
	a *= 2048419325;
	float random = a * (pi / ~(~0u >> 1)); // in [0, 2*Pi]

	// Create the vector from the angle
	vec2 v;
	v.x = sin(random);
	v.y = cos(random);

	return v;
}

// Find dot product of vectors
float dotGridGradient(int ix, int iy, float x, float y)
{
	// Create random grad. for a square index
	vec2 gradient = randomGradient(ix, iy);

	// Find distance
	float dx = (x - float(ix));
	float dy = (y - float(iy));

	return (dx * gradient.x + dy * gradient.y);
}

// Interpolate two values with weight
float interpolate(float a0, float a1, float weight)
{
	return (a1 - a0) * (3.0 - weight * 2.0) * weight * weight + a0;
}

// Calculated value to divide height by st avoid values of > 1
float divHeight(int levels) {
	float divValue = 1;
	for (int i = 1; i < levels; i++) {
		divValue = divValue + (1 / pow(2, i));
	}
	return divValue;
}

// Input coords. to generate perlin in point
float perlinNoiseGen(float x, float y) {

	// Find square corners
	int x0 = int(x);
	int y0 = int(y);
	int x1 = x0 + 1;
	int y1 = y0 + 1;

	// Interpolation weight (from size of square)
	float wx = abs(x - float(x0));
	float wy = abs(y - float(y0));

	// Generate + interpolate top corners
	float n0 = dotGridGradient(x0, y0, x, y);
	float n1 = dotGridGradient(x1, y0, x, y);
	float ix0 = interpolate(n0, n1, wx);

	// Generate + interpolate bot corners
	n0 = dotGridGradient(x0, y1, x, y);
	n1 = dotGridGradient(x1, y1, x, y);
	float ix1 = interpolate(n0, n1, wx);

	// Interpolate between top and bot
	float perlinValue = interpolate(ix0, ix1, wy);

	return perlinValue;
}

// Generate perlin map
float perlinNoise(float x, float y, int sizeX, int sizeY, float noiseScale) {

	int levels = 5;

	x = abs(x / float(sizeX - 1));
	y = abs(y / float(sizeY - 1));

	float normalizedP = 0.0;

	for (int i = levels; i > 0; i--) {
		normalizedP = normalizedP + (((perlinNoiseGen(x * pow(2, i), y * pow(2, i)) + 1.0) / 2.0) / i);
	}
	//if (normalizedP > 0.75 || normalizedP < -0.25) std::cout << "Perlin value: " << normalizedP << '\n';
	//if (normalizedP / divHeight(levels) * 2 > 0.9 || normalizedP / divHeight(levels) * 2 < -0.1) std::cout << "Perlin value: " << normalizedP / divHeight(levels) << '\n';
	return (normalizedP / divHeight(levels)) * noiseScale; // Behövs divHeight??
}


vec3 mod289(vec3 x) {
  return x - floor(x * (1.0 / 289.0)) * 289.0;
}

vec4 mod289(vec4 x) {
  return x - floor(x * (1.0 / 289.0)) * 289.0;
}

vec4 permute(vec4 x) {
     return mod289(((x*34.0)+10.0)*x);
}

vec4 taylorInvSqrt(vec4 r)
{
  return 1.79284291400159 - 0.85373472095314 * r;
}

float snoise(vec3 v)
  { 
  const vec2  C = vec2(1.0/6.0, 1.0/3.0) ;
  const vec4  D = vec4(0.0, 0.5, 1.0, 2.0);
  if(v.x == 0 && v.y == 0) 
  {
	  v.x = 0.00000001;
	  v.y = 0.00000001;
  }
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
  vec4 m = max(0.5 - vec4(dot(x0,x0), dot(x1,x1), dot(x2,x2), dot(x3,x3)), 0.0);
  m = m * m;
  return 105.0 * dot( m*m, vec4( dot(p0,x0), dot(p1,x1), 
                                dot(p2,x2), dot(p3,x3) ) );
  }




  

// Cellular noise ("Worley noise") in 2D in GLSL.
// Copyright (c) Stefan Gustavson 2011-04-19. All rights reserved.
// This code is released under the conditions of the MIT license.
// See LICENSE file for details.
// https://github.com/stegu/webgl-noise

// Modulo 289 without a division (only multiplications)
vec2 mod289(vec2 x) {
  return x - floor(x * (1.0 / 289.0)) * 289.0;
}

// Modulo 7 without a division
vec4 mod7(vec4 x) {
  return x - floor(x * (1.0 / 7.0)) * 7.0;
}


// Cellular noise, returning F1 and F2 in a vec2.
// Speeded up by using 2x2 search window instead of 3x3,
// at the expense of some strong pattern artifacts.
// F2 is often wrong and has sharp discontinuities.
// If you need a smooth F2, use the slower 3x3 version.
// F1 is sometimes wrong, too, but OK for most purposes.
vec2 cellular2x2(vec2 P) {
#define K 0.142857142857 // 1/7
#define K2 0.0714285714285 // K/2
#define jitter 0.8 // jitter 1.0 makes F1 wrong more often
	vec2 Pi = mod289(floor(P));
 	vec2 Pf = fract(P);
	vec4 Pfx = Pf.x + vec4(-0.5, -1.5, -0.5, -1.5);
	vec4 Pfy = Pf.y + vec4(-0.5, -0.5, -1.5, -1.5);
	vec4 p = permute(Pi.x + vec4(0.0, 1.0, 0.0, 1.0));
	p = permute(p + Pi.y + vec4(0.0, 0.0, 1.0, 1.0));
	vec4 ox = mod7(p)*K+K2;
	vec4 oy = mod7(floor(p*K))*K+K2;
	vec4 dx = Pfx + jitter*ox;
	vec4 dy = Pfy + jitter*oy;
	vec4 d = dx * dx + dy * dy; // d11, d12, d21 and d22, squared
	// Sort out the two smallest distances
#if 0
	// Cheat and pick only F1
	d.xy = min(d.xy, d.zw);
	d.x = min(d.x, d.y);
	return vec2(sqrt(d.x)); // F1 duplicated, F2 not computed
#else
	// Do it right and find both F1 and F2
	d.xy = (d.x < d.y) ? d.xy : d.yx; // Swap if smaller
	d.xz = (d.x < d.z) ? d.xz : d.zx;
	d.xw = (d.x < d.w) ? d.xw : d.wx;
	d.y = min(d.y, d.z);
	d.y = min(d.y, d.w);
	return sqrt(d.xy);
#endif
}
float color(vec2 xy) { return 0.7 * snoise(vec3(xy, 0.3*0)); }

float simpexOctave(vec2 pos) {

	vec2 step = vec2(1.3, 1.7);
    float n = color(pos);
    n += 0.5 * color(pos * 2.0 - step);
	n += 0.25 * color(pos * 4.0 - 2.0 * step);
	n += 0.125 * color(pos * 8.0 - 3.0 * step);
	n += 0.0625 * color(pos * 16.0 - 4.0 * step);
	n += 0.03125 * color(pos * 32.0 - 5.0 * step);
	n += 0.015625 * color(pos * 64.0 -6.0 * step);

	
	return n;
}

float calculateZ(vec2 pos) {


	float z = 
	ratio * (simpexAmplitude * simpexOctave(pos / simpexScale).x) + 
	(1-ratio) * (worleyAmplitude * cellular2x2(vec2(pos.x, pos.y) / worleyScale).x);
	return z;
}

void main() {
	float cellsize = 10.0;
	
	

    uint idx = gl_GlobalInvocationID.x;
 	vertices[idx].positionX += (translation.x * cellsize);
    vertices[idx].positionY += (translation.y * cellsize);

	vertices[idx].positionZ = calculateZ(vec2(vertices[idx].positionX, vertices[idx].positionY));
	//vertices[idx].positionZ += 30;

	vertexFollow = vec3(vertices[0].positionX, vertices[0].positionY, vertices[0].positionZ);

    //vertices[idx].positionZ += translation.z;





	vec3 off = vec3(1.0, 1.0, 0.0) * cellsize;
	float hL = calculateZ(vec2(vertices[idx].positionX - off.x, vertices[idx].positionY - off.z));
	float hR = calculateZ(vec2(vertices[idx].positionX + off.x, vertices[idx].positionY + off.z));
	float hD = calculateZ(vec2(vertices[idx].positionX - off.z, vertices[idx].positionY - off.y));
	float hU = calculateZ(vec2(vertices[idx].positionX + off.z, vertices[idx].positionY + off.y));


	vertices[idx].normalX = hL - hR;
	vertices[idx].normalY = hD - hU;
	vertices[idx].normalZ = 20.0 * cellsize;
	
	vec3 normalized = normalize(vec3(vertices[idx].normalX,vertices[idx].normalY,vertices[idx].normalZ));
	

	

}