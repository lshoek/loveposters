// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#version 450 core

uniform UBO
{
	mat4 homographyMatrix;
	vec3 color;
	float alpha;
} ubo;

uniform sampler2D colorTexture;

in vec3 passUV0;
out vec4 out_Color;

void main(void)
{
	vec4 color = texture(colorTexture, passUV0.xy);
	out_Color = vec4(color.rgb * ubo.color, color.a * ubo.alpha);
}
