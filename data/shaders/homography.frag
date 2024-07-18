// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#version 450 core

layout(constant_id = 0) const uint BORDER = 0;
layout(constant_id = 1) const uint GRID = 0;

// Uniforms
uniform UBO
{
	mat4 homographyMatrix;
};

// Sampler
uniform sampler2D colorTexture;

// Input
in vec2 PassUV;

// Fragment Output
out vec4 out_Color;

vec2 tile(vec2 st, float zoom)
{
	st *= zoom;
	return fract(st);
}

float box(vec2 st, vec2 size, float smoothEdges)
{
	size = vec2(0.5) - size*0.5;
	vec2 aa = vec2(smoothEdges*0.5);
	vec2 uv = smoothstep(size, size+aa, st);
	uv *= smoothstep(size, size+aa, vec2(1.0)-st);
	return uv.x*uv.y;
}

float grid(vec2 st)
{
    st = tile(st, 16.0);
    return 1.0 - box(st, vec2(0.95), 0.015);
}

void main()
{
	// vec3 trans = mat3(homographyMatrix) * vec3(PassUV, 1.0); 
	// vec2 coords = vec2(trans.x, trans.y) / trans.z;

	const vec2 size = textureSize(colorTexture, 0);
	vec3 p = vec3(PassUV.x * size.x, PassUV.y * size.y, 1.0); 
	vec3 trans = mat3(homographyMatrix) * p;
	vec2 coords = vec2(trans.x/size.x, trans.y/size.y) / trans.z;

	vec4 color = texture(colorTexture, coords);

	if (GRID > 0)
	{
		color += grid(coords) * 0.5;
	}

	if (BORDER > 0)
	{
		const float edge = 0.005;
		const vec4 red = vec4(1.0, 0.0, 0.0, 1.0);
		color += red * (box(coords, vec2(1.0), edge) - box(coords, vec2(0.99), edge));
	}
	
	out_Color = color;
}
