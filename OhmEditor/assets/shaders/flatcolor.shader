#type vertex

#version 330 core

layout(location = 0) in vec3 a_Position;

void main()
{
	gl_Position = vec4(a_Position, 1.0);
}

#type fragment

#version 330 core

layout(location = 0) out vec4 o_Color;

void main()
{
	o_Color = vec4(1.0, 0.0, 0.0, 1.0);
}

