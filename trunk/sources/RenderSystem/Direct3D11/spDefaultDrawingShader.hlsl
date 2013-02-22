/*
 * D3D11 default drawing shader file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

Texture2D Texture : register(t0);
SamplerState Sampler : register(s0);

cbuffer BufferBasic : register(b0)
{
    float4x4 ProjectionMatrix;
    float4 ImageRect;
};

struct VertexInput
{
    float4 Position : POSITION;
    float4 Color    : COLOR;
    float2 TexCoord : TEXCOORD0;
    uint Index      : SV_VertexID;
};

struct VertexPixelExchange
{
    float4 Position : SV_Position;
    float4 Color    : COLOR;
    float2 TexCoord : TEXCOORD0;
};

VertexPixelExchange VertexMain(VertexInput Input)
{
    VertexPixelExchange Output = (VertexPixelExchange)0;
	
    switch (Input.Index)
    {
        case 0: Output.Position = float4(ImageRect.x, ImageRect.y, 0.0, 1.0); break;
        case 1: Output.Position = float4(ImageRect.z, ImageRect.y, 0.0, 1.0); break;
        case 2: Output.Position = float4(ImageRect.z, ImageRect.w, 0.0, 1.0); break;
        case 3: Output.Position = float4(ImageRect.x, ImageRect.y, 0.0, 1.0); break;
        case 4: Output.Position = float4(ImageRect.z, ImageRect.w, 0.0, 1.0); break;
        case 5: Output.Position = float4(ImageRect.x, ImageRect.w, 0.0, 1.0); break;
    }
    
    Output.Position = mul(ProjectionMatrix, Output.Position);
    Output.Color    = Input.Color;
    Output.TexCoord = Input.TexCoord;
	
    return Output;
}

float4 PixelMain(VertexPixelExchange Input) : SV_Target
{
    return Texture.Sample(Sampler, Input.TexCoord) * Input.Color;
}

