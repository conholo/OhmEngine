#type vertex

#version 450 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec2 a_TexCoord;
layout(location = 2) in vec3 a_Normal;


layout(std140, binding = 0) uniform Camera
{
	mat4 u_ModelView;
	mat4 u_ModelMatrix;
	mat4 u_ViewMatrix;
	mat4 u_ProjectionMatrix;
	mat4 u_NormalMatrix;
	vec3 u_CameraPosition;
};

out vec3 v_HitPosition;
out vec3 v_RayOrigin;
out vec2 v_TexCoord;


void main()
{
	v_TexCoord = a_TexCoord;

	vec4 viewSpacePosition = u_ModelView * vec4(a_Position, 1.0);
	vec3 cameraView = (inverse(u_ModelMatrix) * vec4(u_CameraPosition, 1.0)).xyz;
	v_RayOrigin = cameraView;
	v_HitPosition = a_Position;

	gl_Position = u_ProjectionMatrix * viewSpacePosition;
}


#type fragment

#version 450 core

layout(location = 0) out vec4 o_Color;

uniform vec4 u_Color;
uniform sampler2D u_Texture;

struct LightingData
{
	vec4 LightColor;
	vec3 ViewSpaceLightPosition;
	float _pad;
	vec3 WorldSpaceLightPosition;
	float LightIntensity;
};


layout(std140, binding = 1) uniform Light
{
	LightingData u_LightingData;
};


in vec2 v_TexCoord;
in vec3 v_HitPosition;
in vec3 v_RayOrigin;

float DistanceToScene(vec3 position)
{
	return length(vec2(length(position.xz) - 0.4, position.y)) - 0.1;
}


float RayMarch(vec3 rayOrigin, vec3 rayDirection)
{
	float distanceTraveled = 0.0;

	for (int i = 0; i < 100; i++)
	{
		vec3 rayPosition = rayOrigin + rayDirection * distanceTraveled;

		float distanceToSurface = DistanceToScene(rayPosition);

		distanceTraveled += distanceToSurface;

		if (distanceTraveled > 100.0f || distanceToSurface < 0.001)
			break;
	}

	return distanceTraveled;
}

vec3 GetSceneNormals(vec3 position)
{
	vec2 epsilon = vec2(0.001, 0.0);

	vec3 normal = DistanceToScene(position) - vec3(
		DistanceToScene(position - epsilon.xyy),
		DistanceToScene(position - epsilon.yxy),
		DistanceToScene(position - epsilon.yyx)
	);

	return normalize(normal);
}


void main()
{
	vec3 rayOrigin = v_RayOrigin;
	vec3 rayDirection = normalize(v_HitPosition - v_RayOrigin);

	float rayMarch = RayMarch(rayOrigin, rayDirection);

	vec4 result = vec4(0.0);

	if (rayMarch >= 100.0f)
		discard;
	else
	{
		vec3 rayPosition = rayOrigin + rayDirection * rayMarch;
		vec3 normal = GetSceneNormals(rayPosition);
		vec3 lightDirection = normalize(u_LightingData.ViewSpaceLightPosition - v_HitPosition);

		float NdotL = max(dot(normal, lightDirection), 0.0);

		float diffuseIntensity = NdotL * u_LightingData.LightIntensity;

		result = vec4(diffuseIntensity, diffuseIntensity, diffuseIntensity, 1.0);
	}

	o_Color = result * u_Color;
}

