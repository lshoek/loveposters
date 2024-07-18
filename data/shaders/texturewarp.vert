// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#version 450 core

uniform nap
{
	mat4 projectionMatrix;
	mat4 viewMatrix;
	mat4 modelMatrix;
} mvp;

uniform UBO
{
	mat4 homographyMatrix;
	vec3 color;
	float alpha;
} ubo;

in vec3	in_Position;
in vec3	in_UV0;
out vec3 passUV0;

void main(void)
{
	// Calculate position
    gl_Position = ubo.homographyMatrix * mvp.viewMatrix * mvp.modelMatrix * vec4(in_Position, 1.0);

	// Pass color and uv's 
	passUV0 = in_UV0;
}
