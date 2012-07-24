// #####################################################
// # Direct3D Shader "Tessellation" (Shader Model 5.0) #
// #####################################################
//
// Copyright (c) 2010 - Lukas Hermanns
//

/*
 * Global members
 */

//! Color map.
Texture2D ColorMap			: register(t0);

//! Normal map where the heightmap information is stored in the alpha-channel.
Texture2D NormalHeightMap	: register(t1);

//! Sampler used to get the needed texels.
SamplerState SamplerColorMap		: register(s0);
SamplerState SamplerNormalHeightMap	: register(s1);

/**
Since Shader Model 4.0 constants needed to be set in constant buffers. You can create multiple constant buffers
e.g. one for the object location, one for the material settings and an other for the light positions. Then you only need
to update the constant buffers for the light positions only one times for each object which is better for the performance.
*/
cbuffer BufferObject	: register(b0)
{
	float4x4 WorldMatrix;
	float4x4 ViewProjectionMatrix;
	
	float4 CameraPosition;
	
	float4 LightPosition;
	float4 LightDiffuse;
	float4 LightSpecular;
	
	float Shininess;
	float3 Dummy1;
};

cbuffer BufferSurface	: register(b1)
{
	float HeightFactor;
	float3 Dummy2;
};


/*
 * Macros
 */

#define DIST_BIAS		0.05
#define DIST_FULLBIAS	1.05
#define DIST_SCALE		1.0


/*
 * Structures
 */

struct VertexInput
{
	float3 Position	: POSITION;
	float3 Tangent	: TANGENT;					//!< Tangent (Tangent space) for bump mapping.
	float3 Binormal	: BINORMAL;					//!< Binormal (Tangent space) for bump mapping.
	float3 Normal	: NORMAL;					//!< Normal (Tangent space) for bump mapping.
	float3 TexCoord	: TEXCOORD0;
};

struct VertexOutput
{
	float4 Position	: WORLDPOS;					//!< Vertex position (Global position in World-View space).
	float3 Tangent	: TANGENT;
	float3 Binormal	: BINORMAL;
	float3 Normal	: NORMAL;
	float3 TexCoord	: TEXCOORD0;
	float Distance	: VERTEXDISTANCEFACTOR;		//!< Vertex distance factor used to compute the tessellation factor.
};

struct HullDataOutput
{
	float Edges[3]	: SV_TessFactor;			//!< Final triangle edges tessellation factor. Used to avoid cracks between the tessellated triangles.
	float Inside	: SV_InsideTessFactor;		//!< Internal triangle tessellation factor.
};

struct PixelInput
{
	float4 Position		: SV_Position;			//!< Pixel position (Screen space).
	float4 WorldPos		: WORLDPOS;				//!< Pixel position (World-View space).
	float3 TexCoord		: TEXCOORD0;
	float3 Tangent		: TANGENT;
	float3 Binormal		: BINORMAL;
	float3 Normal		: NORMAL;
};

struct PixelOutput
{
	float4 Color : SV_Target;					//!< Final pixel color.
};


/*
 * Vertex shader
 */

VertexOutput VertexMain(VertexInput Input)
{
	VertexOutput Output = (VertexOutput)0;
	
	//! Transform the vertex coordinate with the world matrix. Projection and other transformation are performed in the other shaders.
	Output.Position	= mul(WorldMatrix, float4(Input.Position, 1.0));
	
	//! Transform the tangent space vectors with the rotation of the world matrix.
	Output.Tangent	= mul((float3x3)WorldMatrix, Input.Tangent);
	Output.Binormal	= mul((float3x3)WorldMatrix, Input.Binormal);
	Output.Normal	= mul((float3x3)WorldMatrix, Input.Normal);
	
	//! Pass the texture coordinate.
	Output.TexCoord	= Input.TexCoord;
	
	//! Compute the vertex distance factor. The farther the vertex the lower the tessellation factor.
	const float MinDistance = 5.0f;
	
	float Distance = distance(Output.Position, CameraPosition);
	
	Output.Distance = saturate(
		DIST_FULLBIAS / ( max(0.0, Distance - MinDistance) * DIST_SCALE + DIST_FULLBIAS ) - DIST_BIAS
	);
	
	return Output;
}


/*
 * Hull shader
 */

/**
This is the HullShader callback procedure which calculates the tessellation factor.
For tessellation there are three new render stages: HullShader (programmable) -> Tessellator (fixed) -> DomainShader (programmable).
In the hull shader we compute the tessellation factor for the triangle and for each edge (similar idea to terrain patches).
Then the tessellator will generate all new trianlges. In the domain shader we manipulate the vertices of the new tessellated triangles.
In this case we use a height map to transformation the new vertex coordinates. But you could also create a sphere out of only one triangle
when it's tessellated high enough.
*/
HullDataOutput HullConstantProc(InputPatch<VertexOutput, 3> Input)
{
	HullDataOutput Output = (HullDataOutput)0;
	
	float3 EdgeSize;
	
	//! Calculate the interim values for the edges tessellation factors. The factor in the range [0.0 .. 1.0].
	EdgeSize.x = 0.5f * ( Input[1].Distance + Input[2].Distance );
	EdgeSize.y = 0.5f * ( Input[2].Distance + Input[0].Distance );
	EdgeSize.z = 0.5f * ( Input[0].Distance + Input[1].Distance );
	
	//! Multiply the factors with the maximal tessellation factor.
	EdgeSize *= 63.0;
	
	//! Clamp the values.
	EdgeSize.x = clamp(EdgeSize.x, 1.0, 63.0);
	EdgeSize.y = clamp(EdgeSize.y, 1.0, 63.0);
	EdgeSize.z = clamp(EdgeSize.z, 1.0, 63.0);
	
	//! Pass the edge values and use the interim value of all factors for the inside value.
	Output.Edges[0]	= EdgeSize.x;
	Output.Edges[1]	= EdgeSize.y;
	Output.Edges[2]	= EdgeSize.z;
	Output.Inside	= clamp((EdgeSize.x + EdgeSize.y + EdgeSize.z) * 0.333, 1.0, 64.0);
	
	return Output;
}

[domain("tri")]											//!< We use triangles (quads are also possible).
[partitioning("fractional_odd")]						//!< Tessellation method for the tessellator stage.
[outputtopology("triangle_cw")]							//!< Output triangle topology is clock-wise.
[outputcontrolpoints(3)]								//!< Output control points are 3.
[patchconstantfunc("HullConstantProc")]					//!< Name of our HullShader callback procedure.
[maxtessfactor(64.0)]									//!< Maximal tessellation factor. More then 64.0 is not possible!
VertexOutput HullMain(
	InputPatch<VertexOutput, 3> Input,					//!< Input patches.
	uint ControlPointID : SV_OutputControlPointID)		//!< Index number of the current input patch.
{
	VertexOutput Output = (VertexOutput)0;
	
	//! Just pass the whole vertex output data.
	Output.Position	= Input[ControlPointID].Position;
	Output.Tangent	= Input[ControlPointID].Tangent;
	Output.Binormal	= Input[ControlPointID].Binormal;
	Output.Normal	= Input[ControlPointID].Normal;
	Output.TexCoord	= Input[ControlPointID].TexCoord;
	Output.Distance	= Input[ControlPointID].Distance;
	
	return Output;
}


/*
 * Domain shader
 */

//! This function calculates the height field position using the normal map's alpha channel where the height map information are stored.
float4 GetHeightFieldPosition(float4 Position, float3 Normal, float2 TexCoord, float Factor)
{
	//! Get the height field value out of the height map.
	float HeightField = NormalHeightMap.SampleLevel(SamplerNormalHeightMap, TexCoord, 0.0).a;
	
	//! Move along the normal vector.
	float3 ResultPosition = Position.xyz + (Normal * HeightField * HeightFactor * Factor);
	
	//! Return the new position and keep the RHW (Reciprocal Homogenous W) coordinate.
	return float4(ResultPosition, Position.w);
}

//! This function returns barycentric coordinate to get the correct position of the individual tessellated triangle's vertex.
float4 GetBarycentric(float4 Vec0, float4 Vec1, float4 Vec2, float3 Barycentric)
{
	return Barycentric.x * Vec0 + Barycentric.y * Vec1 + Barycentric.z * Vec2;
}
float3 GetBarycentric(float3 Vec0, float3 Vec1, float3 Vec2, float3 Barycentric)
{
	return Barycentric.x * Vec0 + Barycentric.y * Vec1 + Barycentric.z * Vec2;
}
float GetBarycentric(float Vec0, float Vec1, float Vec2, float3 Barycentric)
{
	return Barycentric.x * Vec0 + Barycentric.y * Vec1 + Barycentric.z * Vec2;
}

[domain("tri")]
PixelInput DomainMain(
	HullDataOutput Input,									//!< HullShader output data.
	float3 BarycentricCoordinates : SV_DomainLocation,		//!< Barycentric coordinate factors in the range [0.0 .. 1.0] use to interpolate between
															//!  all vertices inside the triangle which is to be tessellated.
	const OutputPatch<VertexOutput, 3> TrianglePatch)		//!< Tessellated vertex output data.
{
	PixelInput Output = (PixelInput)0;
	
	//! Interpolate between each vertex data.
	Output.Position	= GetBarycentric(TrianglePatch[0].Position, TrianglePatch[1].Position, TrianglePatch[2].Position, BarycentricCoordinates);
	Output.Tangent	= GetBarycentric(TrianglePatch[0].Tangent , TrianglePatch[1].Tangent , TrianglePatch[2].Tangent , BarycentricCoordinates);
	Output.Binormal	= GetBarycentric(TrianglePatch[0].Binormal, TrianglePatch[1].Binormal, TrianglePatch[2].Binormal, BarycentricCoordinates);
	Output.Normal	= GetBarycentric(TrianglePatch[0].Normal  , TrianglePatch[1].Normal  , TrianglePatch[2].Normal  , BarycentricCoordinates);
	Output.TexCoord	= GetBarycentric(TrianglePatch[0].TexCoord, TrianglePatch[1].TexCoord, TrianglePatch[2].TexCoord, BarycentricCoordinates);
	
	//! Pass the global vertex position for bump mapping.
	Output.WorldPos = GetHeightFieldPosition(
		Output.Position, Output.Normal, Output.TexCoord.xy,
		GetBarycentric(TrianglePatch[0].Distance, TrianglePatch[1].Distance, TrianglePatch[2].Distance, BarycentricCoordinates)
	);
	
	//! Final vertex coordinate transformation with adding height field information.
	Output.Position = mul(ViewProjectionMatrix, Output.WorldPos);
	
	return Output;
}


/*
 * Pixel shader
 */

PixelOutput PixelMain(PixelInput Input)
{
	PixelOutput Output = (PixelOutput)0;
	
	//! Normalize the tangent space.
	float3 Tangent	= normalize(Input.Tangent);
	float3 Binormal	= normalize(Input.Binormal);
	float3 Normal	= normalize(Input.Normal);
	
	//! Get the normal matrix out of the three tangent space vectors.
	float3x3 NormalMatrix = float3x3(Tangent, Binormal, Normal);
	
	//! Get the pixel normal vector out of the normal map.
	float3 PixelNormal = NormalHeightMap.Sample(SamplerNormalHeightMap, Input.TexCoord.xy).rgb;
	
	//! Convert the pixel normal data into a real vector.
	PixelNormal = (PixelNormal - 0.5) * 2.0;
	
	//! Transform and normalize the pixel normal vector.
	PixelNormal = normalize(mul(NormalMatrix, PixelNormal));
	
	//! Simple lighting computations as shown in several shaders of the "ShaderLibrary" tutorial.
	float3 LightDirection = normalize(Input.WorldPos.xyz - LightPosition.xyz);
	
	float NdotL = -dot(PixelNormal, LightDirection);
	
	float DiffuseIntensity	= max(0.2, NdotL);
	
	//! Compute the shininess for the specular color.
	float3 ViewDir			= normalize(Input.WorldPos.xyz - CameraPosition.xyz);
	float3 ReflectedVec		= normalize(reflect(LightDirection, PixelNormal));
	float NdotHV			= max(0.0, -dot(ViewDir, ReflectedVec));
	
	float SpecularIntensity	= pow(NdotHV, Shininess);
	
	//! Get the color map texel.
	Output.Color = ColorMap.Sample(SamplerColorMap, Input.TexCoord.xy);
	
	//! Calculate the final pixel color.
	Output.Color.rgb = Output.Color.rgb * LightDiffuse.rgb * DiffuseIntensity + LightSpecular.rgb * SpecularIntensity;
	
	return Output;
}

