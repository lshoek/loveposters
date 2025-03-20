// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#version 450 core

uniform UBO
{
	vec3 colors[5];
	float elapsedTime;
};

in vec3 pass_Uvs;
out vec4 out_Color;

uniform sampler2D colorTexture;

// const vec4 colors[] = {
// 	vec4(1.0, 178/255.0, 0.0, 1.0),
// 	vec4(235/255.0, 91/255.0, 0.0, 1.0),
// 	vec4(217/255.0, 22/255.0, 86/255.0, 1.0),
// 	vec4(100/255.0, 13/255.0, 95/255.0, 1.0),
// 	vec4(1.0, 178/255.0, 0.0, 1.0)
// };

void main(void)
{
	float x = texture(colorTexture, pass_Uvs.xy).r;
	x = fract(x + elapsedTime);

	int col_idx[] = {
		clamp(int(floor(x * float(colors.length()))), 0, colors.length()-1),
		clamp(int(ceil(x * float(colors.length()))), 0, colors.length()-1)
	};
	float factor = x * float(colors.length()) - float(col_idx[0]);
	vec3 col = mix(colors[col_idx[0]], colors[col_idx[1]], factor);

	out_Color = vec4(col, 1.0);
}
