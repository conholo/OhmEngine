#type vertex

#version 330 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec2 a_TexCoord;
layout(location = 2) in vec3 a_Normal;

uniform mat4 u_PVM;
uniform mat4 u_NormalMatrix;

out vec3 v_Normal;
out vec3 v_ViewPosition;

void main()
{
	v_Normal = vec3(u_NormalMatrix * vec4(a_Normal, 0.0));

	vec4 clipSpacePosition = u_PVM * vec4(a_Position, 1.0);
	v_ViewPosition = clipSpacePosition.xyz;
	gl_Position = clipSpacePosition;
}

#type fragment

#version 330 core

layout(location = 0) out vec4 o_Color;

uniform vec4 u_Color;
uniform vec3 u_CameraPosition;
uniform vec3 u_LightPosition;

in vec3 v_Normal;
in vec3 v_ViewPosition;

void main()
{
	vec3 lightDirection = normalize(u_LightPosition - v_ViewPosition);
	float NdotL = clamp(dot(v_Normal, lightDirection), 0, 1);

	o_Color = vec4(NdotL, NdotL, NdotL, 1.0);
}

