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
    float4x4 ViewMatrix;
};

cbuffer BufferPS : register(b1)
{
	float4 Diffuse	: packoffset(c0);
	int UseTexture	: packoffset(c1.x);
	int Blending	: packoffset(c1.y);
};

struct SVertexInput
{
	float4 Position	: POSITION;
	float4 Color	: COLOR;
};

struct SGeometryOutput
{
    float4 Position : SV_Position;
	float2 TexCoord : TEXCOORD0;
};

SVertexInput VertexMain(SVertexInput In)
{
	return In;
}

[maxvertexcount(4)]
void GeometryMain(
    point SVertexInput In[1],
    inout TriangleStream<SGeometryOutput> OutStream)
{
	SGeometryOutput Out = (SGeometryOutput)0;
	
	// Setup transformation
	float Scale = In[0].Position.w;
	
	float Position = mul(ViewMatrix, float4(In[0].Position, 1.0)).xyz;
	
	float4x4 Transform =
	{
		Scale, 0.0, 0.0, Position.x,
		0.0, Scale, 0.0, Position.y,
		0.0, 0.0, Scale, Position.z,
		0.0, 0.0, 0.0, 1.0
	};
	
	// Setup vertex 0
	Out.Position = mul(Transform, float4(-1.0,  1.0, 0.0, 1.0));
	Out.TexCoord = float2(0.0, 0.0);
	OutStream.Append(Out);
	
	// Setup vertex 1
	Out.Position = mul(Transform, float4( 1.0,  1.0, 0.0, 1.0));
	Out.TexCoord = float2(1.0, 0.0);
	OutStream.Append(Out);
	
	// Setup vertex 2
	Out.Position = mul(Transform, float4( 1.0, -1.0, 0.0, 1.0));
	Out.TexCoord = float2(1.0, 1.0);
	OutStream.Append(Out);
	
	// Setup vertex 3
	Out.Position = mul(Transform, float4(-1.0, -1.0, 0.0, 1.0));
	Out.TexCoord = float2(0.0, 1.0);
	OutStream.Append(Out);
	
	OutStream.RestartStrip();
}

float4 PixelMain(SGeometryOutput In) : SV_Target0
{
	float4 Out = Diffuse;
	
    if (UseTexture != 0)
	{
		Out *= Tex.Sample(Sampler, In.TexCoord);
		if (Blending != 0)
			clip(Out.a - 0.5);
	}
	
	return Out;
}

