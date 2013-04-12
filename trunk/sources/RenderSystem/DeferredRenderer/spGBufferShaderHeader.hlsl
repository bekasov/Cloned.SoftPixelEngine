/*
 * GBuffer D3D11 shader header file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

/*

Compilation options:

USE_TEXTURE_MATRIX  -> Enables texture matrix transformations for texture coordinates.
NORMAL_MAPPING      -> Enables normal mapping.
PARALLAX_MAPPING    -> Enables parallax-occlusion mapping (Requires NORMAL_MAPPING to be enabled).
NORMALMAP_XYZ_H		-> Normal map contains vector data (xyz in RGB) and height-field data (h in Alpha-Channel). When this is defined, no height-map is used for parallax-mapping.
TESSELLATION        -> Enables height-field tessellation.
HAS_SPECULAR_MAP    -> Specifies that a specular is used.

*/

/*
 * ======= Vertex shader: =======
 */

/* === Structures === */

struct SVertexInput
{
    float3 Position : POSITION;
    float3 Normal   : NORMAL;
    float2 TexCoord : TEXCOORD0;
    #ifdef NORMAL_MAPPING
    float3 Tangent  : TEXCOORD1;
    float3 Binormal : TEXCOORD2;
    #endif
};

struct SVertexOutput
{
    float4 Position                 : SV_Position;
    float2 TexCoord                 : TEXCOORD0;
    float3 WorldPos                 : TEXCOORD1;
    float3 Normal                   : TEXCOORD2;
    #ifdef NORMAL_MAPPING
    float3 Tangent                  : TEXCOORD3;
    float3 Binormal                 : TEXCOORD4;
    #   ifdef PARALLAX_MAPPING
    float4 ViewVertexDirAndDepth    : TEXCOORD5;
    #   endif
    #endif
};


/* === Uniforms === */

cbuffer BufferMain : register(b0)
{
    float4x4 WorldViewProjectionMatrix;
    float4x4 WorldMatrix;

    #ifdef USE_TEXTURE_MATRIX
    float4x4 TextureMatrix;
    #endif

    float3 ViewPosition;    //!< Global camera position.
};


/* === Functions === */

SVertexOutput VertexMain(SVertexInput In)
{
    SVertexOutput Out = (SVertexOutput)0;
    
    /* Process vertex transformation for position and normal */
    float3x3 NormalMatrix = (float3x3)WorldMatrix;

    Out.Position    = mul(WorldViewProjectionMatrix, float4(In.Position, 1.0));
    Out.WorldPos    = mul(WorldMatrix, float4(In.Position, 1.0)).xyz;
    Out.Normal      = mul(NormalMatrix, In.Normal);

    #ifdef NORMAL_MAPPING
    
    /* Process transformation for tangent space */
    Out.Tangent     = mul(NormalMatrix, In.Tangent);
    Out.Binormal    = mul(NormalMatrix, In.Binormal);

    #   ifdef PARALLAX_MAPPING
    NormalMatrix = float3x3(
        normalize(Out.Tangent),
        normalize(Out.Binormal),
        normalize(Out.Normal)
    );

    Out.ViewVertexDirAndDepth.xyz   = ViewPosition - Out.WorldPos.xyz;
    Out.ViewVertexDirAndDepth.xyz   = mul(NormalMatrix, Out.ViewVertexDirAndDepth.xyz);
    Out.ViewVertexDirAndDepth.y     = -Out.ViewVertexDirAndDepth.y;

    Out.ViewVertexDirAndDepth.w     = Out.Position.z;
    #   endif

    #endif

    #ifdef USE_TEXTURE_MATRIX
    Out.TexCoord = (float2)mul(TextureMatrix, float4(In.TexCoord, 0.0, 1.0));
    #else
    Out.TexCoord = In.TexCoord;
    #endif
    
    return Out;
}


/*
 * ======= Pixel shader: =======
 */

/* === Structures === */

struct SPixelOutput
{
    float4 DiffuseAndSpecular   : SV_Target0;
    float4 NormalAndDepth       : SV_Target1;
};


/* === Uniforms === */

SAMPLER2D(DiffuseMap, 0);

#ifdef HAS_SPECULAR_MAP
SAMPLER2D(SpecularMap, 1);
#endif

#ifdef NORMAL_MAPPING

// Contains only height-field information (in the alpha channel) when PARALLAX_MAPPING is enabled
#   ifdef HAS_SPECULAR_MAP
SAMPLER2D(NormalMap, 2);
#       if defined(PARALLAX_MAPPING) && !defined(NORMALMAP_XYZ_H)
SAMPLER2D(HeightMap, 3);
#       endif
#   else
SAMPLER2D(NormalMap, 1);
#       if defined(PARALLAX_MAPPING) && !defined(NORMALMAP_XYZ_H)
SAMPLER2D(HeightMap, 2);
#       endif
#   endif

#endif

cbuffer BufferRelief : register(b1)
{
    float SpecularFactor    : packoffset(c0.x);
    float HeightMapScale    : packoffset(c0.y);
    float ParallaxViewRange : packoffset(c0.z);
    int EnablePOM           : packoffset(c1.x);
    int MinSamplesPOM       : packoffset(c1.y);
    int MaxSamplesPOM       : packoffset(c1.z);
};


/* === Functions === */

SPixelOutput PixelMain(SVertexOutput In)
{
    SPixelOutput Out = (SPixelOutput)0;
    
    float4 DiffuseAndSpecular = (float4)0.0;
    float4 NormalAndDepth = (float4)0.0;

    float2 TexCoord                 = In.TexCoord;
    float3 WorldPos                 = In.WorldPos;
    float3 Normal                   = In.Normal;

    #ifdef NORMAL_MAPPING
    
    float3 Tangent                  = In.Tangent;
    float3 Binormal                 = In.Binormal;

    #   ifdef PARALLAX_MAPPING
    float4 ViewVertexDirAndDepth    = In.ViewVertexDirAndDepth;
    #   endif

    #endif
