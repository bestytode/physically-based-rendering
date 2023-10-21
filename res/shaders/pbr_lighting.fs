#version 330 core
out vec4 FragColor;

in vec3 WorldPos;
in vec2 TexCoords;
in vec3 Normal;

uniform vec3 viewPos;

// material parameters
uniform sampler2D albedoMap;
uniform sampler2D normalMap;
uniform sampler2D metallicMap;
uniform sampler2D roughnessMap;
uniform sampler2D aoMap;

// lighting infos
uniform vec3 lightPosition;
uniform vec3 lightColor;

// Scaling factors
uniform float roughnessScale;
uniform float metallicScale;
uniform vec3 albedoScale;

const float PI = 3.1415926;

vec3 getNormalFromMap()
{
    vec3 tangentNormal = texture(normalMap, TexCoords).xyz * 2.0 - 1.0;

    vec3 Q1  = dFdx(WorldPos);
    vec3 Q2  = dFdy(WorldPos);
    vec2 st1 = dFdx(TexCoords);
    vec2 st2 = dFdy(TexCoords);

    vec3 N   = normalize(Normal);
    vec3 T  = normalize(Q1*st2.t - Q2*st1.t);
    vec3 B  = -normalize(cross(N, T));
    mat3 TBN = mat3(T, B, N);

    return normalize(TBN * tangentNormal);
}

// Calculating how the microfacets are oriented relative to the normal N and H
float DistributionGGX(vec3 N, vec3 H, float roughness)
{
	// looks more natural when squared roughness 
	float a = roughness * roughness;
	a = a * a; //...
	float NdotH = max(dot(N, H), 0.0f);
	float NdotH2 = NdotH * NdotH;

	float nom = a;
	float denom = NdotH2 * (a - 1.0f) + 1.0f;
	denom = PI * denom * denom;

	return nom / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;

    float nom   = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}

// Calculate the possibility that occlussion each other in microfacets
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx1 = GeometrySchlickGGX(NdotV, roughness);
    float ggx2 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
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
	vec3 albedo     = pow(texture(albedoMap, TexCoords).rgb, vec3(2.2)) * albedoScale;
    float metallic  = texture(metallicMap, TexCoords).r * metallicScale;
    float roughness = texture(roughnessMap, TexCoords).r * roughnessScale;
    float ao        = texture(aoMap, TexCoords).r;

	// Both N & V is in world space
	vec3 N = getNormalFromMap(); // Normal
	vec3 V = normalize(viewPos - WorldPos); // View direction

	vec3 F0 = vec3(0.04f); // For non-metal material, we simply use vec3(0.04f)
	F0 = mix(F0, albedo, metallic); // For metal material, we interpolate F0 to albedo based on the metallic coefficient

	vec3 Lo = vec3(0.0f);

	// we only have one light source here
	for(int i = 0; i < 1; i++) {
		// Calculate per-light radiance
		vec3 L = normalize(lightPosition - WorldPos); // Light direction
		vec3 H = normalize(V + L); // HalfwayVector
		float distance = length(lightPosition - WorldPos);
		float attenuation = 1.0f / (distance * distance); // Simple attenuation, may use linear and quadratic coefficient later
		vec3 incomingRadiance = lightColor * attenuation;
		float NdotL = max(dot(N, L), 0.0);
        vec3 scaledIncomingRadiance = incomingRadiance * NdotL; 

		// Cook-Torrance specular BRDF calculation
		float NDF = DistributionGGX(N, H, roughness);
		float G = GeometrySmith(N, V, L, roughness);
		vec3 F = fresnelSchlick(max(dot(H, V), 0.0f), F0);

		vec3 numerator = NDF * G * F;
		float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
		vec3 specular = numerator / denominator;

		vec3 Ks = F; // fresnel function describe the reflect light ratio of total 
		vec3 Kd = vec3(1.0f) - Ks;
		Kd = Kd * (1 - metallic); // reduce the diffuse light of metal material

		vec3 BRDF = Kd * albedo / PI + specular; // already multiplied the BRDF by the Fresnel (kS)
		Lo += BRDF * scaledIncomingRadiance;
	}

	// ambient lighting (note that the next IBL tutorial will replace 
    // this ambient lighting with environment lighting).
    vec3 ambient = vec3(0.03) * albedo * ao;
    vec3 color = ambient + Lo;

	// HDR tonemapping
    color = color / (color + vec3(1.0));
    // gamma correct
    color = pow(color, vec3(1.0/2.2)); 

	FragColor = vec4(color, 1.0f);
}