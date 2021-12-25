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

layout(location = 0) out vec4 o_Color;

uniform vec4 u_DiffuseColor = vec4(1.0);
uniform vec4 u_AmbientColor = vec4(1.0);

uniform float u_SpecularStrength = 0.8;
uniform float u_AmbientStrength = 0.6;
uniform float u_DiffuseStrength = 0.3;
uniform float u_SpecularPower = 32.0;

uniform sampler2D sampler_ShadowMap;
uniform sampler2D sampler_Texture;

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

float ShadowCalculation(vec4 fragPositionLightSpace, vec3 lightDirection, vec3 normal)
{
	// Light-space position in the range of [-1, 1]
	vec3 projectionCoords = fragPositionLightSpace.xyz / fragPositionLightSpace.w;

	// Transform NDC to [0, 1], the depth map is in the range [0, 1]
	projectionCoords = projectionCoords * 0.5 + 0.5;

	// The closest depth from the light's perspective [0, 1]
	float closestDepth = texture(sampler_ShadowMap, projectionCoords.xy).r;
	// The actual depth of the fragment from the light's perspective
	float currentDepth = projectionCoords.z;

	float bias = max(0.05 * (1.0 - dot(normal, lightDirection)), 0.005);

	// Simple shadows
	// If the current depth is greater than the closest, the fragment is in shadow.
	//float shadow = currentDepth - bias > closestDepth ? 1.0 : 0.0;

	// PCF (percentage-closer filtering)
	float shadow = 0.0;
	vec2 texelSize = 1.0 / textureSize(sampler_ShadowMap, 0);
	for (int x = -1; x <= 1; x++)
	{
		for (int y = -1; y <= 1; y++)
		{
			float pcfDepth = texture(sampler_ShadowMap, projectionCoords.xy + vec2(x, y) * texelSize).r;
			shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;
		}
	}

	shadow /= 9.0;

	if (projectionCoords.z > 1.0)
		shadow = 0.0;

	return shadow;
}


void main()
{
	vec3 normal = normalize(v_Normal);
	vec3 lightDirection = normalize(LightPosition - v_WorldPosition);

	// Ambient
	vec4 ambient = LightIntensity * mix(LightColor, u_AmbientColor, u_AmbientStrength) * u_AmbientStrength;

	// Diffuse
	float NdotL = max(dot(normal, lightDirection), 0);
	vec4 diffuse = LightIntensity * NdotL * u_DiffuseStrength * LightColor;

	// Specular
	vec3 viewDirection = normalize(v_CameraPosition - v_WorldPosition);
	vec3 halfway = normalize(lightDirection + viewDirection);

	float specular = pow(max(dot(normal, halfway), 0.0), u_SpecularPower);
	vec4 spec = LightIntensity * specular * LightColor * u_SpecularStrength;

	float shadow = ShadowCalculation(v_LightSpacePosition, lightDirection, normal);
	vec4 result = (ambient + (1.0 - shadow) * (diffuse + spec)) * u_DiffuseColor;

	vec3 texColor = texture(sampler_Texture, v_TexCoord).rgb;
	result.rgb *= texColor;

	o_Color = vec4(result.x, result.y, result.z, 1.0);
}
