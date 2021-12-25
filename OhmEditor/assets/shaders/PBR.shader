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

#define PI 3.14159265359

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

float DistributionGGX(vec3 N, vec3 halfway, float roughness)
{
	float a = roughness * roughness;
	float a2 = a * a;
	float NdotH = max(dot(N, halfway), 0.0);
	float NdotH2 = NdotH * NdotH;

	float numerator = a2;
	float denominator = NdotH2 * (a2 - 1.0) + 1.0;
	denominator = PI * denominator * denominator;

	return numerator / denominator;
}

float GeometrySchlickGGX(float cosTheta, float roughness)
{
	float r = (roughness + 1.0);
	float k = (r * r) / 8.0;

	float numerator = cosTheta;
	float denominator = cosTheta * (1.0 - k) + k;

	return numerator / denominator;
}

float GeometrySmith(vec3 N, vec3 view, vec3 lightDir, float roughness)
{
	float NdotV = max(dot(N, view), 0.0);
	float NdotL = max(dot(N, lightDir), 0.0);
	float ggx2 = GeometrySchlickGGX(NdotV, roughness);
	float ggx1 = GeometrySchlickGGX(NdotL, roughness);

	return ggx1 * ggx2;
}

vec3 FresnelSchlick(float cosTheta, vec3 F0)
{
	return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

void main()
{
	float roughness = max(u_Roughness, 0.01);
	vec3 N = normalize(v_Normal);
	vec3 LD = normalize(LightPosition - v_WorldPosition);
	vec3 V = normalize(v_CameraPosition - v_WorldPosition);
	vec3 halfway = normalize(V + LD);

	vec3 F0 = vec3(0.04);
	F0 = mix(F0, u_AlbedoColor.rgb, u_Metallic);

	// Reflectance Equation
	vec3 L0 = vec3(0.0);
	vec3 radiance = LightColor.rgb;

	// Cook-Torrance BRDF
	float NDF = DistributionGGX(N, halfway, roughness);
	float G = GeometrySmith(N, V, LD, roughness);
	vec3 F = FresnelSchlick(max(dot(halfway, V), 0.0), F0);

	vec3 kS = F;
	vec3 kD = vec3(1.0) - kS;
	kD *= 1.0 - u_Metallic;

	vec3 numerator = NDF * G * F;
	float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, LD), 0.0) + 0.0001;
	vec3 specular = numerator / denominator;

	// Outgoing radiance L0
	float NdotL = max(dot(N, LD), 0.0);
	L0 += (kD * u_AlbedoColor.rgb / PI + specular) * radiance * NdotL;

	vec3 ambient = vec3(0.03) * u_AlbedoColor.rgb * u_AO;
	vec3 color = ambient + L0;

	color = color / (color + vec3(1.0));
	color = pow(color, vec3(1.0 / 2.2));

	o_Color = vec4(color, 1.0);
}