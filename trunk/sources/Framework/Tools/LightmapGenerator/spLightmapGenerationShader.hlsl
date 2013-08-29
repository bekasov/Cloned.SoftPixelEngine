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
	float4x4 WVPMatrix			: packoffset(c0);
	float4 AmbientColor			: packoffset(c4);
	uint NumLights				: packoffset(c5);
	uint2 LightmapSize			: packoffset(c5.y);
};

cbuffer BufferRadiositySetup : register(b1)
{
	uint NumRadiosityRays : packoffset(c0);
	float RadiosityFactor : packoffset(c0.y);	// (1.0 / NumRadiosityRays) * Factor
}

cbuffer BufferRadiosityRays : register(b2)
{
	float4 RadiosityDirections[4096];
};

StructuredBuffer<SLightSource> LightList : register(t0);
StructuredBuffer<SLightmapTexel> Lightmap : register(t1); 	// Active lightmap texels (one draw-call for every lightmap texture)

StructuredBuffer<STriangle> TriangleList : register(t2);
StructuredBuffer<SKDTreeNode> NodeList : register(t3);
Buffer<uint> TriangleIdList : register(t4);

RWTexture2D<float4> OutputLightmap : register(u0);

groupshared SIdStack Stack;


/* === Functions === */

#include "spLightmapGenerationProcs.shader"

[numthreads(1, 1, 1)]
void ComputeDirectIllumination(uint3 Id : SV_DispatchThreadID)
{
	float4 Color = AmbientColor;
	
	/* Get current lightmap texel */
	uint2 TexelPos = Id.xy;
	SLightmapTexel Texel = Lightmap[TexelPos.y * LightmapSize.x + TexelPos.x];
	
	/* Generate lightmap texel for each light source */
	for (uint i = 0; i < NumLights; ++i)
		GenerateLightmapTexel(Color.rgb, LightList[i], Texel);
	
	OutputLightmap[TexelPos] = Color;
}

[numthreads(1, 1, 1)]
void ComputeIndirectIllumination(uint3 Id : SV_DispatchThreadID)
{
	float4 Color = (float4)0.0;
	
	/* Get current lightmap texel */
	uint2 TexelPos = Id.xy;
	SLightmapTexel Texel = Lightmap[TexelPos.y * LightmapSize.x + TexelPos.x];
	
	/* Generate normal matrix (tangent space) */
	float3x3 NormalMatrix = float3x3(
		Texel.Tangent,
		cross(Texel.Normal, Texel.Tangent),
		Texel.Normal
	);
	
	/* Sample radiosity rays */
	SRay Ray;
	Ray.Origin = Texel.WorldPos;
	
	for (uint i = 0; i < NumRadiosityRays; ++i)
	{
		Ray.Direction = GetRandomRayDirection(NormalMatrix, i);
		ComputeRadiosityShading(Color, Ray, Texel.Normal);
	}
	
	InterlockedAdd(OutputLightmap[TexelPos], Color);
}
