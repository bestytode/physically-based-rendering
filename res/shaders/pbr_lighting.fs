#version 330 core
out vec4 FragColor;

in vec3 WorldPos;
in vec2 TexCoords;
in vec3 Normal;

uniform vec3 viewPos;
uniform vec3 albedo;
uniform float metallic;
uniform float roughness;
uniform float ao;

uniform vec3 lightPositions[4];
uniform vec3 lightColors[4];

float DistributionGGX(vec3 N, vec3 H, float roughness)
{
	float 
}

// cosTheta: cosine between H and V
// F0: base reflectance when angle degree is 0, non-metal is usually vec3(0.04f), metal F0 represents its base color.
//
// Notice: we use vec3 as return type because material itself reflects different kinds of light color differently.
vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

void main()
{
	// Both N & V is in world space
	vec3 N = normalize(Normal); // Normal
	vec3 V = normalize(viewPos - WorldPos); // View direction

	vec3 F0 = vec3(0.04f); // For non-metal material, we simply use vec3(0.04f)
	F0 = mix(F0, albedo, metallic); // For metal material, we interpolate F0 to albedo based on the metallic coefficient

	vec3 Lo = vec3(0.0f);
	for(int i = 0; i < 4; i++) {
		// Calculate per-light radiance
		vec3 L = normalize(lightPositions[i] - WorldPos); // Light direction
		vec3 H = normalize(V + L); // HalfwayVector
		float distance = length(lightPositions[i] - WorldPos);
		float attenuation = 1.0f / (distance * distance); // Simple attenuation, may use linear and quadratic coefficient later
		vec3 radiance = lightColors[i] * attenuation; // Yet we calculated the incoming light of a specific direction

		// Cook-Torrance specular BRDF calculation
		float NDF = 
		float G =
		float F = fresnelSchlick(max(dot(H, V), 0.0f), F0);
	}
}