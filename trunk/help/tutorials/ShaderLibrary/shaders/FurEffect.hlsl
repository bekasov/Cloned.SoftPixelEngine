// ##################################################
// # Direct3D Shader "FurEffect" (Shader Model 2.0) #
// ##################################################
//
// Copyright (c) 2010 - Lukas Hermanns
//

/*
 * Global members
 */

//! Base texture.
sampler2D Texture;

//! World matrix used to transform only the vertex normal.
float4x4 WorldMatrix;

//! WorldViewProjection matrix used to transform the vertex coordinate.
float4x4 WorldViewProjectionMatrix;

//! Light direction vector for simple vertex lighting.
float3 LightDirection;

//! Height of the last hair layer.
float MaxLayerHeight;

//! Interpolation value in the range [0.0 .. 1.0].
float LayerInterpolation;

//! Texture coordinate translation.
float3 TexCoordTranslation;


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

struct VertexPixelExchange
{
	float4 Position	: POSITION;
	float4 Color	: COLOR;
	float2 TexCoord	: TEXCOORD0;
};


/*
 * Vertex shader
 */

VertexPixelExchange VertexMain(VertexInput Input)
{
	VertexPixelExchange Output = (VertexPixelExchange)0;
	
	//! Transform the vertex coordiante by moving along the vertex normal and multiplying it with the w-v-p matrix.
	Output.Position.xyz	= Input.Position.xyz + Input.Normal.xyz * LayerInterpolation * MaxLayerHeight;
	Output.Position.w	= 1.0;
	
	Output.Position		= mul(Output.Position, WorldViewProjectionMatrix);
	
	//! Compute the final vertex normal by multiplying with the normal matrix (generated out of the world-matrix) and normalize the vector.
	float3 Normal = normalize(mul(Input.Normal, (float3x3)WorldMatrix));
	
	//! Compute simple directional lighting (for mor detail see "SimpleVertexManipulation" shader).
	float LightIntensity = max(0.2, -dot(Normal, LightDirection));
	
	Output.Color.rgb	= Input.Color.rgb * LightIntensity;
	Output.Color.a		= Input.Color.a * (1.0 - LayerInterpolation);
	
	//! Pass the whole texture coordinate without manipulation.
	Output.TexCoord = Input.TexCoord.xy + (TexCoordTranslation.xy * LayerInterpolation);
	
	//! Return the output data.
	return Output;
}


/*
 * Pixel shader
 */

float4 PixelMain(VertexPixelExchange Input) : COLOR
{
	//! Just pass the texture color multiplied with the vertex color.
	return tex2D(Texture, Input.TexCoord) * Input.Color;
}

