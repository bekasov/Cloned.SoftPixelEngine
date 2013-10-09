/*
 * Deferred D3D11 shader file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include <softpixelengine>

#include "spDeferredShaderHeader.shader"

/*

Compilation options:

SHADOW_MAPPING  -> Enables shadow mapping.
BLOOM_FILTER    -> Enables bloom filter.
FLIP_Y_AXIS     -> Flips Y axis for OpenGL FBOs.
DEBUG_GBUFFER   -> Renders g-buffer for debugging.

*/

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
    float4x4 ProjectionMatrix	: packoffset(c0);
    float4x4 InvViewProjection	: packoffset(c4);
    float4x4 WorldMatrix		: packoffset(c8);		//!< 2D quad world matrix.
    float3 ViewPosition			: packoffset(c12);		//!< Global camera position.
	uint LightGridRowSize		: packoffset(c12.w);	//!< Number of tiles in a row (if tiled shading is enabled).
};


/* === Functions === */

void VecFrustum(inout float2 v)
{
	v = (v - 0.5) * 2.0;
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
	
    VecFrustum(Out.ViewRay.xy);
	Out.ViewRay = mul(InvViewProjection, Out.ViewRay);

    return Out;
}


/*
 * ======= Pixel shader: =======
 */

//#define _DEB_TILES_
//#define _DEB_DEPTH_EXTENT_
#define _DEB_USE_GROUP_SHARED_OPT_

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

// Shadow maps (for standard shadow maps)
SAMPLER2DARRAY(DirLightShadowMaps, 2);
SAMPLERCUBEARRAY(PointLightShadowMaps, 3);

#	ifdef GLOBAL_ILLUMINATION
// Diffuse maps (for reflective shadow maps)
SAMPLER2DARRAY(DirLightDiffuseMaps, 4);
SAMPLERCUBEARRAY(PointLightDiffuseMaps, 5);
#	endif

#endif

#ifdef TILED_SHADING

#	ifdef SHADOW_MAPPING
#		define LG_RESOURCE_INDEX	t4
#		define TLI_RESOURCE_INDEX	t5
#		define VPL_POSITION_INDEX	t6
#		define VPL_COLOR_INDEX		t7
#	else
#		define LG_RESOURCE_INDEX	t2
#		define TLI_RESOURCE_INDEX	t3
#		define VPL_POSITION_INDEX	t4
#		define VPL_COLOR_INDEX		t5
#	endif

// Dynamic tile light index list and 2D tile grid (for tiled deferred shading)
Buffer<uint> LightGrid : register(LG_RESOURCE_INDEX);

#	ifdef _DEB_USE_GROUP_SHARED_OPT_
Buffer<uint> GlobalLightIdList : register(TLI_RESOURCE_INDEX);
#	else
StructuredBuffer<SLightNode> GlobalLightIdList : register(TLI_RESOURCE_INDEX);
#	endif

#	ifdef _DEB_DEPTH_EXTENT_
Buffer<float2> _debDepthExt_Out : register(t4);
#	endif

#endif

cbuffer BufferShading : register(b1)
{
    float3 AmbientColor 	: packoffset(c0);	//!< Ambient light color.
	float GIReflectivity	: packoffset(c0.w);	//!< Global illumination reflectivity.
    int LightCount			: packoffset(c1.x);	//!< Count of light sources.
};

cbuffer BufferLight : register(b2)
{
    SLight Lights[MAX_LIGHTS];
};

cbuffer BufferLightEx : register(b3)
{
    SLightEx LightsEx[MAX_EX_LIGHTS];
};

#ifdef GLOBAL_ILLUMINATION

cbuffer BufferVPL : register(b4)
{
	float4 VPLOffsets[VPL_COUNT];
};

#	ifdef TILED_SHADING

Buffer<float4> VPLPositionList	: register(VPL_POSITION_INDEX);
Buffer<float4> VPLColorList		: register(VPL_COLOR_INDEX);

#	endif

#endif

/* === Functions === */

#include "spDeferredShaderProcs.shader"

SPixelOutput PixelMain(SVertexOutput In)
{
    SPixelOutput Out = (SPixelOutput)0;

    /* Compute global pixel position (world space) */
    float4 NormalAndDepthDist = tex2D(NormalAndDepthMap, In.TexCoord);

	NormalAndDepthDist.xyz = normalize(NormalAndDepthDist.xyz);
	
	float3 ViewRayNorm = normalize(In.ViewRay.xyz);
    float3 WorldPos = ViewPosition + ViewRayNorm * (float3)NormalAndDepthDist.a;
	
    /* Compute light shading */
	#ifdef HAS_LIGHT_MAP
    float3 StaticDiffuseLight = 0.0;
    float3 StaticSpecularLight = 0.0;
    #endif
	
    float3 DiffuseLight = AmbientColor;
    float3 SpecularLight = 0.0;
	
	#ifdef _DEB_TILES_
	uint _DebTileNum_ = 0;
	#endif
	
	ComputeShading(
		In.Position, WorldPos, NormalAndDepthDist.xyz, SHININESS_FACTOR, ViewRayNorm,
		#ifdef HAS_LIGHT_MAP
		StaticDiffuseLight, StaticSpecularLight,
		#endif
		DiffuseLight, SpecularLight
		#ifdef _DEB_TILES_
		,_DebTileNum_;
		#endif
	);
	
	#ifdef HAS_LIGHT_MAP
	
	/* Mix light shading with light-map illumination */
	float Illumination = tex2D(IlluminationMap, In.TexCoord).r;
	
	DiffuseLight += (StaticDiffuseLight * Illumination);
	SpecularLight += (StaticSpecularLight * Illumination);
	
	#endif
	
	#ifndef ALLOW_OVERBLENDING
	DiffuseLight = saturate(DiffuseLight);
	SpecularLight = saturate(SpecularLight);
	#endif
	
    /* Get diffuse and specular colors */
	float4 DiffuseAndSpecular = tex2D(DiffuseAndSpecularMap, In.TexCoord);
	
    DiffuseLight *= DiffuseAndSpecular.rgb;
    SpecularLight *= DiffuseAndSpecular.a;
	
    /* Compute final deferred shaded pixel color */
    Out.Color.rgb   = DiffuseLight + SpecularLight;
    Out.Color.a     = 1.0;
	
	#ifdef BLOOM_FILTER
    Out.Specular.rgb    = SpecularLight;
    Out.Specular.a      = 1.0;
    #endif
	
	/* <<<------- Debugging part ------->>> */
	
	#if defined(_DEB_TILES_) || defined(_DEB_DEPTH_EXTENT_)
	_DebDrawTileUsage_(In.Position, _DebTileNum_, Out.Color.rgb);
	#endif
	
    #ifdef DEBUG_GBUFFER
	
	#   ifdef DEBUG_GBUFFER_WORLDPOS
	WorldPos += 0.01;
    Out.Color.rgb = WorldPos - floor(WorldPos);
	#   else
    //Out.Color.rgb = (float3)(NormalAndDepthDist.a - floor(NormalAndDepthDist.a));
	Out.Color.rgb = NormalAndDepthDist.xyz * (float3)0.5 + (float3)0.5;
	#   endif
	
    #endif
	
    return Out;
}
