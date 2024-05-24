

#version 430 core

layout (local_size_x = 10, local_size_y = 10, local_size_z = 1) in;

// ----------------------------------------------------------------------------
//
// uniforms
//
// ----------------------------------------------------------------------------

layout(rgba32f, binding = 0) uniform image2D imgOutput;

layout (location = 0) uniform float t;                 /** Time */
layout (location = 1) uniform vec3 sunAngle;
uniform mat4 invProjectionMatrix;
uniform mat4 viewMatrix;

// ----------------------------------------------------------------------------
//
// functions
//
// ----------------------------------------------------------------------------





vec3 sunDirection = normalize(vec3(0.0, 1.0, 0.0));
float earthRadius = 6360e3;
float atmosphereRadius = 6420e3;
float Hr = 7994;
float Hm = 1200;
vec3 betaR = vec3(3.8e-6, 13.5e-6, 33.1e-6);
vec3 betaM = vec3(21e-6);
#define M_PI (3.14159265358979323846)

bool solveQuadratic(float a, float b, float c, out float x1, out float x2)
{
    if (b == 0) {
        // Handle special case where the the two vector ray.dir and V are perpendicular
        // with V = ray.orig - sphere.centre
        if (a == 0) return false;
        x1 = 0; x2 = sqrt(-c / a);
        return true;
    }
    float discr = b * b - 4 * a * c;

    if (discr < 0) return false;

    float q = (b < 0.f) ? -0.5f * (b - sqrt(discr)) : -0.5f * (b + sqrt(discr));
    x1 = q / a;
    x2 = c / q;

    return true;
}
bool raySphereIntersect(vec3 orig, vec3 dir, float radius, out float t0, out float t1)
{
    // They ray dir is normalized so A = 1 
    float A = dir.x * dir.x + dir.y * dir.y + dir.z * dir.z;
    float B = 2 * (dir.x * orig.x + dir.y * orig.y + dir.z * orig.z);
    float C = orig.x * orig.x + orig.y * orig.y + orig.z * orig.z - radius * radius;

    if (!solveQuadratic(A, B, C, t0, t1)) return false;

    if (t0 > t1)
    {
        float temp = t0; 
        t0 = t1;
        t1 = temp;
    }

    return true;
}
vec3 computeIncidentLight(vec3 orig, vec3 dir, float tmin, float tmax)
{
    
    float t0, t1;
   
    if (!raySphereIntersect(orig, dir, atmosphereRadius, t0, t1) || t1 < 0) return vec3(1.0, 0.0, 0.0);
   
    
    if (t0 > tmin && t0 > 0) tmin = t0;
    
    if (t1 < tmax) tmax = t1;
  
    int numSamples = 16;
    int numSamplesLight = 8;
    float segmentLength = (tmax - tmin) / numSamples;
    float tCurrent = tmin;
   
    vec3 sumR = vec3(0.0);
    vec3 sumM = vec3(0.0); // mie and rayleigh contribution
    float opticalDepthR = 0, opticalDepthM = 0;
    float mu = dot(dir, sunAngle); // mu in the paper which is the cosine of the angle between the sun direction and the ray direction
    float phaseR = 3.0 / (16.0 * M_PI) * (1.0 + mu * mu);
    float g = 0.76;
    float phaseM = 3.0 / (8.0 * M_PI) * ((1.0 - g * g) * (1.0 + mu * mu)) / ((2.0 + g * g) * pow(1.0 + g * g - 2.f * g * mu, 1.5));
   // return vec3(phaseM);
    for (int i = 0; i < numSamples; ++i) {
        vec3 samplePosition = orig + (tCurrent + segmentLength * 0.5) * dir;
        float height = length(samplePosition) - earthRadius;
        if (height < 0) return vec3(0.0, 1.0, 0.0); // Green for invalid height
        // compute optical depth for light
        float hr = exp(-height / Hr) * segmentLength;
        float hm = exp(-height / Hm) * segmentLength;
        opticalDepthR += hr;
        opticalDepthM += hm;
        // light optical depth
        float t0Light, t1Light;
        if (!raySphereIntersect(samplePosition, sunAngle, atmosphereRadius, t0Light, t1Light)) continue;
        float segmentLengthLight = t1Light / float(numSamplesLight), tCurrentLight = 0;
        float opticalDepthLightR = 0, opticalDepthLightM = 0;
        int j;
        for (j = 0; j < numSamplesLight;  j++) {
            
            vec3 samplePositionLight = samplePosition + (tCurrentLight + segmentLengthLight * 0.5) * sunAngle;
            float heightLight = length(samplePositionLight) - earthRadius;
            if (heightLight < 0) break;
            opticalDepthLightR += exp(-heightLight / Hr) * segmentLengthLight;
            opticalDepthLightM += exp(-heightLight / Hm) * segmentLengthLight;
            tCurrentLight += segmentLengthLight;
            
        }
      
        if (j == numSamplesLight) {
             
            vec3 tau = betaR * (opticalDepthR + opticalDepthLightR) + betaM * 1.1 * (opticalDepthM + opticalDepthLightM);
            vec3 attenuation = vec3(exp(-tau.x), exp(-tau.y), exp(-tau.z));
            sumR += attenuation * hr;
            sumM += attenuation * hm;
           
        }
        tCurrent += segmentLength;
    }

    // [comment]
    // We use a magic number here for the intensity of the sun (20). We will make it more
    // scientific in a future revision of this lesson/code
    // [/comment]
    //return vec3(betaM);
    return (sumR * betaR * phaseR + sumM * betaM * phaseM) * 20.0;
    // sumR = value
    // betaR = 0
    // phaseR = value
    // sumM = 0
    // betaM = 0
    // phaseM = value
}


float kInfinity = 1e20;
void main() {

    
	// Fisheye perspective
     ivec2 pixelCoords = ivec2(gl_GlobalInvocationID.xy);
    ivec2 imageSize = imageSize(imgOutput);
    vec2 uv = (vec2(pixelCoords) / vec2(imageSize)) * 2.0 - 1.0;
    float z2 = uv.x * uv.x + uv.y * uv.y;
    vec3 color = vec3(0.0);

    if (z2 <= 1.0) {
        float phi = atan(uv.y, uv.x);
        float theta = acos(1.0 - z2);
        vec3 dir = vec3(sin(theta) * cos(phi), cos(theta), sin(theta) * sin(phi));
        color = computeIncidentLight(vec3(0.0, earthRadius + 1.0, 0.0), dir, 0.0, kInfinity);
        color = pow(color * 0.38317, vec3(1.0 / 2.2)); // Apply tone mapping
       
    }
    
      imageStore(imgOutput, pixelCoords, vec4(color, 1.0));
  
    /*
    // Standard perspective test
    ivec2 pixelCoords = ivec2(gl_GlobalInvocationID.xy);
    ivec2 imageSize = imageSize(imgOutput);
    if (pixelCoords.x >= imageSize.x || pixelCoords.y >= imageSize.y) return;

    float aspectRatio = float(imageSize.x) / float(imageSize.y);
    float fov = 65.0;
    float angle = tan(radians(fov) * 0.5);
    uint numPixelSamples = 4;
    vec3 orig = vec3(0, earthRadius + 1000, 30000);

    vec3 color = vec3(0, 0, 0);
    for (uint m = 0; m < numPixelSamples; ++m) {
        for (uint n = 0; n < numPixelSamples; ++n) {
            float rayx = (2 * (pixelCoords.x + (m + fract(sin(dot(pixelCoords.xy, vec2(12.9898, 78.233))) * 43758.5453)) / numPixelSamples) / float(imageSize.x) - 1) * aspectRatio * angle;
            float rayy = (1 - 2 * (pixelCoords.y + (n + fract(sin(dot(pixelCoords.xy, vec2(12.9898, 78.233))) * 43758.5453)) / numPixelSamples) / float(imageSize.y)) * angle;
            vec3 dir = normalize(vec3(rayx, rayy, -1));
            float t0, t1, tMax = kInfinity;
            if (raySphereIntersect(orig, dir, earthRadius, t0, t1) && t1 > 0)
                tMax = max(0.0, t0);
            color =+ computeIncidentLight(orig, dir, 0, tMax);
        }
    }
    color = color / float(numPixelSamples * numPixelSamples);
    color = pow(color * 0.38317, vec3(1.0 / 2.2));
    imageStore(imgOutput, pixelCoords, vec4(color, 1.0));
    */
}

