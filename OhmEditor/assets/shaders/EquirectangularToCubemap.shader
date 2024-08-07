#type compute
#version 450 core

layout(binding = 0, rgba16f) restrict writeonly uniform imageCube o_CubeMap;
uniform sampler2D sampler_EquirectangularTexture;

#define PI 3.14159265359

vec3 GetCubeMapTexCoord(vec2 imageSize)
{
    vec2 st = gl_GlobalInvocationID.xy / imageSize;
    vec2 uv = 2.0 * vec2(st.x, 1.0 - st.y) - vec2(1.0);

    vec3 ret;
    if      (gl_GlobalInvocationID.z == 0) ret = vec3(  1.0, uv.y, -uv.x);
    else if (gl_GlobalInvocationID.z == 1) ret = vec3( -1.0, uv.y,  uv.x);
    else if (gl_GlobalInvocationID.z == 2) ret = vec3( uv.x,  1.0, -uv.y);
    else if (gl_GlobalInvocationID.z == 3) ret = vec3( uv.x, -1.0,  uv.y);
    else if (gl_GlobalInvocationID.z == 4) ret = vec3( uv.x, uv.y,   1.0);
    else if (gl_GlobalInvocationID.z == 5) ret = vec3(-uv.x, uv.y,  -1.0);
    return normalize(ret);
}

layout(local_size_x = 32, local_size_y = 32, local_size_z = 1) in;
void main()
{
	vec3 CubeCoords = GetCubeMapTexCoord(vec2(imageSize(o_CubeMap)));

    // Calculate sampling coords for equirectangular texture
	// https://en.wikipedia.org/wiki/Spherical_coordinate_system#Cartesian_coordinates
	float Phi = atan(CubeCoords.z, CubeCoords.x);
	float Theta = acos(CubeCoords.y);
    vec2 UV = vec2(Phi / (2.0 * PI) + 0.5, Theta / PI);

	vec4 Color = texture(sampler_EquirectangularTexture, UV);
	imageStore(o_CubeMap, ivec3(gl_GlobalInvocationID), Color);
}

