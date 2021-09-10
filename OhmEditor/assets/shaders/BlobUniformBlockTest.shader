#type vertex

#version 450 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec2 a_TexCoord;
layout(location = 2) in vec3 a_Normal;

out vec2 v_TexCoord;


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
	v_TexCoord = a_TexCoord;
	gl_Position = u_ProjectionMatrix * u_ModelView * vec4(a_Position, 1.0);
}


#type frag

#version 450 core

layout(location = 0) out vec4 o_Color;

in vec3 v_TexCoord;

layout(binding = 2) uniform BlobSettings
{
	vec4 InnerColor;
	vec4 OuterColor;
	float RadiusInner;
	float RadiusOuter;
}

void main()
{
	float dx = v_TexCoord.x - 0.5;
	float dy = v_TexCoord.y - 0.5;

	float dist = sqrt(dx * dx + dy * dy);

	o_Color = mix(InnerColor, OuterColor, smoothstep(RadiusInner, RadiusOuter, dist));
}
