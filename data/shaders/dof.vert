#version 450

uniform nap
{
	mat4 projectionMatrix;
	mat4 viewMatrix;
	mat4 modelMatrix;
} mvp;

uniform UBO
{
	vec2 textureSize;	// The size of 'colorTexture', used to pre-calculate sampling coordinates in vertex shader
	vec2 direction;		// The sampling direction
	vec2 nearFar;		// camera near/far planes
	float aperture;
	float focalDistance;
	float focusDistance;
} ubo;

in vec3 in_Position;
in vec3 in_UV0;

out vec2 pass_UV;

void main()
{
	pass_UV = in_UV0.xy;
    gl_Position = mvp.projectionMatrix * mvp.viewMatrix * mvp.modelMatrix * vec4(in_Position, 1.0);
}
