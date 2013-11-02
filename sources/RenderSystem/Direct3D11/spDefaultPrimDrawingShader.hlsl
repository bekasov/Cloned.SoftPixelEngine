/*
 * D3D11 default primitive drawing shader file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#define MAX_NUM_VERTICES 8

struct SVertex
{
	float4 Position;
	float4 Color;
};

cbuffer BufferVS : register(b0)
{
    float4x4 WVPMatrix;	//!< World-view-projection matrix.
	SVertex Vertices[MAX_NUM_VERTICES];
};

struct SVertexOutput
{
    float4 Position	: SV_Position;
    float4 Color	: COLOR0;
};

SVertexOutput VertexMain(uint Id : SV_VertexID)
{
    SVertexOutput Out = (SVertexOutput)0;
	
    // Process vertex coordinate
	SVertex Vert = Vertices[Id];
	
	Out.Position	= mul(WVPMatrix, Vert.Position);
    Out.Color		= Vert.Color;
	
    return Out;
}

float4 PixelMain(SVertexOutput In) : SV_Target0
{
    return In.Color;
}

