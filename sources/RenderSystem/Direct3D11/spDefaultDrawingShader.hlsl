/*
 * D3D11 default drawing shader file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

Texture2D Texture : register(t0);
SamplerState Sampler : register(s0);

cbuffer BufferMain : register(b0)
{
    float4x4 ProjectionMatrix;
    float4 Color;
    int UseTexture;
};

cbuffer BufferMapping : register(b1)
{
    float2 Position;
    float2 TexPosition;
    float4x4 WorldMatrix;
    float4x4 TextureMatrix;
};

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

SVertexOutput VertexMain(SVertexInput In)
{
    SVertexOutput Out = (SVertexOutput)0;
	
    // Process vertex coordinate
    float2 Coord = Position + mul((float2x2)WorldMatrix, In.Position);

    Out.Position = mul(ProjectionMatrix, float4(Coord.x, Coord.y, 0.0, 1.0));

    // Process texture coordinate
    Out.TexCoord = TexPosition + mul((float2x2)TextureMatrix, In.TexCoord);
	
    return Out;
}

float4 PixelMain(SVertexOutput In) : SV_Target0
{
    return UseTexture != 0 ? Texture.Sample(Sampler, In.TexCoord) * Color : Color;
}

