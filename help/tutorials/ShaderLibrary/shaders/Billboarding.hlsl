// #####################################################
// # Direct3D Shader "Billboarding" (Shader Model 2.0) #
// #####################################################
//
// Copyright (c) 2010 - Lukas Hermanns
//

//! WorldView matrix used to get the un-projected object position.
float4x4 WorldViewMatrix;

//! Matrix to project the vertex coordinate into the final window coordinate.
float4x4 ProjectionMatrix;

//! Material diffuse color.
float4 DiffuseColor;


/*
 * Structures
 */

struct VertexInput
{
	float3 Position	: POSITION;
	float3 Normal	: NORMAL;
	float4 Color	: COLOR;
	float3 TexCoord	: TEXCOORD0;
};

struct VertexOutput
{
	float4 Position	: POSITION;
	float4 Color	: COLOR;
	float3 TexCoord	: TEXCOORD0;
};


/*
 * Vertex shader
 */

VertexOutput VertexMain(VertexInput Input)
{
	VertexOutput Output = (VertexOutput)0;
	
	//! Create the final world-view matrix without rotation but only position.
	float4x4 FinalWorldViewMatrix = float4x4(
		1.0, 0.0, 0.0, 0.0,
		0.0, 1.0, 0.0, 0.0,
		0.0, 0.0, 1.0, 0.0,
		WorldViewMatrix[3].x, WorldViewMatrix[3].y, WorldViewMatrix[3].z, 1.0
	);
	
	//! Transform the final vertex coordinate.
	Output.Position = mul(mul(float4(Input.Position, 1.0), FinalWorldViewMatrix), ProjectionMatrix);
	
	//! Just set the material diffuse color. If you want to apply lighting look in the "SimpleVertexManipulation" shader.
	Output.Color = Input.Color * DiffuseColor;
	
	/**
	For billboards we normally only need one texture. If you want to use more add the
	other texture coordinate transformations which is shown in the "SimpleVertexManipulation" shader.
	*/
	Output.TexCoord = Input.TexCoord;
	
	return Output;
}

