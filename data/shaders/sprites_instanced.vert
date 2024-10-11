#version 450 core

// Extensions
#extension GL_GOOGLE_include_directive : enable

// Includes
#include "noise.glslinc"
#include "loveutils.glslinc"
#include "utils.glslinc"

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

out float passRot;

const float ROTATION_INTENSITY = 16.0;


void main()
{
	uint index = gl_VertexIndex;
	vec4 p = position[index];
	vec4 hash = hash[index];
	float t = ubo.elapsedTime;

	float rot_sign = mix(-1.0, 1.0, float(index%2));
	float rot_noise = (simplexd(p.xyz).w - 0.5) * rot_sign;
	passRot = t * ROTATION_INTENSITY * rot_noise;

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
	const float stretch = 2.0;
	const vec3 noise_coord = p.xyz*stretch + vec3(0.0, 0.0, 0.2);
	const float noise = simplexd(noise_coord).w * 0.5 + 0.5;

	const float pulse_speed = 30.0;
	const float pulse_time = (t + noise) * pulse_speed;
	const float pulse_size = sin(pulse_time) * ubo.pointSize * 0.5;

	const float noise_2 = simplexd(p.xyz).w * 0.5 + 0.5;
	const float pulse_time_2 = (t*0.5 + noise_2);
	const float scale_effect = sin(pulse_time_2) * 0.5 + 0.5;

	float point_size = ubo.pointSize * p.w + pulse_size + ubo.pointScale * scale_effect;
	gl_PointSize = point_size / length(view_position);
}
