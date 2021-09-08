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

void main()
{
	gl_Position = u_ProjectionMatrix * u_ModelView * vec4(a_Position, 1.0);
}

#type fragment

#version 450 core

layout(location = 0) out vec4 o_Color;

uniform vec4 u_Color;

void main()
{
	o_Color = u_Color;
}

