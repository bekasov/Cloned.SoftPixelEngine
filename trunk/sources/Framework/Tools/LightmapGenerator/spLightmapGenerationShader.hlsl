/*
 * Lightmap generation D3D11 shader file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include <softpixelengine>

#include "spLightmapGenerationHeader.shader"

/*
 * ======= Vertex shader: =======
 */

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


/*
 * ======= Compute shader: =======
 */

/* === Uniforms === */

StructuredBuffer<SLightSource> LightList : register(t0);
StructuredBuffer<SLightmapTexel> Lightmap : register(t1); 	// Active lightmap texels (one draw-call for every lightmap texture)

StructuredBuffer<STriangle> TriangleList : register(t2);
StructuredBuffer<SKDTreeNode> NodeList : register(t3);
Buffer<uint> TriangleIdList : register(t4);

RWTexture2D<float4> OutputLightmap : register(u0);


/* === Functions === */

#include "spLightmapGenerationProcs.shader"

[numthreads(8, 8, 1)]
void ComputeMain(
	uint3 GroupId : SV_GroupID,
	uint3 LocalId : SV_GroupThreadID,
	uint3 GlobalId : SV_DispatchThreadID)
{
	float4 Color = AmbientColor;
	
	/* Get current lightmap texel */
	uint2 TexelPos = GlobalId.xy;
	SLightmapTexel Texel = Lightmap[TexelPos.y * LightmapSize.x + TexelPos.x];
	
	/* Generate lightmap texel for each light source */
	for (uint i = 0; i < NumLights; ++i)
		GenerateLightmapTexel(Color.rgb, LightList[i], Texel);
	
	OutputLightmap[TexelPos] = Color;
}

