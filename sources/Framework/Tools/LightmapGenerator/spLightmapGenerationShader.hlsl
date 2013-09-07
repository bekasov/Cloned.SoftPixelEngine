/*
 * Lightmap generation D3D11 shader file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include <softpixelengine>

#include "spLightmapGenerationHeader.shader"


/* === Uniforms === */

cbuffer BufferMain : register(b0)
{
	float4x4 InvWorldMatrix	: packoffset(c0);
	float4 AmbientColor		: packoffset(c4);
	uint NumLights			: packoffset(c5);
	uint LightmapSize		: packoffset(c5.y);
	
	#if 0
	float1x4 t0;
	half1x1 t1;
	float3x2 t2;
	float2x2 t3;
	#endif
};

cbuffer BufferRadiositySetup : register(b1)
{
	uint NumRadiosityRays : packoffset(c0);
	float RadiosityFactor : packoffset(c0.y);	// (1.0 / NumRadiosityRays) * Factor
}

cbuffer BufferRadiosityRays : register(b2)
{
	float4 RadiosityDirections[MAX_NUM_RADIOSITY_RAYS];
};

StructuredBuffer<SLightSource> LightList : register(t0);
StructuredBuffer<SLightmapTexel> LightmapGrid : register(t1); 	// Active lightmap texels (one draw-call for every lightmap texture)

StructuredBuffer<STriangle> TriangleList : register(t2);
StructuredBuffer<SKDTreeNode> NodeList : register(t3);
Buffer<uint> TriangleIdList : register(t4);

/*
Input lightmap image (will be a copy of "OutputLightmap" after
direct illumination was computed, and will be used for indirect illumination).
*/
Texture2D<float4> InputLightmap : register(t5);
RWTexture2D<float4> OutputLightmap : register(u0);	// Output lightmap image

//RWBuffer<uint> StackBuffer : register(u1);	// One stack for each thread

groupshared SIdStack Stack;


/* === Functions === */

#include "spLightmapGenerationProcs.shader"

[numthreads(1, 1, 1)]
void ComputeDirectIllumination(uint3 Id : SV_DispatchThreadID)
{
	/* Get current lightmap texel */
	uint2 TexelPos = Id.xy;
	SLightmapTexel Texel = LightmapGrid[TexelPos.y * LightmapSize + TexelPos.x];
	
	/* Ignore invalid texels */
	if (!any(Texel.Normal))
		return;
	
	/* Get stack start offset */
	uint StackOffset = (TexelPos.x * LightmapSize + TexelPos.y) * (MAX_STACK_SIZE + 1);
	
	/* Generate lightmap texel for each light source */
	float4 Color = AmbientColor;
	
	for (uint i = 0; i < NumLights; ++i)
		GenerateLightmapTexel(Color.rgb, LightList[i], Texel, StackOffset);
	
	OutputLightmap[TexelPos] = Color;
}

[numthreads(1, 1, 1)]
void ComputeIndirectIllumination(uint3 Id : SV_DispatchThreadID)
{
	/* Get current lightmap texel */
	uint2 TexelPos = Id.xy;
	SLightmapTexel Texel = LightmapGrid[TexelPos.y * LightmapSize + TexelPos.x];
	
	/* Ignore invalid texels */
	if (!any(Texel.Normal))
		return;
	
	/* Generate normal matrix (tangent space) */
	float3x3 NormalMatrix = float3x3(
		Texel.Tangent,
		cross(Texel.Normal, Texel.Tangent),
		Texel.Normal
	);
	
	/* Get direct illuminated color */
	float4 Color = InputLightmap[TexelPos];
	
	/* Sample radiosity rays */
	SRay Ray;
	Ray.Origin = Texel.WorldPos;
	
	for (uint i = 0; i < NumRadiosityRays; ++i)
	{
		Ray.Direction = GetRandomRayDirection(NormalMatrix, i);
		ComputeRadiosityShading(Color.rgb, Ray, Texel.Normal);
	}
	
	/* Store final direct and indirect illuminated color */
	OutputLightmap[TexelPos] = Color;
}
