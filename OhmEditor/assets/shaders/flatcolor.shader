#type vertex

#version 450 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normal;
layout(location = 2) in vec3 a_Tangent;
layout(location = 3) in vec3 a_Binormal;
layout(location = 4) in vec2 a_TexCoord;

layout(std140, binding = 0) uniform Camera
{
	mat4 u_ViewProjectionMatrix;
	mat4 u_ModelMatrix;
	mat4 u_ProjectionMatrix;
	mat4 u_ViewMatrix;
	mat4 u_NormalMatrix;
};

void main()
{
	gl_Position = u_ViewProjectionMatrix * u_ModelMatrix * vec4(a_Position, 1.0);
}

#type fragment

#version 450 core

layout(location = 0) out vec4 o_Color;

uniform vec4 u_Color = vec4(1.0);

void main()
{
	o_Color = u_Color;
}

