#type vertex
#version 450 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normal;
layout(location = 2) in vec3 a_Tangent;
layout(location = 3) in vec3 a_Binormal;
layout(location = 4) in vec2 a_TexCoord;

layout(std140, binding = 1) uniform Camera
{
    vec4 CameraPosition;
    mat4 ViewProjectionMatrix;
    mat4 ProjectionMatrix;
    mat4 ViewMatrix;
};

layout(std140, binding = 3) uniform Entity
{
    mat4 ModelMatrix;
};

out Interpolators
{
	vec3 WorldPosition;           
	vec3 Normal;                  
	vec2 TexCoord;                
	mat3 WorldNormals;            
	mat3 WorldTransform;          
	vec3 Binormal;                
	vec3 WorldSpaceViewDirection; 
	vec3 ViewPosition;  
} VertexOutput;

void main()
{
	vec4 worldPosition = ModelMatrix * vec4(a_Position, 1.0);
	VertexOutput.WorldPosition = worldPosition.xyz;
	VertexOutput.Normal = mat3(ModelMatrix) * a_Normal;
	VertexOutput.TexCoord = a_TexCoord;
	VertexOutput.WorldNormals = mat3(ModelMatrix) * mat3(a_Tangent, a_Binormal, a_Normal);
	VertexOutput.WorldTransform = mat3(ModelMatrix);
	VertexOutput.Binormal = a_Binormal;
	VertexOutput.WorldSpaceViewDirection = normalize(VertexOutput.WorldPosition - CameraPosition.xyz);
	VertexOutput.WorldSpaceViewDirection.z *= -1;

	VertexOutput.ViewPosition = vec3(ViewMatrix * vec4(VertexOutput.WorldPosition, 1.0));
	gl_Position = ViewProjectionMatrix * ModelMatrix * vec4(a_Position, 1.0);
}

#type fragment
#version 450 core

const float PI = 3.141592;
const float Epsilon = 0.00001;
const vec3 FresnelDialectric = vec3(0.04);

layout(location = 0) out vec4 o_Color;

uniform vec3 AlbedoColor = vec3(1.0);
uniform float Metalness = 0.0;
uniform float Roughness = 1.0;
uniform float Emission = 0.0;

uniform float TextureTiling = 1.0f;
uniform float EnvironmentIntensity = 1.0;
uniform float EnvMapRotation = 0.0;
uniform int UseNormalMap = 0;

uniform sampler2D sampler_AlbedoTexture;
uniform sampler2D sampler_NormalTexture;
uniform sampler2D sampler_MetalnessTexture;
uniform sampler2D sampler_RoughnessTexture;

uniform samplerCube sampler_RadianceCube;
uniform samplerCube sampler_IrradianceCube;
uniform sampler2D sampler_BRDFLUT;


layout(std140, binding = 0) uniform Global
{
    float ElapsedTime;
    float DeltaTime;
};

layout(std140, binding = 1) uniform Camera
{
    vec4 CameraPosition;
    mat4 ViewProjectionMatrix;
    mat4 ProjectionMatrix;
    mat4 ViewMatrix;
};

layout(std140, binding = 2) uniform Scene
{
    vec3 LightRadiance;
    float LightIntensity;
    vec3 LightDirection;
    float ShadowAmount;
};

in Interpolators
{
	vec3 WorldPosition;
	vec3 Normal;
	vec2 TexCoord;
	mat3 WorldNormals;
	mat3 WorldTransform;
	vec3 Binormal;
	vec3 WorldSpaceViewDirection;
	vec3 ViewPosition;
} VertexInput;

struct PBRParameters
{
	vec3 Albedo;
	float Roughness;
	float Metalness;

	vec3 Normal;
	vec3 View;
	float NdotV;
} PBRParams;

vec3 FresnelSchlick(vec3 F0, float cosTheta, float roughness)
{
	return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

vec3 FresnelSchlickRoughness(vec3 F0, float cosTheta, float roughness)
{
	return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(1.0 - cosTheta, 5.0);
}

// GGX/Towbridge-Reitz normal distribution function.
// Uses Disney's reparametrization of alpha = roughness^2
float NdfGGX(float cosLh, float roughness)
{
	float alpha = roughness * roughness;
	float alphaSquared = alpha * alpha;

	float denominator = (cosLh * cosLh) * (alphaSquared - 1.0) + 1.0;
	return alphaSquared / (PI * denominator * denominator);
}

float GaSchlickG1(float cosTheta, float k)
{
	return cosTheta / (cosTheta * (1.0 - k) + k);
}

// Schlick-GGX approximation of geometric attenuation function using Smith's method.
float GaSchlickGGX(float cosLi, float NdotV, float roughness)
{
	float r = roughness + 1.0;
	float k = (r * r) / 8.0; // Epic suggests using this roughness remapping for analytic lights.
	return GaSchlickG1(cosLi, k) * GaSchlickG1(NdotV, k);
}

vec3 RotateVectorAboutY(float radians, vec3 vec)
{
	mat3x3 rotationMatrix = 
	    { 
	        vec3(cos(radians),    0.0,    sin(radians)),
            vec3(0.0,           1.0,    0.0),
            vec3(-sin(radians),   0.0,    cos(radians)) 
        };
	return rotationMatrix * vec;
}

vec3 IBL(vec3 F0, vec3 Lr)
{
	vec3 irradiance = textureLod(sampler_IrradianceCube, PBRParams.Normal, 0.0).rgb;
	vec3 F = FresnelSchlickRoughness(F0, PBRParams.NdotV, PBRParams.Roughness);
	vec3 kd = (1.0 - F) * (1.0 - PBRParams.Metalness);
	vec3 diffuseIBL = PBRParams.Albedo * irradiance;

	int envRadianceTexLevels = textureQueryLevels(sampler_RadianceCube);
	float NoV = clamp(PBRParams.NdotV, 0.0, 1.0);
	vec3 R = 2.0 * dot(PBRParams.View, PBRParams.Normal) * PBRParams.Normal - PBRParams.View;
	vec3 specularIrradiance = textureLod(
	            sampler_RadianceCube, 
	            RotateVectorAboutY(EnvMapRotation, Lr), 
	            envRadianceTexLevels * PBRParams.Roughness).rgb;

	vec2 specularBRDF = texture(sampler_BRDFLUT, vec2(PBRParams.NdotV, PBRParams.Roughness)).rg;
	vec3 specularIBL = specularIrradiance * (F0 * specularBRDF.x + specularBRDF.y);

	return kd * diffuseIBL + specularIBL;
}

vec3 CalculateLighting(vec3 F0)
{
	vec3 Li = normalize(-LightDirection);
    vec3 LRadiance = LightRadiance * LightIntensity;
    vec3 Lh = normalize(Li + PBRParams.View);

	// Calculate angles between surface normal and various light vectors.
	float cosLi = max(0.0, dot(PBRParams.Normal, Li));
	float cosLh = max(0.0, dot(PBRParams.Normal, Lh));
	
	vec3 F = FresnelSchlickRoughness(F0, max(0.0, dot(Lh, PBRParams.View)), PBRParams.Roughness);
	float D = NdfGGX(cosLh, PBRParams.Roughness);
	float G = GaSchlickGGX(cosLi, PBRParams.NdotV, PBRParams.Roughness);

	vec3 kD = (1.0 - F) * (1.0 - PBRParams.Metalness);
	vec3 diffuseBRDF = kD * PBRParams.Albedo;

	// Cook-Torrance
	vec3 specularBRDF = (F * D * G) / max(Epsilon, 4.0 * cosLi * PBRParams.NdotV);
    specularBRDF = clamp(specularBRDF, vec3(0.0f), vec3(10.0f));

	vec3 result = (diffuseBRDF + specularBRDF) * LRadiance * cosLi;
	result += PBRParams.Albedo * Emission;
	return result;
}

void main()
{
	vec2 texCoord = VertexInput.TexCoord * TextureTiling;

	vec4 albedoTexColor = texture(sampler_AlbedoTexture, texCoord);
	PBRParams.Albedo = albedoTexColor.rgb * AlbedoColor;
	float alpha = albedoTexColor.a;

	PBRParams.Metalness = texture(sampler_MetalnessTexture, texCoord).r * Metalness;
	PBRParams.Roughness = texture(sampler_RoughnessTexture, texCoord).r * Roughness;
	PBRParams.Roughness = max(PBRParams.Roughness, 0.05); 

	PBRParams.Normal = normalize(VertexInput.Normal);
	if (UseNormalMap == 1)
	{
		PBRParams.Normal = normalize(texture(sampler_NormalTexture, texCoord).rgb * 2.0f - 1.0f);
		PBRParams.Normal = normalize(VertexInput.WorldNormals * PBRParams.Normal);
	}
	
	PBRParams.View = normalize(CameraPosition.xyz - VertexInput.WorldPosition);
	PBRParams.NdotV = max(0.0, dot(PBRParams.Normal, PBRParams.View));

	// Specular reflection vector
	vec3 Lr = 2.0 * PBRParams.NdotV * PBRParams.Normal - PBRParams.View;
	// Fresnel reflectance, metals use albedo
	vec3 F0 = mix(FresnelDialectric, PBRParams.Albedo, PBRParams.Metalness);

	vec3 di = CalculateLighting(F0);
    vec3 gi = IBL(F0, Lr) * EnvironmentIntensity;

	o_Color = vec4(di + gi, 1.0);
}