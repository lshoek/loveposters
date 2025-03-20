// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#version 450 core

// Extensions
#extension GL_GOOGLE_include_directive : enable

// Includes
#include "noise.glslinc"

uniform UBO
{
	float decay;
	float expand;
	float vertical;
	float tangent;
	float expandBlend;
	float noiseScale;
	float elapsedTime;
};

in vec3 pass_Uvs;

out vec4 out_Color;

uniform sampler2D colorTexture;

void main(void)
{
	const vec2 size = textureSize(colorTexture, 0);
	const vec2 aspect = vec2(1.0, size.x/size.y);

	vec2 dx_outward = normalize(vec2(0.5)-pass_Uvs.xy) + vec2(0.0, vertical);
	vec2 turn = cross(vec3(0.5, 0.5, 0.0)-pass_Uvs, vec3(0.0, 0.0, 1.0)).xy;
	dx_outward += normalize(turn) * tangent;

	vec2 dx = simplexd(vec3((pass_Uvs.xy-0.5) * noiseScale, elapsedTime)).xy;
	vec2 shift = mix(dx, dx_outward, expandBlend) * 1.0/size;
	vec4 color = texture(colorTexture, pass_Uvs.xy + shift*expand);
	out_Color = color - decay;
}
