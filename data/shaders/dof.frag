#version 450

// Extensions
#extension GL_GOOGLE_include_directive : enable

// Includes
#include "shadow.glslinc"

uniform UBO
{
	vec2 textureSize;	// The size of 'colorTexture', used to pre-calculate sampling coordinates in vertex shader
	vec2 direction;		// The sampling direction
	vec2 nearFar;		// camera near/far planes
	float aperture;
	float focalDistance;
	float focusDistance;
} ubo;

uniform sampler2D colorTexture;		// The input color texture to sample from
uniform sampler2D depthTexture;		// The input color texture to sample from

in vec2 pass_UV;
out vec4 out_Color;

const float EPSILON = 0.000001;

// Sampler must be configured with filter VK_FILTER_LINEAR
vec4 blur(sampler2D tx, vec4 col, float s) 
{
	const vec2 off1 = ubo.direction * (1.0 / ubo.textureSize);
	const vec2 off2 = ubo.direction * (2.0 / ubo.textureSize);
	const vec2 off3 = ubo.direction * (3.0 / ubo.textureSize);

	col = col * 0.1964825502;
	col += texture(tx, pass_UV + off1 * s) * 0.2969069647;
	col += texture(tx, pass_UV + off1 * s) * 0.2969069647;
	col += texture(tx, pass_UV + off2 * s) * 0.0944703979;
	col += texture(tx, pass_UV + off2 * s) * 0.0944703979;
	col += texture(tx, pass_UV + off3 * s) * 0.0103813624;
	col += texture(tx, pass_UV + off3 * s) * 0.0103813624;

	return col;
}


void main() 
{
	// Alpha test
	vec4 frag_col = texture(colorTexture, pass_UV);
	if (frag_col.a <= EPSILON)
		discard;

	// Adding this small constant to resolve sampling artifacts in cube seams
	float near = min(ubo.nearFar.x + 0.001, ubo.nearFar.y);
	float far = ubo.nearFar.y;

	float frag_depth = texture(depthTexture, pass_UV).x;
	float linear_depth = linearDepth(frag_depth, near, far);

	// https://developer.nvidia.com/gpugems/gpugems/part-iv-image-processing/chapter-23-depth-field-survey-techniques
	const float aperture = ubo.aperture;
	const float focal = ubo.focalDistance;
	const float focus = ubo.focusDistance;

	float coc_scale = (aperture * focal * focus * (far - near)) / ((focus - focal) * near * far);
	float coc_bias = (aperture * focal * (near - focus)) / ((focus * focal) * near);
	float coc = abs(linear_depth * coc_scale + coc_bias);

	out_Color = blur(colorTexture, frag_col, coc);
}
 