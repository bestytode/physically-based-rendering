#version 330 core
out vec4 FragColor;
in vec3 WorldPos;

uniform samplerCube environmentMap;

const float PI = 3.14159265359;

void main()
{		
	// The world vector is a surface normal from the origin to WorldPos.
    // Using this, compute incoming radiance from the environment.
    // This radiance comes from the -Normal direction, used in the PBR shader for irradiance sampling.
    vec3 N = normalize(WorldPos);

    vec3 irradiance = vec3(0.0);   
    
    // tangent space calculation from origin point
    vec3 up    = vec3(0.0, 1.0, 0.0);
    vec3 right = normalize(cross(up, N));
    up         = normalize(cross(N, right));
       
    float sampleDelta = 0.025; 
    float nrSamples = 0.0;

    // Iterate over hemisphere oriented around normal N.
    for(float phi = 0.0; phi < 2.0 * PI; phi += sampleDelta) {
        for(float theta = 0.0; theta < 0.5 * PI; theta += sampleDelta) {
            // Convert spherical coordinates to tangent space.
            vec3 tangentSample = vec3(sin(theta) * cos(phi),  sin(theta) * sin(phi), cos(theta));
            
            // Transform sample from tangent space to world space.
            vec3 sampleVec = tangentSample.x * right + tangentSample.y * up + tangentSample.z * N; 

            // Accumulate irradiance: sample environment map and weight by angle.
            // Angle weighting ensures that the sampling correctly captures the varying influence of light from different directions.
            irradiance += texture(environmentMap, sampleVec).rgb * cos(theta) * sin(theta);
            nrSamples++;
        }
    }

    // Average accumulated irradiance and scale by PI. (this Pi here is a normalization step)
    irradiance = PI * irradiance * (1.0 / float(nrSamples));
    
    FragColor = vec4(irradiance, 1.0);
}