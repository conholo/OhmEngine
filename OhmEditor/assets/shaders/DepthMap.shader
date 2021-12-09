#type vertex

#version 450 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec2 a_TexCoord;
layout(location = 2) in vec3 a_Normal;

out vec2 v_TexCoord;

void main()
{
	v_TexCoord = a_TexCoord;
	gl_Position = vec4(a_Position, 1.0);
}

#type fragment

#version 450 core

layout(location = 0) out vec4 o_Color;

in vec2 v_TexCoord;

uniform sampler2D sampler_DepthMap;
uniform float u_NearPlane;
uniform float u_FarPlane;

// required when using a perspective projection matrix
//https://learnopengl.com/Advanced-Lighting/Shadows/Shadow-Mapping
float LinearizeDepth(float depth)
{
	float z = depth * 2.0 - 1.0; // Back to NDC 
	return (2.0 * u_NearPlane * u_FarPlane) / (u_FarPlane + u_NearPlane - z * (u_FarPlane - u_NearPlane));
}

void main()
{
	//vec4 depthValue = vec4(vec3(LinearizeDepth(depthValue) / u_FarPlane), 1.0); // perspective
	float depthValue = texture(sampler_DepthMap, v_TexCoord).x;
	o_Color = vec4(vec3(depthValue), 1.0);
}