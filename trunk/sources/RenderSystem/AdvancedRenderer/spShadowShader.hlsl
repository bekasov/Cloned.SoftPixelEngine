/*
 * Shadow D3D11 shader file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include <softpixelengine>

/*

Compilation options:

USE_VSM             -> Enables use of VSMs (variance shadow maps).
USE_RSM				-> Enables use of RMSs (reflective shadow maps).
USE_TEXTURE         -> Enables use of one texture for alpha test.
USE_TEXTURE_MATRIX  -> Enables texture matrix transformations for texture coordinates.

*/

/*
 * ======= Vertex shader: =======
 */

/* === Structures === */

struct SVertexInput
{
    float3 Position	: POSITION;
    #if defined(USE_TEXTURE) || defined(USE_RSM)
    float2 TexCoord	: TEXCOORD0;
    #endif
};

struct SVertexOutput
{
    float4 Position : SV_Position;
    float3 WorldPos : TEXCOORD0;
    #if defined(USE_TEXTURE) || defined(USE_RSM)
    float2 TexCoord : TEXCOORD1;
    #endif
};


/* === Buffers === */

cbuffer BufferMain : register(b0)
{
    float4x4 WorldViewProjectionMatrix;
    float4x4 WorldMatrix;
    float4x4 TextureMatrix;
    float4 ViewPosition;                //!< Global camera position.
};


/* === Functions === */

SVertexOutput VertexMain(SVertexInput In)
{
    SVertexOutput Out = (SVertexOutput)0;
    
    /* Process vertex transformation for perspective- and global position */
    Out.Position = mul(WorldViewProjectionMatrix, float4(In.Position, 1.0));
    Out.WorldPos = mul(WorldMatrix, float4(In.Position, 1.0)).xyz;

    /* Process texture coordinate */
    #ifdef USE_TEXTURE
    #   ifdef USE_TEXTURE_MATRIX
    Out.TexCoord = (float2)mul(TextureMatrix, float4(In.TexCoord, 0.0, 1.0));
    #   else
    Out.TexCoord = In.TexCoord;
    #   endif
    #endif
	
    return Out;
}


/*
 * ======= Pixel shader: =======
 */

/* === Structures === */

struct SPixelOutput
{
    #ifdef USE_VSM
    float4 DepthDist	: SV_Target0;
    #else
    float DepthDist		: SV_Target0;
    #endif
	#ifdef USE_RSM
	float3 Color		: SV_Target1;
	#endif
};


/* === Buffers === */

#if defined(USE_RSM)
SAMPLER2D(DiffuseMap, 0);
#elif defined(USE_TEXTURE)
SAMPLER2D(AlphaMap, 0);
#endif


/* === Functions === */

#ifdef USE_VSM
	#include "spShadowShaderProcs.shader"
#endif

#define DEPTH_DIST	Out.DepthDist
#define WORLD_POS	In.WorldPos
#define TEX_COORD	In.TexCoord

SPixelOutput PixelMain(SVertexOutput In)
{
    SPixelOutput Out = (SPixelOutput)0;
    
	#include "spShadowShaderMain.shader"
	
	#ifdef USE_RSM
	
	/* Final pixel output */
	Out.Color = Diffuse.rgb;
	
	#endif
	
    return Out;
}
