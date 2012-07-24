// ########################################################
// # Direct3D Shader "WaterSimulation" (Shader Model 3.0) #
// ########################################################
//
// Copyright (c) 2010 - Lukas Hermanns
//

/*
 * Global members
 */

//! WorldViewProjection matrix used to transform the vertex coordinate.
float4x4 WorldViewProjectionMatrix;

//! Refraction map where everything is visible which is under the water plane (RenderTarget texture filled by deferred rendering).
sampler2D RefractionMap;

//! Reflection map where everything is visible mirrored which is over the water plane (RenderTarget texture filled by deferred rendering).
sampler2D ReflectionMap;

//! Water surface map (Or rather color map).
sampler2D WaterSurfaceMap;

//! Water surface normal map.
sampler2D NormalMap;

//! Heigth field map (gray-scaled texture).
sampler2D HeightMap;

//! Screen resolution width.
int ScreenWidth;

//! Screen resolution height.
int ScreenHeight;

//! Enables or disable under water status.
bool IsUnderWater;

//! Global water plane height.
float WaterPlaneHeight;

//! Local object height.
float ObjectHeight;


/*
 * Structures
 */

struct VertexInput
{
	float3 Position		: POSITION;
	float4 Color		: COLOR;
	float3 TexCoord0	: TEXCOORD0;
	float3 TexCoord1	: TEXCOORD1;
};

struct VertexOutput
{
	float4 Position			: POSITION;		//!< Projected vertex coordinate (Into screen coordaintes).
	float4 Color			: COLOR;
	float2 TexCoord0		: TEXCOORD0;
	float2 TexCoord1		: TEXCOORD1;
	float4 VertexPosition	: TEXCOORD2;	//!< Additional vertex position stored in a texture coordinate (Used like a varying in OpenGL).
	float3 LocalPos			: TEXCOORD3;
};

struct PixelInput
{
	float4 Position			: VPOS;			//!< Pixel screen coordinate (In screen space). This is only available in "Shader Model 3.x".
	float4 VertexPosition	: TEXCOORD2;
	float4 Color			: COLOR;
	float2 TexCoord0		: TEXCOORD0;
	float2 TexCoord1		: TEXCOORD1;
	float3 LocalPos			: TEXCOORD3;
};


/*
 * Pixel shader
 */

VertexOutput VertexMain(VertexInput Input)
{
	VertexOutput Output = (VertexOutput)0;
	
	//! Just transform the coordinate with the world-view-projection matrix.
	Output.Position = mul(float4(Input.Position, 1.0), WorldViewProjectionMatrix);
	
	//! Store the vertex coordinate additionally in a second vector (This coordinate will not be affected by the viewport matrix).
	Output.VertexPosition = Output.Position;
	
	//! Pass the vertex color.
	Output.Color = Input.Color;
	
	//! Pass the texture coordinates.
	Output.TexCoord0 = Input.TexCoord0.xy;
	Output.TexCoord1 = Input.TexCoord1.xy;
	
	//! Pass the vertex coordinate in object space.
	Output.LocalPos	= Input.Position.xyz;
	
	return Output;
}


/*
 * Pixel shader
 */

float4 PixelMain(PixelInput Input) : COLOR
{
	//! Get the normal vector from the normal map.
	float3 PixelNormal	= tex2D(NormalMap, Input.TexCoord1).rgb;
	
	//! Convert the color to a normal vector.
	PixelNormal			= (PixelNormal - 0.5) * 2.0;
	
	//! Store the screen resolution in floating-points.
	float Width		= (float)ScreenWidth;
	float Height	= (float)ScreenHeight;
	
	//! Calculate the texture coordinate for the refraction- and reflection map.
	float2 TexCoord	= float2(Input.Position.x / Width, (Input.Position.y + 2.0) / Height);
	
	//! Calculate displacement factor to avoid ugly effect at water border.
	float DisplacementFactor	= -WaterPlaneHeight - tex2D(HeightMap, float2(Input.LocalPos.x + 0.5, 0.5 - Input.LocalPos.z)).r * ObjectHeight;
	DisplacementFactor			= clamp(DisplacementFactor, 0.0, 1.0);
	
	//! Perform displacement mapping by manipulating the texture coordinate with the normal vector.
	TexCoord.xy		+= (PixelNormal.xy / 64.0) * DisplacementFactor;
	
	//! Clamp the texture coordinate to avoid ugly areas at the screen border.
	TexCoord.x		= clamp(TexCoord.x, 1.0 / Width, (Width - 1.0) / Width);
	TexCoord.y		= clamp(TexCoord.y, 1.0 / Height, (Height - 1.0) / Height);
	
	//! Get the refraction color.
	float4 RefractionColor = tex2D(RefractionMap, TexCoord);
	float4 ReflectionColor = 0.0;
	
	//! Get the reflection color (If needed).
	if (!IsUnderWater)
		ReflectionColor = tex2D(ReflectionMap, TexCoord);
	
	/**
	Calculate ratio between refraction and reflection. We also clamp the value to the
	range [0.1 .. 0.9] so that we will always see both refractions and reflections.
	*/
	float Ratio = clamp(( Input.VertexPosition.z - 1.0 ) / 10.0, 0.1, 0.9);
	
	//! Get the water surface color.
	float4 WaterSurfaceColor = tex2D(WaterSurfaceMap, Input.TexCoord0);
	
	//! Temporary variables.
	float4 Color = 0.0;
	
	/**
	Set the final pixel color. If the view camera is under the water plane we do not
	want to draw the reflections which are over the water plane.
	*/
	if (IsUnderWater)
		Color = WaterSurfaceColor * RefractionColor;
	else
	{
		//! Interpolate between the refraction- and reflection color using the ratio value which is in the range [0.1 .. 0.9].
		Color = WaterSurfaceColor * ( RefractionColor * ( 1.0 - Ratio ) + ReflectionColor * Ratio );
	}
	
	return Color;
}

