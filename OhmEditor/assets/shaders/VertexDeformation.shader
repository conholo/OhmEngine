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

layout(std140, binding = 3) uniform Global
{
	float Elapsed;
	float DeltaTime;
};

out vec3 v_Position;

uniform float u_Speed = 1.0;

float Random(vec2 st)
{
	return fract(sin(dot(st.xy,
		vec2(12.9898, 78.233))) *
		43758.5453123);
}


void main()
{
	v_Position = a_Position;

	v_Position.x += sin(Elapsed) * u_Speed * Random(a_Position.xy);
	v_Position.y += cos(Elapsed) * u_Speed * Random(a_Position.xy);
	v_Position.z += sin(Elapsed) * u_Speed * Random(a_TexCoord);

	gl_Position = u_ViewProjectionMatrix * u_ModelMatrix * vec4(v_Position, 1.0);
}

#type fragment

#version 450 core

layout(location = 0) out vec4 o_Color;

uniform vec4 u_Color = vec4(1.0);

void main()
{
	o_Color = u_Color;
}