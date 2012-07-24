// #####################################################
// # Direct3D Shader "DepthOfField" (Shader Model 3.0) #
// #####################################################
//
// Copyright (c) 2010 - Lukas Hermanns
//

/*
 * Global members
 */

//! Color map.
sampler2D ColorMap;

//! WorldViewProjection matrix used to transform the vertex coordinate.
float4x4 WorldViewProjectionMatrix;

//! Screen resolution width.
int ScreenWidth;

//! Screen resolution height.
int ScreenHeight;

//! Depth bias for modifying the begin of depth bluring.
float DepthBias;

//! Depth range for modifying the range of depth bluring.
float DepthRange;


/*
 * Structures
 */

struct VertexInput
{
	float3 Position	: POSITION;
};

struct VertexOutput
{
	float4 Position		: POSITION;
	float VertexDepth	: TEXCOORD0;
};

struct PixelInput
{
	float2 Position		: VPOS;
	float VertexDepth	: TEXCOORD0;
};


/*
 * Vertex shader
 */

VertexOutput VertexMain(VertexInput Input)
{
	VertexOutput Output = (VertexOutput)0;
	
	//! Pass the vertex coordinate.
	Output.Position = mul(float4(Input.Position, 1.0), WorldViewProjectionMatrix);
	
	//! Pass the vertex depth.
	Output.VertexDepth = Output.Position.z;
	
	return Output;
}


/*
 * Pixel shader
 */

float4 PixelMain(PixelInput Input) : COLOR
{
	float4 Color = 1.0;
	
	//! Convert the screen coordinate into a texture coordinate.
	float2 TexCoord;
	
	TexCoord.x = Input.Position.x / (float)ScreenWidth;
	TexCoord.y = Input.Position.y / (float)ScreenHeight;
	
	//! Get the default color.
	float3 ColorDefault = tex2D(ColorMap, TexCoord).rgb;
	
	//! Set the gaussian blur factor.
	float2 Factor = float2(2.0 / (float)ScreenWidth, 2.0 / (float)ScreenHeight);
	
	//! Expect a rectangular color area together to get a blured result.
	int Range = 4, i, j;
	float3 ColorBlur = 0.0;
	
	//! Pass each of the image colors used for bluring the pixel.
	for (i = -Range; i <= Range; ++i)
	{
		for (j = -Range; j <= Range; ++j)
		{
			//! Add a surrounding texel.
			ColorBlur += tex2D(
				ColorMap,
				float2(
					TexCoord.x + Factor.x * (float)i,
					TexCoord.y + Factor.y * (float)j
				)
			).rgb;
		}
	}
	
	//! Devide the color components with the count of added image colors.
	int Count = Range * 2 + 1;
	ColorBlur /= (float)Count*Count;
	
	//! Calculate the final blur factor.
	float BlurFactor = clamp(abs(Input.VertexDepth - DepthBias) / DepthRange, 0.0, 1.0);
	
	//! Set the final pixel color by interpolating between normal and blured image.
	Color.rgb	= ColorDefault * (1.0 - BlurFactor) + BlurFactor * ColorBlur;
	Color.a		= 1.0;
	
	return Color;
}

