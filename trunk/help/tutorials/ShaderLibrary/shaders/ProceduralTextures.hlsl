// ###########################################################
// # Direct3D Shader "ProceduralTextures" (Shader Model 3.0) #
// ###########################################################
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

//! Light position for per-pixel lighting.
float3 LightPosition;

//! Mode for procedure texture demonstration.
uint DemoMode;

//! Material ambient color.
float4 AmbientColor;

//! Material diffuse color.
float4 DiffuseColor;

//! Material specular color (For shininess).
float4 SpecularColor;

//! Material shininess factor.
float Shininess;


/*
 * Macros
 */

#define DEMO_SIMPLE		0	//!< Simple diffuse colored material.
#define DEMO_CHESSBOARD	1	//!< Quads ordered in form of a chess board.
#define DEMO_BRICKS		2	//!< Bricks (or rather walls).
#define DEMO_CIRCLES	3	//!< Colored circles.


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
	float3 Normal	: NORMAL;
	float4 Color	: COLOR;
	float2 TexCoord	: TEXCOORD0;
	float3 WorldPos	: TEXCOORD1;
};


/*
 * Functions
 */

//! Mapping function used by the most texture generation demonstrations.
float2 GetMapping(float2 TexCoord, float Factor)
{
	//! Cuts off the integer value (e.g. 12.34 to 0.34).
	return float2(
		frac(TexCoord.x * Factor),
		frac(TexCoord.y * Factor)
	);
}

//! Returns true if the point is inside the specified circle.
bool IsInside(float2 Point, float2 CircleCenter, float CircleRadius)
{
	/**
	This caclulation has the same result like "return distance(Point, CircleCenter) < CircleRadius;" but
	it is faster because for distance comparisions we don't need to call the sqrt (square root) function.
	*/
	float2 DistSq = float2(Point.x - CircleCenter.x, Point.y - CircleCenter.y);
	return (DistSq.x*DistSq.x + DistSq.y*DistSq.y) < CircleRadius*CircleRadius;
}


/*
 * Vertex shader
 */

VertexPixelExchange VertexMain(VertexInput Input)
{
	VertexPixelExchange Output = (VertexPixelExchange)0;
	
	//! Use standard transformation.
	Output.Position = mul(float4(Input.Position, 1.0), WorldViewProjectionMatrix);
	
	//! Compute the final vertex normal.
	Output.Normal = normalize(mul(Input.Normal, (float3x3)WorldMatrix));
	
	//! Compute the vertex position (Object space).
	Output.WorldPos = mul(float4(Input.Position, 1.0), WorldMatrix).xyz;
	
	//! Pass the vertex color.
	Output.Color = Input.Color;
	
	//! In this example we only need one texture coordinate.
	Output.TexCoord = Input.TexCoord.xy;
	
	return Output;
}


/*
 * Pixel shader
 */

float4 PixelMain(VertexPixelExchange Input) : COLOR
{
	float4 Color = 1.0;
	
	//! Switch between the demonstrations.
	switch (DemoMode)
	{
		case DEMO_SIMPLE:
		{
			//! Do nothing, material color and lighitng will be applied at the end.
		}
		break;
		
		case DEMO_CHESSBOARD:
		{
			float2 Mapping = GetMapping(Input.TexCoord, 5.0);
			
			if ( ( Mapping.x > 0.5 && Mapping.y > 0.5 ) || ( Mapping.x < 0.5 && Mapping.y < 0.5 ) )
				Color.rgb = float3(0.0, 0.0, 0.0);
		}
		break;
		
		case DEMO_BRICKS:
		{
			float2 Mapping = GetMapping(Input.TexCoord, 5.0);
			
			if ( ( Mapping.x < 0.45 && Mapping.y < 0.4 ) ||
				 ( Mapping.x > 0.55 && Mapping.y < 0.4 ) ||
				 ( Mapping.x < 0.9 && Mapping.y > 0.5 && Mapping.y < 0.9 ) )
			{
				Color.rgb = float3(0.7, 0.35, 0.1);
			}
			else
				Color.rgb = 0.8;
		}
		break;
		
		case DEMO_CIRCLES:
		{
			float2 Mapping = GetMapping(Input.TexCoord, 2.0);
			
			if ( IsInside(float2(0.1, 0.1), Mapping, 0.05 ) ||
				 IsInside(float2(0.6, 0.2), Mapping, 0.1  ) ||
				 IsInside(float2(0.9, 0.1), Mapping, 0.05 ) ||
				 IsInside(float2(0.1, 0.3), Mapping, 0.05 ) ||
				 IsInside(float2(0.3, 0.3), Mapping, 0.05 ) ||
				 IsInside(float2(0.9, 0.3), Mapping, 0.05 ) )
			{
				Color.rgb = float3(0.0, 0.0, 1.0);
			}
			else if (IsInside(float2(0.3, 0.7), Mapping, 0.2))
				Color.rgb = float3(0.0, 1.0, 0.0);
			else if (IsInside(float2(0.3, 0.1), Mapping, 0.05))
				Color.rgb = float3(1.0, 0.0, 0.0);
		}
		break;
	}
	
	//! Get the light direction vector.
	float3 LightDirection = normalize(Input.WorldPos - LightPosition);
	
	/**
	Compute simple directional lighting by calculating the dot-product of the final
	transformed vertex normal and the light direction vector and clamp the light intensity
	to the range of [0.0 .. 1.0].
	In this case we normalize the normal vector again to have correct per-pixel lighting.
	We also set the minimal value to 0.0 because we use an ambient color.
	*/
	float LightIntensity = max(0.0, -dot(normalize(Input.Normal), LightDirection));
	
	/**
	Apply per-pixel lighting using the material's ambient, diffuse and specular color.
	Compute the final color by multiplying the material diffuse color with the light intensity
	and adding the specular color with the specified shininess factor. The minimal value is the material ambient color.
	*/
	Color.rgb = (
		Color.rgb * (
			AmbientColor.rgb +
			DiffuseColor.rgb * Input.Color.rgb * LightIntensity
		) +
		SpecularColor.rgb * pow(LightIntensity, Shininess)
	);
	
	//! Set the color transparency.
	//Color.a = DiffuseColor.a * Input.Color.a;
	
	return Color;
}

