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
    float4x4 ProjectionMatrix;
    float4x4 InvViewProjection;
    float4x4 WorldMatrix;       //!< 2D quad world matrix.
    float3 ViewPosition;        //!< Global camera position.
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
#	else
#		define LG_RESOURCE_INDEX	t2
#		define TLI_RESOURCE_INDEX	t3
#	endif

// Dynamic tile light index list and 2D tile grid (for tiled deferred shading)
Buffer<uint> LightGrid : register(LG_RESOURCE_INDEX);

//#	define _DEB_USE_GROUP_SHARED_OPT_
#	ifdef _DEB_USE_GROUP_SHARED_OPT_
Buffer<uint> GlobalLightIdList : register(TLI_RESOURCE_INDEX);
#	else
StructuredBuffer<SLightNode> GlobalLightIdList : register(TLI_RESOURCE_INDEX);
#	endif

//#	define _DEB_DEPTH_EXTENT_//!!!
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

//!TODO! -> rename to "BufferVPL"
cbuffer VPLOffsetBlock : register(b4)
{
	float4 VPLOffsets[VPL_COUNT];
};

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
	
	#ifdef TILED_SHADING
	
	/* Get light count and offset from the tiled light grid */
	uint2 TilePos = GetTilePos(In.Position);
	uint TileIndex = TilePos.y * TILED_LIGHT_GRID_NUM_X + TilePos.x;
	
	uint Next = LightGrid[TileIndex];
	
	//#define _DEB_TILES_
	#ifdef _DEB_TILES_
	uint _DebTileNum_ = 0;
	#endif
	
	#ifdef _DEB_USE_GROUP_SHARED_OPT_
	while (1)
	#else
	while (Next != EOL)
	#endif
	{
		/* Get light node */
		#ifndef _DEB_USE_GROUP_SHARED_OPT_
		SLightNode Node = GlobalLightIdList[Next];
		#endif
		
		#ifdef _DEB_USE_GROUP_SHARED_OPT_
		uint i = GlobalLightIdList[Next];
		if (i == EOL)
			break;
		++Next;
		#else
		uint i = Node.LightID;
		
		/* Get next light node */
		Next = Node.Next;
		#endif
		
		#ifdef _DEB_TILES_
		++_DebTileNum_;
		#endif
	#else
    for (int i = 0; i < LightCount; ++i)
    {
	#endif
		ComputeLightShading(
			Lights[i], LightsEx[Lights[i].ExID],
			WorldPos, NormalAndDepthDist.xyz, SHININESS_FACTOR, ViewRayNorm,
			#ifdef HAS_LIGHT_MAP
			StaticDiffuseLight, StaticSpecularLight,
			#endif
			DiffuseLight, SpecularLight
		);
    }
	
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
	
	/* <<< Debugging part >>> */
	#if defined(_DEB_TILES_) || defined(_DEB_DEPTH_EXTENT_)
	float3 c_list[11] = {
		float3(0.0, 1.0, 0.0), float3(0.0, 0.8, 0.2),
		float3(0.0, 0.6, 0.4), float3(0.0, 0.4, 0.6),
		float3(0.0, 0.2, 0.8), float3(0.0, 0.0, 1.0),
		float3(0.2, 0.0, 0.8), float3(0.4, 0.0, 0.6),
		float3(0.6, 0.0, 0.4), float3(0.8, 0.0, 0.2),
		float3(1.0, 0.0, 0.0)
	};
	
	#	ifdef _DEB_TILES_
	if (_DebTileNum_ > 0)
	{
		_DebTileNum_ /= 2;
		float3 c = c_list[min(_DebTileNum_, 10)];
		Out.Color.rgb += c;
	}
	#	endif
	
	#	ifdef _DEB_DEPTH_EXTENT_
	float2 depth_ext = _debDepthExt_Out[TileIndex].xy;
	#		if 0
	float l = ((In.Position.x / TILED_LIGHT_GRID_WIDTH) - (float)TilePos.x);
	Out.Color.rg = lerp((float2)depth_ext.r * 0.1, (float2)depth_ext.g * 0.1, l);
	#		else
	int c_i = (int)(abs(depth_ext.x - depth_ext.y) * 0.5);
	float3 c = c_list[min(c_i, 10)];
	Out.Color.rgb += c;
	#		endif
	#	endif
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
