// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#version 450 core

in vec2 passUV;

uniform sampler2D colorTexture;

out vec4 out_Color;

void main(void)
{
	vec4 color = texture(colorTexture, passUV);

	if (color.a < 0.5/255.0)
		discard;

	if (color.a <= 3.0/255.0)
	{
		out_Color = vec4(0.0, 0.0, 0.0, 1.0);
		return;
	}

	out_Color = color;
}
