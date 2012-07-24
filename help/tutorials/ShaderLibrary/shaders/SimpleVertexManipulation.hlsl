// #################################################################
// # Direct3D Shader "SimpleVertexManipulation" (Shader Model 2.0) #
// #################################################################
//
// Copyright (c) 2010 - Lukas Hermanns
//

/*
 * Global members (configured in the C++ program)
 */

//! World matrix used to transform only the vertex normal.
float4x4 WorldMatrix;

//! WorldViewProjection matrix used to transform the vertex coordinate.
float4x4 WorldViewProjectionMatrix;

//! Light direction vector for simple vertex lighting.
float3 LightDirection;

//! Mode for vertex manipulation demonstration.
uint DemoMode;

//! Transformation for the individual demonstration.
float TransformFactor;

//! Mesh bounding box heights.
float BoundBoxMinHeight;
float BoundBoxMaxHeight;


/*
 * Macros
 */

#define DEMO_NORMAL		0 //!< Normal vertex transformation.
#define DEMO_SPHERIFY	1 //!< Spherifies the mesh.
#define DEMO_TWIST		2 //!< Twists the mesh (turning on Y axis).
#define DEMO_BLOAT		3 //!< Bloats the mesh (morphing animation along the vertex normal).


/*
 * Structures
 */

/**
Vertex input structure. All of the following members are stored for each vertex in the vertex buffer:
 - Position: Vertex coordinate.
 - Normal: Vertex normal (for lighting computations).
 - Color: Vertex color.
 - TexCoord0 - 7: Vertex texture coordinates (are 3 dimensional because 3D textures can also be used).
*/
struct VertexInput
{
	float3 Position		: POSITION;
	float3 Normal		: NORMAL;
	float4 Color		: COLOR;
	float3 TexCoord0	: TEXCOORD0;
	float3 TexCoord1	: TEXCOORD1;
	float3 TexCoord2	: TEXCOORD2;
	float3 TexCoord3	: TEXCOORD3;
	float3 TexCoord4	: TEXCOORD4;
	float3 TexCoord5	: TEXCOORD5;
	float3 TexCoord6	: TEXCOORD6;
	float3 TexCoord7	: TEXCOORD7;
};

/**
Vertex output structure. All of the following members will be interpolated for each pixel:
 - Position: Final vertex position in screen space.
 - Color: Final vertex color.
 - Normal: Final vertex normal.
 - TexCoord0 - 7: Final vertex texture coordinates.
*/
struct VertexOutput
{
	float4 Position		: POSITION;
	float4 Color		: COLOR;
	float3 TexCoord0	: TEXCOORD0;
	float3 TexCoord1	: TEXCOORD1;
	float3 TexCoord2	: TEXCOORD2;
	float3 TexCoord3	: TEXCOORD3;
	float3 TexCoord4	: TEXCOORD4;
	float3 TexCoord5	: TEXCOORD5;
	float3 TexCoord6	: TEXCOORD6;
	float3 TexCoord7	: TEXCOORD7;
};


/*
 * Vertex shader
 */

VertexOutput VertexMain(VertexInput Input)
{
	VertexOutput Output = (VertexOutput)0;
	
	//! Temporary variable.
	float3 Normal = Input.Normal;
	
	//! Switch between the demonstrations.
	switch (DemoMode)
	{
		case DEMO_NORMAL:
		{
			//! Just transform the coordinate with the world-view-projection matrix.
			Output.Position = mul(float4(Input.Position, 1.0), WorldViewProjectionMatrix);
			
			//! Compute the final vertex normal by multiplying with the normal matrix (generated out of the world-matrix) and normalize the vector.
			Normal = normalize( mul(Input.Normal, (float3x3)WorldMatrix) );
		}
		break;
		
		case DEMO_SPHERIFY:
		{
			//! Morph between the actual coordinate and the normalized coordinate.
			Output.Position.xyz	= Input.Position * (1.0 - TransformFactor) + normalize(Input.Position) * TransformFactor;
			Output.Position.w	= 1.0;
			
			//! Transform the coordinate with the w-v-p matrix.
			Output.Position = mul(Output.Position, WorldViewProjectionMatrix);
			
			//! Compute the final vertex normal by multiplying with the normal matrix and interpolating between the spherification.
			Normal = normalize(
				mul((Input.Normal * (1.0 - TransformFactor) + normalize(Input.Position) * TransformFactor), (float3x3)WorldMatrix)
			);
		}
		break;
		
		case DEMO_TWIST:
		{
			//! Compute the rotation angle for the twist performance.
			float Angle =
				TransformFactor																			//!< Our rotation angle (in degrees) set by the C++ program.
				* ( (Input.Position.y - BoundBoxMinHeight) / (BoundBoxMaxHeight - BoundBoxMinHeight) )	//!< Height factor for the individual vertex coordinate.
				* 3.141592 / 180.0;																		//!< Convert the angle from degrees to radians.
			
			//! Calculate the sine and cosine for our rotation.
			float s = sin(Angle);
			float c = cos(Angle);
			
			//! Generate the rotation matrix (on Y axis).
			float4x4 RotationMatrix = float4x4(
				  c, 0.0,   s, 0.0,
				0.0, 1.0, 0.0, 0.0,
				 -s, 0.0,   c, 0.0,
				0.0, 0.0, 0.0, 1.0
			);
			
			//! Twist the coordinate using the rotation matrix.
			Output.Position = mul(RotationMatrix, float4(Input.Position, 1.0));
			
			//! Transform the coordinate with the w-v-p matrix.
			Output.Position = mul(Output.Position, WorldViewProjectionMatrix);
			
			//! Compute the final vertex normal by multiplying with the normal matrix and the generated rotation matrix.
			Normal = normalize(
				mul(mul((float3x3)RotationMatrix, Input.Normal), (float3x3)WorldMatrix)
			);
		}
		break;
		
		case DEMO_BLOAT:
		{
			//! Bloat the coordinate by morphing along the un-transformed normal vector.
			Output.Position.xyz	= Input.Position + Input.Normal * TransformFactor;
			Output.Position.w	= 1.0;
			
			//! Transform the coordinate with the w-v-p matrix.
			Output.Position = mul(Output.Position, WorldViewProjectionMatrix);
			
			//! Compute the final vertex normal by multiplying with the normal matrix and normalize the vector.
			Normal = normalize(mul(Input.Normal, (float3x3)WorldMatrix));
		}
		break;
	}
	
	/**
	Compute simple directional lighting by calculating the dot-product of the final
	transformed vertex normal and the light direction vector and clamp the light intensity
	to the range of [0.2 .. 1.0].
	*/
	float LightIntensity = max(0.2, -dot(Normal, LightDirection));
	
	Output.Color.rgb	= Input.Color.rgb * LightIntensity;
	Output.Color.a		= Input.Color.a;
	
	//! Pass the whole texture coordinate without manipulation.
	Output.TexCoord0 = Input.TexCoord0;
	Output.TexCoord1 = Input.TexCoord1;
	Output.TexCoord2 = Input.TexCoord2;
	Output.TexCoord3 = Input.TexCoord3;
	Output.TexCoord4 = Input.TexCoord4;
	Output.TexCoord5 = Input.TexCoord5;
	Output.TexCoord6 = Input.TexCoord6;
	Output.TexCoord7 = Input.TexCoord7;
	
	//! Return the output data.
	return Output;
}


