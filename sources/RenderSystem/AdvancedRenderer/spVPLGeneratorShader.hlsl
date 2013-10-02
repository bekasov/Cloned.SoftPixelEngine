/*
 * VPL generator D3D11 compute shader file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "spDeferredShaderHeader.shader"

/*
 * ======= Compute shader: =======
 */

/* === Macros === */

#define NUM_MAX_LIGHTS		16

#define THREAD_GROUP_NUM_X	8
#define THREAD_GROUP_NUM_Y	8
#define THREAD_GROUP_SIZE	(THREAD_GROUP_NUM_X * THREAD_GROUP_NUM_Y)


/* === Buffers === */

cbuffer BufferMain : register(b0)
{
	uint NumVPLsPerLight	: packoffset(c0);
	uint3 NumThreadGroups	: packoffset(c0.y);
};

cbuffer BufferLights : register(b1)
{
    SLight Lights[NUM_MAX_LIGHTS];
    SLightEx LightsEx[NUM_MAX_LIGHTS];
};

//RWStructuredBuffer<SVPL> VPLList : register(u0);
RWBuffer<float4> VPLPositionList	: register(u0);
RWBuffer<float4> VPLColorList		: register(u1);

Texture2DArray DirLightShadowMaps		: register(t0);
TextureCubeArray PointLightShadowMaps	: register(t1);

Texture2DArray DirLightDiffuseMaps		: register(t3);
TextureCubeArray PointLightDiffuseMaps	: register(t4);

SamplerState ShadowMapSampler
{
	Filter = MIN_MAG_MIP_POINT;
	AddressU = Wrap;
	AddressV = Wrap;
};


/* === Functions === */

void GenerateVPL(SLight Lit, SLightEx LitEx, uint Index)
{
	SVPL VPL;
	
	//todo...
	
	VPLList[Index] = VPL;
}


/* === Main compute shader === */

[numthreads(THREAD_GROUP_NUM_X, THREAD_GROUP_NUM_Y, 1)]
void ComputeMain(
	uint3 GroupId : SV_GroupID,
	uint LocalIndex : SV_GroupIndex)
{
	/* Get VPL index for current light */
	uint VPLIndex = GroupId.x * THREAD_GROUP_SIZE + LocalIndex;
	
	if (VPLIndex < NumVPLsPerLight)
	{
		/* Get current light */
		uint LightIndex = GroupId.y;
		
		SLight Lit = Lights[LightIndex];
		SLightEx LitEx = LightsEx[LightIndex];
		
		/* Final VPL index for global list */
		VPLIndex += LightIndex * NumVPLsPerLight;
		
		/* Generate VPL with current index (each thread only generates one VPL) */
		GenerateVPL(Lit, LitEx, VPLIndexOffset + i);
	}
}
