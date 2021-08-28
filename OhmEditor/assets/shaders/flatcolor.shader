#type vertex

#version 330 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec2 a_TexCoord;

uniform mat4 u_ProjectionView;
uniform mat4 u_Model;

void main()
{
	gl_Position = u_ProjectionView * u_Model * vec4(a_Position, 1.0);
}

#type fragment

#version 330 core

layout(location = 0) out vec4 o_Color;

void main()
{
	o_Color = vec4(0.0, 0.0, 1.0, 1.0);
}

