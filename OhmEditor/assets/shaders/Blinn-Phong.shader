#type vertex

#version 450 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec2 a_TexCoord;
layout(location = 2) in vec3 a_Normal;


layout(std140, binding = 0) uniform Camera
{
	mat4 u_ModelView;
	mat4 u_ProjectionMatrix;
	mat4 u_NormalMatrix;
};

out vec3 v_Normal;
out vec3 v_ViewPosition;
out vec2 v_TexCoord;

void main()
{
	v_TexCoord = a_TexCoord;
	v_Normal = vec3(u_NormalMatrix * vec4(a_Normal, 0.0));

	vec4 viewSpacePosition = u_ModelView * vec4(a_Position, 1.0);
	v_ViewPosition = viewSpacePosition.xyz;
	gl_Position = u_ProjectionMatrix * viewSpacePosition;
}

#type fragment

#version 450 core

layout(location = 0) out vec4 o_Color;

uniform vec4 u_Color;
uniform float u_SpecularStrength;
uniform float u_AmbientStrength;

struct LightingData
{
	vec4 LightColor;
	vec3 LightPosition;
};


layout(std140, binding = 1) uniform Light
{
	LightingData u_LightingData;
};


uniform sampler2D u_Texture;

in vec2 v_TexCoord;
in vec3 v_Normal;
in vec3 v_ViewPosition;

void main()
{
	vec4 lightColor = u_LightingData.LightColor;
	vec3 lightPosition = u_LightingData.LightPosition;

	vec3 normal = normalize(v_Normal);
	vec3 lightDirection = normalize(lightPosition - v_ViewPosition);

	// Ambient
	vec4 ambient = u_AmbientStrength * lightColor;

	// Diffuse
	float NdotL = max(dot(normal, lightDirection), 0.0);

	vec4 diffuse = NdotL * lightColor;

	// Specular
	vec3 viewDirection = normalize(-v_ViewPosition);
	vec3 reflectDirection = reflect(-lightDirection, normal);

	float specular = pow(max(dot(viewDirection, reflectDirection), 0.0), 256);
	vec4 spec = u_SpecularStrength * specular * lightColor;

	vec4 texColor = texture(u_Texture, v_TexCoord);

	vec4 result = (ambient + diffuse + spec) * u_Color * texColor;

	o_Color = vec4(result.r, result.g, result.b, 1.0f);
}

