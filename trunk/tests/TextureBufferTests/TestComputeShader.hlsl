
// Test HLSL Compute Shader

#if 0

struct SBlock
{
	float4 Color;
};

RWStructuredBuffer<SBlock> PixelColors : register(u0);

#else

RWBuffer<float4> PixelColors : register(u0);

#endif

[numthreads(1, 1, 1)]
void ComputeMain(uint3 Id : SV_DispatchThreadID)
{
	uint i = Id.y*4 + Id.x;
	
	#if 0
	PixelColors[i].Color = float4(
	#else
	PixelColors[i] = float4(
	#endif
		((float)Id.x) / 3.0,
		((float)Id.y) / 3.0,
		0.0,
		1.0
	);
}

