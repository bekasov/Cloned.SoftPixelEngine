
// GLSL Test Compute Shader - 20/08/2013

#version 430

uniform writeonly image2D DestTex;

layout(local_size_x = 8, local_size_y = 8) in;

void main()
{
	uvec2 LocalId = gl_LocalInvocationID.xy;
	uvec2 GlobalId = gl_GlobalInvocationID.xy;
	
	imageStore(
		DestTex,
		ivec2(GlobalId),
		//vec4(float(LocalId.x)/8.0, float(LocalId.y)/8.0, 0.0, 1.0)
		vec4(
			sin(float(GlobalId.x)*0.1)*0.5 + 0.5 + float(LocalId.y)/8.0,
			cos(float(GlobalId.y)*0.1)*0.5 + 0.5 + float(LocalId.x)/8.0,
			0.0,
			1.0
		)
	);
}
