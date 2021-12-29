#type vertex
#version 450 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec2 a_TexCoord;
layout(location = 2) in vec3 a_Normal;

layout(std140, binding = 0) uniform Camera
{
	vec4 CameraPosition;
	mat4 ViewProjectionMatrix;
	mat4 ModelMatrix;
	mat4 ProjectionMatrix;
	mat4 ViewMatrix;
	mat4 NormalMatrix;
};

layout(std140, binding = 2) uniform Shadow
{
	mat4 ShadowMatrix;
};

out vec3 v_WorldPosition;
out vec3 v_Normal;
out vec4 v_LightSpacePosition;
out vec2 v_TexCoord;
flat out vec3 v_CameraPosition;

void main()
{
	v_CameraPosition = CameraPosition.xyz;
	v_TexCoord = a_TexCoord;
	v_WorldPosition = (ModelMatrix * vec4(a_Position, 1.0)).xyz;
	v_LightSpacePosition = ShadowMatrix * vec4(v_WorldPosition, 1.0);
	v_Normal = vec3(NormalMatrix * vec4(a_Normal, 0.0));

	gl_Position = ViewProjectionMatrix * vec4(v_WorldPosition, 1.0);
}



#type fragment
#version 450 core

const float PI = 3.141592;
const float Epsilon = 0.00001;
const vec3 FresnelDialectric = vec3(0.04);


layout(location = 0) out vec4 o_Color;

uniform float u_Roughness;
uniform float u_Metallic;
uniform float u_AO;
uniform vec4 u_AlbedoColor;

uniform sampler2D sampler_ShadowMap;
uniform sampler2D sampler_DiffuseTexture;

layout(std140, binding = 1) uniform Light
{
	vec4 LightColor;
	vec3 LightPosition;
	float LightIntensity;
};

in vec2 v_TexCoord;
in vec3 v_WorldPosition;
in vec3 v_Normal;
in vec4 v_LightSpacePosition;
flat in vec3 v_CameraPosition;

vec3 FresnelSchlick(vec3 F0, float cosTheta, float roughness)
{
	return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(1.0 - cosTheta, 5.0);
}

float NDFGGX(float cosLh, float roughness)
{
	float alpha = roughness * roughness;
	float alphaSquared = alpha * alpha;

	float denominator = (cosLh * cosLh) * (alphaSquared - 1.0) + 1.0;
	return alphaSquared / (PI * denominator * denominator);
}

float GASchlickGGX(float cosLi, float NdotV, float roughness)
{
	float r = roughness + 1.0;
	float k = (r * r) / 8.0;

	float ggx1 = cosLi / (cosLi * (1.0 - k) + k);
	float ggx2 = NdotV / (NdotV * (1.0 - k) + k);

	return ggx1 * ggx2;
}

void main()
{
	vec3 N = normalize(v_Normal);
	vec3 V = normalize(v_CameraPosition - v_WorldPosition);
	vec3 Li = normalize(LightPosition - v_WorldPosition);
	vec3 Lh = normalize(Li + V);

	vec3 F0 = mix(FresnelDialectric, u_AlbedoColor.rgb, u_Metallic);

	// Reflectance Equation
	vec3 radiance = vec3(0.0);
	vec3 Lradiance = LightColor.rgb * LightIntensity;

	float NdotV = dot(N, V);
	float cosLi = max(0.0, dot(N, Li));
	float cosLh = max(0.0, dot(N, Lh));

	vec3  F = FresnelSchlick(F0, max(0.0, dot(Lh, V)), u_Roughness);
	float D = NDFGGX(cosLh, u_Roughness);
	float G = GASchlickGGX(cosLi, NdotV, u_Roughness);

	vec3 kD = (1.0 - F) * (1.0 - u_Metallic);
	vec3 diffuseBRDF = kD * u_AlbedoColor.rgb;

	// Cook-Torrance
	vec3 specularBRDF = (F * D * G) / max(Epsilon, 4.0 * cosLi * NdotV);
	specularBRDF = clamp(specularBRDF, vec3(0.0f), vec3(10.0f));

	radiance = (diffuseBRDF + specularBRDF) * Lradiance * cosLi;


	o_Color = vec4(radiance, 1.0);
}