// ####################################################
// # Direct3D Shader "BumpMapping" (Shader Model 3.0) #
// ####################################################
//
// Copyright (c) 2010 - Lukas Hermanns
//

/*
 * Global members
 */

//! Global member: World matrix (Object space).
float4x4 WorldMatrix;

//! WorldViewProjection matrix used to transform the vertex coordinate.
float4x4 WorldViewProjectionMatrix;

//! Color map.
sampler2D ColorMap;

//! Normal map where the normal vectors for each texel (or rather pixel) are stored in form of colors.
sampler2D NormalMap;

//! Enables or disables bump mapping.
bool EnableBumps;

//! Light #1 position (if you want to use more than 1 light add new uniforms and "ProcessLighting calles using the other values).
float3 Light1_Position;

//! Light #1 diffuse color.
float4 Light1_Diffuse;

//! Light #1 specular color.
float4 Light1_Specular;

//! Light #1 radius.
float Light1_Radius;

//! Light #1 shininess factor.
float Light1_Shininess;


/*
 * Structures
 */

struct VertexInput
{
	float3 Position	: POSITION;
	float3 Normal	: NORMAL;
	float4 Color	: COLOR;
	float3 TexCoord	: TEXCOORD0;
	float3 Tangent	: TEXCOORD1;
	float3 Binormal	: TEXCOORD2;
};

struct VertexPixelExchange
{
	float4 Position	: POSITION;
	float3 Normal	: NORMAL;
	float4 Color	: COLOR;
	float2 TexCoord	: TEXCOORD0;
	float3 Tangent	: TEXCOORD1;
	float3 Binormal	: TEXCOORD2;
	float4 WorldPos	: TEXCOORD3;
};


/*
 * Functions
 */

//! Lighting processing function.
void ProcessLighting(
	float3 PixelNormal, float3 VertexPosition, float3x3 NormalMatrix,
	float3 LightPosition, float3 LightDiffuse, float3 LightSpecular, float LightRadius, float LightShininess,
	inout float3 DiffuseColor, inout float3 SpecularColor)
{
	//! Get the light direction (For each pixel to have a point light) and final normal vector.
	float3 LightDir			= normalize(VertexPosition - LightPosition);
	float3 FinalNormal		= normalize(mul(PixelNormal, NormalMatrix));
	
	//! Compute the light intensity interim value.
	float LightIntensity	= -dot(FinalNormal, LightDir);
	
	//! Define the attenuation values like in Direct3D9 standard lighting (But in this case for per-pixel lighting).
    float AttenuationConstant	= 1.0;
    float AttenuationLinear		= 1.0 / LightRadius;
    float AttenuationQuadratic	= 1.0 / LightRadius;
	
	//! Get distance between the vertex- and the light position.
	float Distance 		= distance(VertexPosition, LightPosition);
	
	//! Compute attenuation to reduce lighting for pixels which are farther from the light source.
	float Attenuation	= 1.0 / (
		AttenuationConstant + AttenuationLinear * Distance + AttenuationQuadratic * Distance * Distance
	);
	
	//! Compute the light intensities using the attenuation and shininess for the specular color.
	float DiffuseIntensity	= max(0.0, LightIntensity) * Attenuation;
	
	//! Compute the shininess for the specular color.
	float3 ViewPos			= 0.0;
	float3 ViewDir			= normalize(VertexPosition - ViewPos);
	float3 ReflectedVec		= normalize(reflect(LightDir, FinalNormal));
	float NdotHV			= max(0.0, -dot(ViewDir, ReflectedVec));
	
	float SpecularIntensity	= pow(NdotHV, LightShininess) * Attenuation;
	
	//! Apply the lighting colors to the output values.
	DiffuseColor	+= LightDiffuse * DiffuseIntensity;
	SpecularColor	+= LightSpecular * SpecularIntensity;
}


/*
 * Vertex shader
 */

VertexPixelExchange VertexMain(VertexInput Input)
{
	VertexPixelExchange Output = (VertexPixelExchange)0;
	
	//! Use standard transformation.
	Output.Position = mul(float4(Input.Position, 1.0), WorldViewProjectionMatrix);
	
	//! Pass the vertex position (World space)
	Output.WorldPos = mul(float4(Input.Position, 1.0), WorldMatrix);
	
	/**
	Update tangent space. This is important for correct lighting computations.
	We need a tangent space rotation matrix to transform (Or rather rotate) the pixel normal which is stored in the normal map.
	*/
	Output.Normal	= mul(Input.Normal, (float3x3)WorldMatrix);
	Output.Tangent	= mul(Input.Tangent, (float3x3)WorldMatrix);
	Output.Binormal	= mul(Input.Binormal, (float3x3)WorldMatrix);
	
	//! Pass the vertex color.
	Output.Color = Input.Color;
	
	//! Pass the first texture coordinate.
	Output.TexCoord = Input.TexCoord.xy;
	
	return Output;
}


/*
 * Pixel shader
 */

float4 PixelMain(VertexPixelExchange Input) : COLOR
{
	float4 Color = 1.0;
	
	//! Get the base color by multiplying the texture color with the vertex color.
	float4 BaseColor = tex2D(ColorMap, Input.TexCoord) * Input.Color;
	
	//! Pixel normal set to the default direction when bump mapping is disabled.
	float3 PixelNormal = float3(0.0, 0.0, 1.0);
	
	if (EnableBumps)
	{
		//! Get the normal vector from the normal map.
		PixelNormal = tex2D(NormalMap, Input.TexCoord).rgb;
		
		//! Convert the color to a normal vector.
		PixelNormal = PixelNormal*2.0 - 1.0;
	}
	
	//! Define the variables where the final lighting colors are to be stored.
	float3 LightDiffuseColor	= 0.0;
	float3 LightSpecularColor	= 0.0;
	
	//! Process lighting calculations for light #1.
	ProcessLighting(
		PixelNormal, Input.WorldPos, float3x3(Input.Tangent, Input.Binormal, Input.Normal),
		Light1_Position, Light1_Diffuse.rgb, Light1_Specular.rgb, Light1_Radius, Light1_Shininess,
		LightDiffuseColor, LightSpecularColor
	);
	
	//! You can add more "ProcessLighting" calls for further light sources here (Just exchange the "Light1_..." variables) ...
	
	//! Set the final pixel color.
	Color.rgb = (
		BaseColor.rgb * clamp(LightDiffuseColor + 0.2, 0.0, 1.0) + LightSpecularColor
	);
	
	//! Set the final pixel transparency.
	Color.a = BaseColor.a;
	
	return Color;
}

