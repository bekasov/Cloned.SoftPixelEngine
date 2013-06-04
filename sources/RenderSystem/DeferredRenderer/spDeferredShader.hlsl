/*
 * Deferred D3D11 shader file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

/*

Compilation options:

SHADOW_MAPPING  -> Enables shadow mapping.
BLOOM_FILTER    -> Enables bloom filter.
FLIP_Y_AXIS     -> Flips Y axis for OpenGL FBOs.
DEBUG_GBUFFER   -> Renders g-buffer for debugging.

*/

#include "spDeferredShaderHeader.shader"
#include "spDeferredShaderProcs.shader"

/*
 * ======= Vertex shader: =======
 */

/* === Structures === */

struct SVertexInput
{
    float2 Position : POSITION;
    float2 TexCoord : TEXCOORD0;
};

struct SVertexOutput
{
    float4 Position : SV_Position;
    float2 TexCoord : TEXCOORD0;
    float4 ViewRay : TEXCOORD1;
};


/* === Uniforms === */

cbuffer BufferMain : register(b0)
{
    float4x4 ProjectionMatrix;
    float4x4 InvViewProjection;
    float4x4 WorldMatrix;       //!< 2D quad world matrix.
    float3 ViewPosition;        //!< Global camera position.
};


/* === Functions === */

void Frustum(inout float4 v)
{
    v.x = (v.x - 0.5) * 2.0;
    v.y = (v.y - 0.5) * 2.0;
}

SVertexOutput VertexMain(SVertexInput In)
{
    SVertexOutput Out = (SVertexOutput)0;
    
    /* Process vertex transformation */
    float2 Coord = mul((float2x2)WorldMatrix, In.Position);

    Out.Position = mul(ProjectionMatrix, float4(Coord.x, Coord.y, 0.0, 1.0));

    /* Process vertex transformation for position and normal */
    Out.TexCoord = In.TexCoord;
    
	/* Pre-compute view ray */
    Out.ViewRay = float4(In.TexCoord.x, 1.0 - In.TexCoord.y, 1.0, 1.0);
	
    Frustum(Out.ViewRay);
	
	Out.ViewRay = mul(InvViewProjection, Out.ViewRay);

    return Out;
}


/*
 * ======= Pixel shader: =======
 */

/* === Structures === */

struct SPixelOutput
{
    float4 Color : SV_Target0;
    #ifdef BLOOM_FILTER
    float4 Specular : SV_Target1;
    #endif
};


/* === Uniforms === */

SAMPLER2D(DiffuseAndSpecularMap, 0);
SAMPLER2D(NormalAndDepthMap, 1);

#ifdef SHADOW_MAPPING
SAMPLER2DARRAY(DirLightShadowMaps, 2);
SAMPLERCUBEARRAY(PointLightShadowMaps, 3);
#endif

#ifdef TILED_SHADING

// Dynamic tile light index list and 2D tile grid (for tiled deferred shading)
tbuffer TileLightIndexList : register(t4)
{
	int TileLightIndices[100];
};

#if 1//!!!
#define TILED_LIGHT_GRID_NUM_X 8
#define TILED_LIGHT_GRID_NUM_Y 8
#endif

cbuffer BufferLightGrid : register(b5)
{
	int4 LightGrid[TILED_LIGHT_GRID_NUM_X * TILED_LIGHT_GRID_NUM_Y];
};

#endif

cbuffer BufferShading : register(b1)
{
    float3 AmbientColor : packoffset(c0);	//!< Ambient light color.
    int LightCount		: packoffset(c0.w);	//!< Count of light sources.
};

cbuffer BufferLight : register(b2)
{
    SLight Lights[MAX_LIGHTS];
};

cbuffer BufferLightEx : register(b3)
{
    SLightEx LightsEx[MAX_EX_LIGHTS];
};


/* === Functions === */

SPixelOutput PixelMain(SVertexOutput In)
{
    SPixelOutput Out = (SPixelOutput)0;

    /* Get texture colors */
	float4 DiffuseAndSpecular = tex2D(DiffuseAndSpecularMap, In.TexCoord);
    float4 NormalAndDepthDist = tex2D(NormalAndDepthMap, In.TexCoord);

    #if 1//!!!
    NormalAndDepthDist.xyz = normalize(NormalAndDepthDist.xyz);
    #endif
	
    /* Compute global pixel position (world space) */
	float3 ViewRayNorm = normalize(In.ViewRay.xyz);
    float3 WorldPos = ViewPosition + ViewRayNorm * (float3)NormalAndDepthDist.a;
	
    /* Compute light shading */
	#ifdef HAS_LIGHT_MAP
    float3 StaticDiffuseLight = 0.0;
    float3 StaticSpecularLight = 0.0;
    #endif
	
    float3 DiffuseLight = AmbientColor;
    float3 SpecularLight = 0.0;
	
    for (int i = 0, j = 0; i < LightCount; ++i)
    {
		ComputeLightShading(
			Lights[i], LightsEx[j], WorldPos.xyz, NormalAndDepthDist.xyz, 90.0, ViewRayNorm,
			#ifdef HAS_LIGHT_MAP
			StaticDiffuseLight, StaticSpecularLight,
			#endif
			DiffuseLight, SpecularLight
		);
        
        if (Lights[i].Type != LIGHT_POINT)
            ++j;
    }
	
	#if 0//!!!
	SpecularLight += (float3)((float)(LightGrid[0] * TileLightIndices[0]) * 0.001);
	#endif
	
	#ifdef HAS_LIGHT_MAP
	
	/* Mix light shading with light-map illumination */
	float Illumination = tex2D(IlluminationMap, TexCoord).r;
	
	DiffuseLight += (StaticDiffuseLight * Illumination);
	SpecularLight += (StaticSpecularLight * Illumination);
	
	#endif
	
	#ifndef ALLOW_OVERBLENDING
	DiffuseLight = saturate(DiffuseLight);
	SpecularLight = saturate(SpecularLight);
	#endif
	
    DiffuseLight *= DiffuseAndSpecular.rgb;
    SpecularLight *= DiffuseAndSpecular.a;
	
    /* Compute final deferred shaded pixel color */
    Out.Color.rgb   = DiffuseLight + SpecularLight;
    Out.Color.a     = 1.0;
	
	#ifdef BLOOM_FILTER
    Out.Specular.rgb    = SpecularLight;
    Out.Specular.a      = 1.0;
    #endif

    #if 0//!!!
    Out.Color.rgb = NormalAndDepthDist.xyz * 0.5 + 0.5;
    #endif

    #ifdef DEBUG_GBUFFER
	
	#   ifdef DEBUG_GBUFFER_WORLDPOS
	WorldPos.xyz += 0.01;
    Out.Color.rgb = WorldPos.xyz - floor(WorldPos.xyz);
	#   else
    Out.Color.rgb = float3(NormalAndDepthDist.a - floor(NormalAndDepthDist.a));
	#   endif
	
    #endif

    return Out;
}
