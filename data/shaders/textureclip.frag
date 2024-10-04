// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#version 450 core

in vec2 passUV;

out vec4 out_Color;

uniform sampler2D colorTexture;

const float ALPHA_CLIP = 0.75;

void main(void)
{
	if (texture(colorTexture, passUV).a < ALPHA_CLIP)
		discard;

	gl_FragDepth = gl_FragCoord.z;
}
