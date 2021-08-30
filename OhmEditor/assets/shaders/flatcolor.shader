#type vertex

#version 330 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec2 a_TexCoord;
layout(location = 2) in vec3 a_Normal;

uniform mat4 u_PVM;

void main()
{
	gl_Position = u_PVM * vec4(a_Position, 1.0);
}

#type fragment

#version 330 core

layout(location = 0) out vec4 o_Color;

uniform vec4 u_Color;

void main()
{
	o_Color = u_Color;
}

