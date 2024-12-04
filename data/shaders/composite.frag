// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#version 450 core

uniform FRAGUBO
{
	float blend;
	float abberation;
	float brightness;
} ubo;

in vec3 pass_UV;

out vec4 out_Color;

uniform sampler2D colorTextures[2];

vec4 chromatic_abberation_sample(sampler2D tex, vec2 uv)
{
	const vec2 ctr = { 0.5, 0.5 };
	const ivec2 res = textureSize(tex, 0);
	vec2 diff = uv - ctr;
	float intensity = clamp(length(diff), 0.0, 1.0);
	vec2 d = normalize(diff) * intensity * ubo.abberation;

	vec4 col = vec4(0.0);
	col.r = texture(tex, uv + d).r;
	col.ga = texture(tex, uv).ga;
	col.b = texture(tex, uv - d).b;

	return col;
}

// @param color: input color
// @param value: [-1, 1] where negative reduces brightness and positive increases it
vec3 brightness(vec3 color, float value) 
{
	return clamp(color + value, 0.0, 1.0);
} 

void main(void)
{	
	// Get texel color values
	vec4 col0 = chromatic_abberation_sample(colorTextures[0], pass_UV.xy);
	vec4 col1 = texture(colorTextures[1], pass_UV.xy);

	// Get screened blend color
	const vec3 vunit = vec3(1.0, 1.0, 1.0);
	vec3 screen_color = vunit-(vunit-col0.rgb)*(vunit-col1.rgb);

	// Blend into original based on blend value
	vec3 color = mix(col0.rgb, screen_color, ubo.blend);

	// Brightness post-processing
	color = brightness(color.rgb, ubo.brightness);

	out_Color = vec4(color, 1.0);
}
