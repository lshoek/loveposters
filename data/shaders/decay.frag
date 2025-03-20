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
	float expandBlend;
	float noiseScale;
	float elapsedTime;
};

in vec3 pass_Uvs;

out vec4 out_Color;

uniform sampler2D colorTexture;

void main(void)
{
	vec2 dx_outward = normalize(vec2(0.5)-pass_Uvs.xy);
	vec2 dx = simplexd(vec3((pass_Uvs.xy-0.5) * noiseScale, elapsedTime)).xy;
	vec2 shift = mix(dx_outward, dx, expandBlend) * 1.0/textureSize(colorTexture, 0);
	vec4 color = texture(colorTexture, pass_Uvs.xy + shift*expand);
	out_Color = color - decay;
}
