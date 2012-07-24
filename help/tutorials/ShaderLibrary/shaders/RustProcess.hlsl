// ####################################################
// # Direct3D Shader "RustProcess" (Shader Model 2.0) #
// ####################################################
//
// Copyright (c) 2010 - Lukas Hermanns
//

/*
 * Global members
 */

//! World matrix (Object space).
float4x4 WorldMatrix;

//! WorldViewProjection matrix used to transform the vertex coordinate.
float4x4 WorldViewProjectionMatrix;

//! Light direction vector for simple vertex lighting.
float3 LightDirection;

//! First color map.
sampler2D ColorMapA;

//! Second color map.
sampler2D ColorMapB;

//! Transition factor multiplication map.
sampler2D FactorMap;

//! Transition factor to morph between the two color maps.
float TransitionFactor;


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
	float2 TexCoord	: TEXCOORD0;
};

struct PixelInput
{
	float4 Color	: COLOR;
	float2 TexCoord	: TEXCOORD0;
};


/*
 * Vertex shader
 */

VertexOutput VertexMain(VertexInput Input)
{
	VertexOutput Output = (VertexOutput)0;
	
	//! Transform the vertex coordinate.
	Output.Position = mul(float4(Input.Position, 1.0), WorldViewProjectionMatrix);
	
	//! Transform normal vector.
	float3 Normal = normalize(mul(Input.Normal, (float3x3)WorldMatrix));
	
	//! Compute simple directional lighting.
	float LightIntensity = max(0.2, -dot(Normal, LightDirection));
	
	Output.Color.rgb	= Input.Color.rgb * LightIntensity;
	Output.Color.a		= Input.Color.a;
	
	//! Pass only the first texture coordinate.
	Output.TexCoord = Input.TexCoord.xy;
	
	return Output;
}


/*
 * Pixel shader
 */

float4 PixelMain(PixelInput Input) : COLOR
{
	float4 Color = 1.0;
	
	//! Get the pixel color of the two color maps.
	float3 ColorA = tex2D(ColorMapA, Input.TexCoord.xy).rgb;
	float3 ColorB = tex2D(ColorMapB, Input.TexCoord.xy).rgb;
	
	//! Get the factor color.
	float3 Factor = tex2D(FactorMap, Input.TexCoord.xy).rgb;
	
	//! Multiply the factor color with the transition factor.
	Factor *= TransitionFactor;
	
	//! Clamp the factor colors.
	Factor.r = clamp(Factor.r, 0.0, 1.0);
	Factor.g = clamp(Factor.g, 0.0, 1.0);
	Factor.b = clamp(Factor.b, 0.0, 1.0);
	
	//! Set the final pixel color.
	Color.rgb = Input.Color.rgb * (ColorA * (1.0 - Factor) + ColorB * Factor);
	
	//! Pass the transparency.
	Color.a = Input.Color.a;
	
	return Color;
}

