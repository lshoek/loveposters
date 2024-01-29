#version 450 core

uniform	sampler2D sprite;

in vec4 passNDC;
in float passRot;

out vec4 out_FragColor;

const float ALPHA_CUTOFF = 0.99;

vec2 rotate(vec2 uv, vec2 offset, float theta) 
{
	vec2 centered = uv - offset;
	mat2 rot = mat2(
		cos(theta), -sin(theta),
		sin(theta), cos(theta)
	);
	return (rot * centered) + offset;
}

void main()
{
	vec2 uv = rotate(gl_PointCoord, vec2(0.5), passRot);
	vec4 col = texture(sprite, uv);
	if (col.a < ALPHA_CUTOFF) 
		discard;

	out_FragColor = col;
}
