/*
 * D3D11 default drawing shader file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include <softpixelengine>

/* === Uniforms === */

DeclSampler2D(ColorMap, 0);

cbuffer BufferVS : register(b0)
{
	float4x4 ProjectionMatrix;  //!< Projection matrix.
	float4x4 WorldMatrix;       //!< Image transformation matrix.
	float4 TextureTransform;    //!< Texture offset (XY), Texture scaling (ZW).
	float4 Position;            //!< Image origin (XY), Image offset (ZW).
}

cbuffer BufferPS : register(b0)
{
	float4 Color;
	int UseTexture;
}

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
};

/* === Functions === */

SVertexOutput VertexMain(SVertexInput In)
{
    SVertexOutput Out = (SVertexOutput)0;
	
    // Process vertex coordinate
    float2 Coord = Position.xy + mul(WorldMatrix, float4(Position.zw + In.Position, 0.0, 1.0)).xy;

    Out.Position = mul(ProjectionMatrix, float4(Coord.x, Coord.y, 0.0, 1.0));

    // Process texture coordinate
    Out.TexCoord = TextureTransform.xy + In.TexCoord * TextureTransform.zw;
	
    return Out;
}

float4 PixelMain(SVertexOutput In) : SV_Target0
{
    return UseTexture != 0 ? tex2D(ColorMap, In.TexCoord) * Color : Color;
}

