#version 420

// required by GLSL spec Sect 4.5.3 (though nvidia does not, amd does)
precision highp float;

 in vec3 pos;
 in vec2 texCoord;
  uniform vec3 sunAngle;
  out vec4 fragColor;

uniform vec3 cameraPosition;
uniform vec3 cameraDirection;
uniform mat4 invViewProjection;




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
  void main()
  {
    /*
    if (pos.y < 0)
      discard;
    
    // Atmosphere Scattering
    float mu = dot(normalize(pos), normalize(sunAngle));
    float rayleigh = 3.0 / (8.0 * 3.14) * (1.0 + mu * mu);
    vec3 mie = (Kr + Km * (1.0 - g * g) / (2.0 + g * g) / pow(1.0 + g * g - 2.0 * g * mu, 1.5)) / (Br + Bm);

    vec3 day_extinction = exp(-exp(-((pos.y + sunAngle.y * 4.0) * (exp(-pos.y * 16.0) + 0.1) / 80.0) / Br) * (exp(-pos.y * 16.0) + 0.1) * Kr / Br) * exp(-pos.y * exp(-pos.y * 8.0 ) * 4.0) * exp(-pos.y * 2.0) * 4.0;
    vec3 night_extinction = vec3(1.0 - exp(sunAngle.y));
    vec3 extinction = mix(day_extinction, night_extinction, -sunAngle.y * 0.2 + 0.5);
    color.rgb = rayleigh * mie * extinction;

    float time = 0.0;
    // Cirrus Clouds
    float density = smoothstep(1.0 - cirrus, 1.0, fbm(pos.xyz / pos.y * 2.0 + time * 0.05)) * 0.3;
    color.rgb = mix(color.rgb, extinction * 4.0, density * max(pos.y, 0.0));

    // Cumulus Clouds
    for (int i = 0; i < 3; i++)
    {
      float density = smoothstep(1.0 - cumulus, 1.0, fbm((0.7 + float(i) * 0.01) * pos.xyz / pos.y + time * 0.3));
      color.rgb = mix(color.rgb, extinction * density * 5.0, min(density, 1.0) * max(pos.y, 0.0));
    }
    
    // Dithering Noise
    color.rgb += noise(pos * 1000) * 0.01;
    */

    	// Fisheye perspective
    //ivec2 pixelCoords = ivec2(gl_GlobalInvocationID.xy);
    //ivec2 imageSize = imageSize(imgOutput);
    //vec2 texCoord = (vec2(texCoord) / vec2(1920, 1080)) * 2.0 - 1.0;

    // Fisheye
    /*
    vec2 uv = (texCoord - 0.5) * 2.0;
    float z2 = uv.x * uv.x + uv.y * uv.y;
    vec3 color = vec3(0.0);

    if (z2 <= 1.0) {
        float phi = atan(uv.y, uv.x);
        float theta = acos(1.0 - z2);
        vec3 dir = vec3(sin(theta) * cos(phi), cos(theta), sin(theta) * sin(phi));
        color = computeIncidentLight(vec3(0.0, earthRadius + 1.0, 0.0), dir, 0.0, atmosphereRadius);
        color = pow(color * 0.38317, vec3(1.0 / 2.2)); // Apply tone mapping
    }
    
    fragColor = vec4(color, 1.0);
    */

    vec2 uv = (texCoord - 0.5) * 2.0;
    vec4 clipSpace = vec4(uv, 1.0, 1.0);
    vec4 viewSpace = invViewProjection * clipSpace;
    vec3 worldDir = normalize(viewSpace.xyz / viewSpace.w);

    float tiltAmount = 0.2; // Adjust this value to control the amount the horizon is lowered
    worldDir.y += tiltAmount;
    worldDir = normalize(worldDir);

    // Adjust camera position to be above the Earth's surface
    vec3 cameraPosition = vec3(0.0, earthRadius + 1.0, 0.0);

    vec3 color = computeIncidentLight(cameraPosition, worldDir, 0.0, atmosphereRadius);
   

     if (worldDir.y < 0.00000001) {


        // Sample color just above the horizon
        vec3 horizonDir = normalize(vec3(worldDir.x, 0.01, worldDir.z));
        color = computeIncidentLight(cameraPosition, horizonDir, 0.0, atmosphereRadius);
        //color = vec3(0.0);
    }
    color = pow(color * 0.38317, vec3(1.0 / 2.2)); // Apply tone mapping
    fragColor = vec4(color, 1.0);
    
    fragColor = vec4(color, 1.0);
   // color = vec4(texCoord, 0.0,1.0);
    
     
  }