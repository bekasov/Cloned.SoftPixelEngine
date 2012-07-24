// ###################################################
// # Direct3D Shader "CelShading" (Shader Model 2.0) #
// ###################################################
//
// Copyright (c) 2010 - Lukas Hermanns
//


/*
 * Global members
 */

//! World matrix used to transform only the vertex normal.
float4x4 WorldMatrix;

//! WorldViewProjection matrix used to transform the vertex coordinate.
float4x4 WorldViewProjectionMatrix;

//! Light direction vector for simple vertex lighting.
float3 LightDirection;


/*
 * Structures
 */

struct VertexInput
{
	float3 Position		: POSITION;
	float3 Normal		: NORMAL;
	float4 Color		: COLOR;
	float3 TexCoord0	: TEXCOORD0;
	float3 TexCoord1	: TEXCOORD1;
};

struct VertexOutput
{
	float4 Position		: POSITION;
	float4 Color		: COLOR;
	float TexCoord0		: TEXCOORD0;
	float2 TexCoord1	: TEXCOORD1;
};


/*
 * Vertex shader
 */

VertexOutput VertexMain(VertexInput Input)
{
	VertexOutput Output = (VertexOutput)0;
	
	//! Just transform the coordinate with the world-view-projection matrix.
	Output.Position = mul(float4(Input.Position, 1.0), WorldViewProjectionMatrix);
	
	//! Pass the normalized vertex normal vector.
	float3 Normal = normalize(mul(Input.Normal, (float3x3)WorldMatrix));
	
	//! Compute simple directional lighting.
	float LightIntensity = -dot(Normal, LightDirection);
	
	/**
	Set the first texture coordinate as the light intensity factor.
	The 1D texture will hold the cel-shading light values.
	*/
	Output.TexCoord0 = clamp(LightIntensity, 0.0, 1.0);
	
	//! Pass the second texture coordinate in the normal way.
	Output.TexCoord1 = Input.TexCoord1;
	
	//! Set the vertex color.
	Output.Color = Input.Color;
	
	return Output;
}

