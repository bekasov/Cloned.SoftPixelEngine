/*
 * D3D11 default billboard shader file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

Texture2D Tex;
SamplerState Sampler;

cbuffer BufferGS : register(b0)
{
    float4x4 WVPMatrix;	//!< World-view-projection matrix.
};

cbuffer BufferPS : register(b1)
{
	float4 Color;
	int UseTexture;
};

struct SDummy
{
};

struct SGeometryOutput
{
    float4 Position : SV_Position;
	float2 TexCoord : TEXCOORD0;
};

SDummy VertexMain(SVertexInput In)
{
	return (SDummy)0;
}

[maxvertexcount(4)]
void GeometryMain(
    point SDummy Unused[1],
    inout TriangleStream<SGeometryOutput> OutStream)
{
	SGeometryOutput Out = (SGeometryOutput)0;
	
	// Setup vertex 0
	Out.Position = mul(WVPMatrix, float4(-1.0,  1.0, 0.0, 1.0));
	Out.TexCoord = float2(0.0, 0.0);
	OutStream.Append(Out);
	
	// Setup vertex 1
	Out.Position = mul(WVPMatrix, float4( 1.0,  1.0, 0.0, 1.0));
	Out.TexCoord = float2(1.0, 0.0);
	OutStream.Append(Out);
	
	// Setup vertex 2
	Out.Position = mul(WVPMatrix, float4( 1.0, -1.0, 0.0, 1.0));
	Out.TexCoord = float2(1.0, 1.0);
	OutStream.Append(Out);
	
	// Setup vertex 3
	Out.Position = mul(WVPMatrix, float4(-1.0, -1.0, 0.0, 1.0));
	Out.TexCoord = float2(0.0, 1.0);
	OutStream.Append(Out);
	
	OutStream.RestartStrip();
}

float4 PixelMain(SGeometryOutput In) : SV_Target0
{
    return UseTexture != 0 ? Color * Tex.Sample(Sampler, In.TexCoord) : Color;
}

