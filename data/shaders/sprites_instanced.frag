#version 450 core

// Extensions
#extension GL_GOOGLE_include_directive : enable

// Includes
#include "loveutils.glslinc"

uniform	sampler2D sprite;

in float passRot;

out vec4 out_FragColor;

const float ALPHA_CUTOFF = 0.99;

void main()
{
	vec2 uv = rotate(gl_PointCoord, vec2(0.5), passRot);
	vec4 col = texture(sprite, uv);
	if (col.a < ALPHA_CUTOFF) 
		discard;

	out_FragColor = col;
}
