#type vertex

#version 450 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec2 a_TexCoord;
layout(location = 2) in vec3 a_Normal;

uniform mat4 u_ModelMatrix;

layout(std140, binding = 2) uniform Shadow
{
	mat4 u_ShadowMatrix;
};


void main()
{
	gl_Position = u_ShadowMatrix * u_ModelMatrix * vec4(a_Position, 1.0);
}

#type fragment

#version 450 core

void main()
{
}