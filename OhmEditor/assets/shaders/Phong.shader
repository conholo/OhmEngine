#type vertex

#version 450 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec2 a_TexCoord;
layout(location = 2) in vec3 a_Normal;

layout(std140, binding = 0) uniform Camera
{
	mat4 u_ViewProjectionMatrix;
	mat4 u_ModelMatrix;
	mat4 u_ProjectionMatrix;
	mat4 u_ViewMatrix;
	mat4 u_NormalMatrix;
};

out vec3 v_ViewPosition;
out vec3 v_Normal;

void main()
{
	vec4 viewPosition = u_ViewMatrix * u_ModelMatrix * vec4(a_Position, 1.0);
	v_ViewPosition = viewPosition.xyz;

	v_Normal = vec3(u_NormalMatrix * vec4(a_Normal, 0.0));

	gl_Position = u_ViewProjectionMatrix * u_ModelMatrix * vec4(a_Position, 1.0);
}


#type fragment

#version 450 core

layout(location = 0) out vec4 o_Color;

uniform vec4 u_DiffuseColor = vec4(1.0);
uniform vec4 u_AmbientColor = vec4(0.2, 0.6, 0.3, 1.0);

uniform float u_SpecularStrength = 0.8;
uniform float u_AmbientStrength = 0.6;
uniform float u_DiffuseStrength = 0.3;
uniform float u_SpecularPower = 32.0;

layout(std140, binding = 1) uniform Light
{
	vec4 u_LightColor;
	vec3 u_ViewSpaceLightPosition;
	float u_LightIntensity;
};

in vec3 v_ViewPosition;
in vec3 v_Normal;

void main()
{
	vec3 normal = normalize(v_Normal);
	vec3 lightDirection = normalize(u_ViewSpaceLightPosition - v_ViewPosition);

	// Ambient
	vec4 ambient = u_LightIntensity * mix(u_LightColor, u_AmbientColor, u_AmbientStrength);

	// Diffuse
	float NdotL = max(dot(normal, lightDirection), 0);
	vec4 diffuse = u_LightIntensity * NdotL * u_DiffuseStrength * u_LightColor;

	// Specular
	vec3 inverseView = normalize(-v_ViewPosition);
	vec3 reflectDirection = reflect(-lightDirection, normal);

	float specular = pow(max(dot(inverseView, reflectDirection), 0.0), u_SpecularPower);

	vec4 spec = u_LightIntensity * specular * u_LightColor * u_SpecularStrength;

	vec4 result = (diffuse + spec + ambient) * u_DiffuseColor;

	o_Color = vec4(result.x, result.y, result.z, 1.0);
}
