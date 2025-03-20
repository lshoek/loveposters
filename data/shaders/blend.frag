// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#version 450 core

in vec3 pass_UV;

out vec4 out_Color;

uniform sampler2D colorTexture;
uniform sampler2D backgroundTexture;

void main(void)
{	
	const vec4 front = texture(colorTexture, pass_UV.xy);
	const vec4 back = texture(backgroundTexture, pass_UV.xy);
	out_Color = vec4(mix(back.rgb, front.rgb, front.a), 1.0);
}
