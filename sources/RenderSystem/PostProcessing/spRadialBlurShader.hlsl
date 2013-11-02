/*
 * Radial blur HLSL shader file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include <softpixelengine>

/* === Uniforms === */

DeclSampler2D(ColorMap, 0);

cbuffer BufferPS : register(b0)
{
	uint NumSamples	: packoffset(c0);
	float Scaling	: packoffset(c0.y);
	float2 Pad0		: packoffset(c0.z);
};

/* === Structures === */

struct SVertexOutput
{
    float4 Position : SV_Position;
    float2 TexCoord : TEXCOORD0;
};

/* === Vertex Shader === */

SFullscreenQuadVertexOutput VertexMain(uint Id : SV_VertexID)
{
	return FullscreenQuadVertexMain(Id);
}

/* === Pixel Shader === */

float2 TransformTexCoord(float2 TexCoord, float Scale)
{
	// Transform texture coordinate
	TexCoord -= (float2)0.5;
	TexCoord *= (float2)Scale;
	TexCoord += (float2)0.5;
	
	return TexCoord;
}

float4 PixelMain(SFullscreenQuadVertexOutput In) : SV_Target0
{
	float4 Color = (float4)0.0;
	
	if (NumSamples > 0)
	{
		float InvNumSamples = 1.0 / (float)NumSamples;
		
		// Compute average color
		for (uint i = 0; i < NumSamples; ++i)
		{
			Color += tex2D(
				ColorMap,
				TransformTexCoord(In.TexCoord, 1.0 + Scaling * (float)i * InvNumSamples)
			);
		}
		
		Color *= (float4)InvNumSamples;
	}
	
	return Color;
}
