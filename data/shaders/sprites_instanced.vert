#version 450 core

// Extensions
#extension GL_GOOGLE_include_directive : enable
#include "noise.glslinc"

// STORAGE
layout(std430) readonly buffer PositionBuffer_In
{
	vec4 position[4096];
};

layout(std430) readonly buffer HashBuffer_In
{
	vec4 hash[4096];
};


uniform nap
{
	mat4 projectionMatrix;
	mat4 viewMatrix;
	mat4 modelMatrix;
	vec3 cameraPosition;
} mvp;

uniform UBO
{
	float elapsedTime;
	float pointSize;
	float pointScale;
} ubo;

out vec4 passColor;
out float passRot;

const float ROTATION_INTENSITY = 32.0;


void main()
{
	uint index = gl_VertexIndex;
	vec4 p = position[index];
	vec4 hash = hash[index];
	float t = ubo.elapsedTime;

	float rot_sign = mix(-1.0, 1.0, float(index%2));
	float rot_noise = (simplexd(p.xyz).w - 0.5) * rot_sign;
	passRot = ubo.elapsedTime * ROTATION_INTENSITY * rot_noise;

	// Generate variation
	vec4 hash1k = hash * 1000.0;
	vec3 sway = { 
		simplexd(vec3(t, hash1k.x, 0.0)).w,
		simplexd(vec3(t, hash1k.y, 0.0)).w,
		simplexd(vec3(t, hash1k.z, 0.0)).w
	};
	sway *= 0.125;

	vec4 view_position = mvp.viewMatrix * mvp.modelMatrix * vec4(p.xyz + sway, 1.0);
	vec4 clip = mvp.projectionMatrix * view_position;
	gl_Position = clip;
	
	// Point size
	float point_size = ubo.pointSize * p.w + ubo.pointScale * hash.w;
	gl_PointSize = point_size / length(view_position);
}
