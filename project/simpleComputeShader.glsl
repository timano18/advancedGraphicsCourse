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
layout (location = 1) uniform float noiseScale;
layout (location = 2) uniform int screenWidth;

layout(std430, binding = 0) buffer VertexBuffer {

	Vertex vertices[];
};


// ----------------------------------------------------------------------------
//
// functions
//
// ----------------------------------------------------------------------------

const float M_PI = 3.1415926535897932384626433832795;

vec2 randomGradient(int ix, int iy) {
    int w = 8 * 8;
    int s = w / 2;
    int a = ix, b = iy;
    a *= 3284157443;
    b ^= a << s | a >> (w - s);
    b *= 1911520717;
    a ^= b << s | b >> (w - s);
    a *= 2048419325;
    float random = a * (M_PI / float(~0u >> 1));
    
    return vec2(sin(random), cos(random));
}

// Find dot product of vectors
float dotGridGradient(int ix, int iy, float x, float y)
{
	// Create random grad. for a square index
	vec2 gradient = randomGradient(ix, iy);

	// Find distance
	float dx = x - float(ix);
	float dy = y - float(iy);

	return (dx * gradient.x + dy * gradient.y);
}

// Interpolate two values with weight
float interpolate(float a0, float a1, float weight)
{
	return (a1 - a0) * (3.0 - weight * 2.0) * weight * weight + a0;
}

// Calculated value to divide hight by st avoid values of > 1
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
	float wx = x - float(x0);
	float wy = y - float(y0);

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

	int levels = 3;

	x = float(x) / (sizeX - 1);
	y = float(y) / (sizeY - 1);

	float normalizedP = 0.0;

	for (int i = levels; i > 0; i--) {
		normalizedP = normalizedP + (((perlinNoiseGen(x * pow(2, i), y * pow(2, i)) + 1.0) / 2.0) / i);
	}

	return (normalizedP / divHeight(levels)) * noiseScale;
}

void calculateNormals() {
	

		//vertices[i].normal = glm::normalize(vertices[i].normal);
}

//
// Description : Array and textureless GLSL 2D/3D/4D simplex 
//               noise functions.
//      Author : Ian McEwan, Ashima Arts.
//  Maintainer : stegu
//     Lastmod : 20201014 (stegu)
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
/*
// demo code:
float color(vec2 xy) { return 0.7 * snoise(vec3(xy, 0.3*iTime)); }
void mainImage(out vec4 fragColor, in vec2 fragCoord) {
    vec2 p = (fragCoord.xy/iResolution.y) * 2.0 - 1.0;

    vec3 xyz = vec3(p, 0);

    vec2 step = vec2(1.3, 1.7);
    float n = color(xyz.xy);
    n += 0.5 * color(xyz.xy * 2.0 - step);
    n += 0.25 * color(xyz.xy * 4.0 - 2.0 * step);
    n += 0.125 * color(xyz.xy * 8.0 - 3.0 * step);
    n += 0.0625 * color(xyz.xy * 16.0 - 4.0 * step);
    n += 0.03125 * color(xyz.xy * 32.0 - 5.0 * step);

    fragColor.xyz = vec3(0.5 + 0.5 * vec3(n, n, n));

}

*/

void main() {
	
    uint idx = gl_GlobalInvocationID.x;
   // if (idx >= 100) return;

    // Example modification: Add a sine wave effect to the z-position
    //vertices[idx].positionZ = perlinNoise(vertices[idx].positionX, vertices[idx].positionY, size, size, 500*noiseScale);
	vertices[idx].positionZ = 100 * snoise(vec3(vertices[idx].positionX, vertices[idx].positionY, 0) / noiseScale); 
	//vertices[idx].position.x += 100;
	//vertices[idx].position.y += 100;
	//vertices[idx].positionZ += 100;
	//vertices[idx].normal += vec3(10, 10, 10);
	/*
	vec3 off = vec3(1.0, 1.0, 0.0);
	float hL = perlinNoise(vertices[idx].positionX/10.0 - off.x, vertices[idx].positionY / 10.0 - off.z, size, size, noiseScale);
	float hR = perlinNoise(vertices[idx].positionX / 10.0 + off.x, vertices[idx].positionY / 10.0 + off.z, size, size, noiseScale);
	float hD = perlinNoise(vertices[idx].positionX / 10.0 - off.z, vertices[idx].positionY / 10.0 - off.y, size, size, noiseScale);
	float hU = perlinNoise(vertices[idx].positionX / 10.0 + off.z, vertices[idx].positionY / 10.0 + off.y, size, size, noiseScale);

	vertices[idx].normalX = hL - hR;
	vertices[idx].normalY = hD - hU;
	vertices[idx].normalZ = 0.02;
	vec3 Normals = normalize(vec3(vertices[idx].normalX, vertices[idx].normalY, vertices[idx].normalZ));
	vertices[idx].normalX = Normals.x;
	vertices[idx].normalY = Normals.y;
	vertices[idx].normalZ = Normals.z;
	*/
}