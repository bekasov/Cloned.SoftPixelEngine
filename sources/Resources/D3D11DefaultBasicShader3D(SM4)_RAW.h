// #########################################################
// # Direct3D11 Default Basic Shader 3D (Shader Model 4.0) #
// #########################################################
//
// ========== Copyright (c) 2010 - Lukas Hermanns ==========
//


/*
 * Global members
 */

Texture2D Texture2D0		: register(t0);
Texture2D Texture2D1		: register(t1);
Texture2D Texture2D2		: register(t2);
Texture2D Texture2D3		: register(t3);

SamplerState SamplerLinear0	: register(s0);
SamplerState SamplerLinear1	: register(s1);
SamplerState SamplerLinear2	: register(s2);
SamplerState SamplerLinear3	: register(s3);


/*
 * Constant buffer structures
 */

struct SLight
{
	int Model;							// Light model (Directionl, Point, Spot)
	int Enabled;						// Enabled/ disabled
	int2 pad1;
	float4 Position;					// Position for Point- and Spot light and Direction for Directional light
	float4 Diffuse, Ambient, Specular;	// Light colors
	float4 SpotDir;						// Spot light direction
	float Attn0, Attn1, Attn2;			// Attunation values
	int pad2;
	float Theta, Phi, Falloff, Range;	// Spot light attributes
};

struct SMaterial
{
	float4 Diffuse, Ambient, Specular, Emission;	// Material colors
	int Shading;									// Shading (flat, gouraud, phong, perpixel)
	int LightingEnabled;							// Global lighting enabled/ disabled
	int FogEnabled;									// Global fog enabled/ disabled
	float Shininess;								// Specular shininess
	int AlphaMethod;								// Alpha test function
	float AlphaReference;							// Alpha test reference value
	int2 pad;
};

struct STexture
{
	int3 MapGenType;	// Texture coordinate generation
	int EnvType;		// Texture environment
	float4x4 Matrix;	// Texture coordiante transformation
};

struct SClipPlane
{
	int Enabled;	// Enabled/ disabled
	int3 pad;
	float3 Normal;	// Plane normal vector
	float Distance;	// Plane distance to the origin
};

struct SFogStates
{
	int Mode;			// Fog mode (Plane, Thick etc.)
	float Density;		// Density/ thickness
	float Near, Far;	// Near/ far planes
	float4 Color;		// Fog color
};


/*
 * Constant buffers
 */

cbuffer ConstantBufferLights			: register(b0)
{
	SLight Lights[8];	// Light sources
};

cbuffer ConstantBufferObject			: register(b1)
{
	float4x4 WorldMatrix, ViewMatrix, ProjectionMatrix;	// Matrices
	SMaterial Material;									// Material attributes
};

cbuffer ConstantBufferSurface			: register(b2)
{
	int TextureLayers;		// Count of texture layers
	int3 pad;
	STexture Textures[8];	// Texture surfaces
};

cbuffer ConstantBufferDriverSettings	: register(b3)
{
	SClipPlane Planes[8];	// Clipping planes
	SFogStates Fog;			// Fog effect states
};


/*
 * Macros
 */

#define MAPGEN_DISABLE 			0
#define MAPGEN_OBJECT_LINEAR	1
#define MAPGEN_EYE_LINEAR		2
#define MAPGEN_SPHERE_MAP		3
#define MAPGEN_NORMAL_MAP		4
#define MAPGEN_REFLECTION_MAP	5

#define LIGHT_DIRECTIONAL		0
#define LIGHT_POINT				1
#define LIGHT_SPOT				2

#define SHADING_FLAT			0
#define SHADING_GOURAUD			1
#define SHADING_PHONG			2
#define SHADING_PERPIXEL		3

#define TEXENV_MODULATE			0
#define TEXENV_REPLACE			1
#define TEXENV_ADD				2
#define TEXENV_ADDSIGNED		3
#define TEXENV_SUBTRACT			4
#define TEXENV_INTERPOLATE		5
#define TEXENV_DOT3				6

#define CMPSIZE_NEVER			0
#define CMPSIZE_EQUAL			1
#define CMPSIZE_NOTEQUAL		2
#define CMPSIZE_LESS			3
#define CMPSIZE_LESSEQUAL		4
#define CMPSIZE_GREATER			5
#define CMPSIZE_GREATEREQUAL	6
#define CMPSIZE_ALWAYS			7

#define FOG_STATIC_PALE			0
#define FOG_STATIC_THICK		1
#define FOG_VOLUMETRIC			2


/*
 * Structures
 */

struct VertexInput
{
	float3 Position		: POSITION;
	float3 Normal		: NORMAL;
	float4 Color		: COLOR;
	float2 TexCoord0	: TEXCOORD0;
	float2 TexCoord1	: TEXCOORD1;
	float2 TexCoord2	: TEXCOORD2;
	float2 TexCoord3	: TEXCOORD3;
};

struct VertexPixelExchange
{
	float4 Position		: SV_POSITION;
	float3 Normal		: NORMAL;
	float4 Diffuse		: COLOR0;
	float4 Ambient		: COLOR1;
	float2 TexCoord0	: TEXCOORD0;
	float2 TexCoord1	: TEXCOORD1;
	float2 TexCoord2	: TEXCOORD2;
	float2 TexCoord3	: TEXCOORD3;
	
	float4 WorldPos		: POSITION1;
	float4 WorldViewPos	: POSITION2;
};

struct PixelOutput
{
	float4 Color : SV_TARGET;
};


/*
 * Inline functions
 */

inline void DiscardPixel()
{
	clip(-1.0f);
}


/*
 * Functions
 */

void LightCalculation(int i, float3 Normal, float3 Position, inout float4 ColorOut)
{
	float Intensity = 1.0f;
	
	// Compute intensity
	switch (Lights[i].Model)
	{
		case LIGHT_DIRECTIONAL:
			Intensity = saturate(dot(Normal, Lights[i].Position.xyz));
			break;
		case LIGHT_POINT:
			Intensity = saturate(dot(Normal, normalize(Lights[i].Position.xyz - Position)));
			break;
		case LIGHT_SPOT:
			break;
	}
	
	// Compute attenuation
	if (Lights[i].Model != LIGHT_DIRECTIONAL)
	{
		const float Distance = distance(Lights[i].Position.xyz, Position);
		float Attenuation = 1.0f / ( Lights[i].Attn0 + Lights[i].Attn1*Distance + Lights[i].Attn2*Distance*Distance );
		Intensity *= Attenuation;
	}
	
	// Apply light color
	ColorOut += Lights[i].Diffuse * Intensity;
	//ColorOut += Lights[i].Specular * pow(Intensity, Material.Shininess);
}

void FogCalculation(float Depth, inout float4 Color)
{
	float Factor = 0.0f;
	
	switch (Fog.Mode)
	{
		case FOG_STATIC_PALE:
			Factor = exp(-Fog.Density * Depth); break;
		case FOG_STATIC_THICK:
		{
			float TempFactor = Fog.Density * Depth;
			Factor = exp(-TempFactor * TempFactor);
		}
		break;
	}
	
	clamp(Factor, 0.0f, 1.0f);
	
	Color.xyz = Fog.Color.xyz * (1.0f - Factor) + Color.xyz * Factor;
}

bool ClippingPlane(int i, float4 Position)
{
	return dot(Position.xyz, normalize(Planes[i].Normal)) + Planes[i].Distance < 0.0f;
}

void TexCoordGeneration(int MapGenType, float Pos, float WorldViewPos, float TransNormal, float TexCoordIn, inout float TexCoordOut)
{
	// Texture coordinate generation
	switch (MapGenType)
	{
		case MAPGEN_DISABLE:
			TexCoordOut = TexCoordIn; break;
		case MAPGEN_OBJECT_LINEAR:
			TexCoordOut = Pos; break;
		case MAPGEN_EYE_LINEAR:
			TexCoordOut = WorldViewPos; break;
		case MAPGEN_SPHERE_MAP:
			TexCoordOut = TransNormal*0.5f + 0.5f; break;
		case MAPGEN_NORMAL_MAP:
			break; //!TODO!
		case MAPGEN_REFLECTION_MAP:
			break; //!TODO!
	}
}

void TextureMapping(int i, Texture2D Tex, SamplerState Sampler, float2 TexCoord, inout float4 ColorOut)
{
	const float4 TexColor = Tex.Sample(Sampler, TexCoord);
	
	switch (Textures[i].EnvType)
	{
		case TEXENV_MODULATE:
			ColorOut *= TexColor; break;
		case TEXENV_REPLACE:
			ColorOut = TexColor; break;
		case TEXENV_ADD:
			ColorOut += TexColor; break;
		case TEXENV_ADDSIGNED:
			ColorOut += TexColor - float4(0.5f, 0.5f, 0.5f, 1.0f); break;
		case TEXENV_SUBTRACT:
			ColorOut -= TexColor; break;
		case TEXENV_INTERPOLATE:
			break; //!TODO!
		case TEXENV_DOT3:
			ColorOut.r = (ColorOut.r - 0.5f)*(TexColor.r - 0.5f);
			ColorOut.g = (ColorOut.g - 0.5f)*(TexColor.g - 0.5f);
			ColorOut.b = (ColorOut.b - 0.5f)*(TexColor.b - 0.5f);
			break;
	}
}


/*
 * ======= Vertex Shader =======
 */

VertexPixelExchange VertexMain(VertexInput Input)
{
	#define mcrTexCoordGeneration(t, c)																									\
		TexCoordGeneration(Textures[t].MapGenType.x, Input.Position.x, Output.WorldViewPos.x, TransNormal.x, Input.c.x, Output.c.x);	\
		TexCoordGeneration(Textures[t].MapGenType.y, Input.Position.y, Output.WorldViewPos.y, TransNormal.y, Input.c.y, Output.c.y);
	#define mcrTexCoordTransform(i, t)																\
		Output.t = mul(float4(Output.t.x, Output.t.y, 0.0f, 1.0f), Textures[i].Matrix).xy;
	
	// Temporary variables
	VertexPixelExchange Output = (VertexPixelExchange)0;
	
	// Compute vertex positions (local, gloabl, projected)
	Output.WorldPos		= mul(float4(Input.Position, 1.0), WorldMatrix);
	Output.WorldViewPos	= mul(Output.WorldPos, ViewMatrix);
	Output.Position		= mul(Output.WorldViewPos, ProjectionMatrix);
	
	// Compute normals
	float3 TransNormal	= mul(Input.Normal, WorldMatrix);
	TransNormal			= mul(TransNormal, ViewMatrix);
	TransNormal			= normalize(TransNormal);
	Output.Normal		= TransNormal;
	
	// Compute final vertex color
	Output.Diffuse = Input.Color * Material.Diffuse;
	Output.Ambient = Material.Ambient;
	
	// Light computations
	if (Material.LightingEnabled && Material.Shading < SHADING_PHONG)
	{
		float4 LightColor = Output.Ambient;
		
		for (int i = 0; i < 8; ++i)
		{
			if (Lights[i].Enabled)
				LightCalculation(i, Output.Normal, Output.WorldViewPos.xyz, LightColor);
		}
		
		Output.Diffuse.rgb *= LightColor.rgb;
	}
	
	// Compute texture coordinates
	if (TextureLayers > 0)
	{
		mcrTexCoordGeneration(0, TexCoord0);
		mcrTexCoordTransform(0, TexCoord0);
		
		if (TextureLayers > 1)
		{
			mcrTexCoordGeneration(1, TexCoord1);
			mcrTexCoordTransform(1, TexCoord1);
			
			if (TextureLayers > 2)
			{
				mcrTexCoordGeneration(2, TexCoord2);
				mcrTexCoordTransform(2, TexCoord2);
				
				if (TextureLayers > 3)
				{
					mcrTexCoordGeneration(3, TexCoord3);
					mcrTexCoordTransform(3, TexCoord3);
				} // fi tex-layer 3
			} // fi tex-layer 2
		} // fi tex-layer 1
	} // fi tex-layer 0
	
	return Output;
	
	#undef mcrTexCoordGeneration
	#undef mcrTexCoordTransform
}


/*
 * ======= Pixel Shader =======
 */

PixelOutput PixelMain(VertexPixelExchange Input)
{
	// Temporary variables
	PixelOutput Output = (PixelOutput)0;
	
	float4 TexColor		= 1.0f;
	float4 LightColor	= 1.0f;
	
	// Light computations
	if (Material.LightingEnabled && Material.Shading >= SHADING_PHONG)
	{
		LightColor = Input.Ambient;
		const float3 Normal = normalize(Input.Normal);
		
		for (int i = 0; i < 8; ++i)
		{
			if (Lights[i].Enabled)
				LightCalculation(i, Normal, Input.WorldViewPos.xyz, LightColor);
		}
	}
	
	// Texture mapping
	if (TextureLayers > 0)
	{
		TextureMapping(0, Texture2D0, SamplerLinear0, Input.TexCoord0, TexColor);
		if (TextureLayers > 1)
		{
			TextureMapping(1, Texture2D1, SamplerLinear1, Input.TexCoord1, TexColor);
			if (TextureLayers > 2)
			{
				TextureMapping(2, Texture2D2, SamplerLinear2, Input.TexCoord2, TexColor);
				if (TextureLayers > 3)
				{
					TextureMapping(3, Texture2D3, SamplerLinear3, Input.TexCoord3, TexColor);
				} // fi tex-layer 3
			} // fi tex-layer 2
		} // fi tex-layer 1
	} // fi tex-layer 0
	
	// Final color output
	Output.Color = Input.Diffuse * TexColor * float4(LightColor.rgb, 1.0);
	
	// Fog computations
	if (Material.FogEnabled)
		FogCalculation(Input.WorldViewPos.z, Output.Color);
	
	// Alpha reference method
	switch (Material.AlphaMethod)
	{
		case CMPSIZE_ALWAYS:
			break;
		case CMPSIZE_GREATEREQUAL:
			if (!(Output.Color.a >= Material.AlphaReference))
				DiscardPixel();
			break;
		case CMPSIZE_GREATER:
			if (!(Output.Color.a > Material.AlphaReference))
				DiscardPixel();
			break;
		case CMPSIZE_LESSEQUAL:
			if (!(Output.Color.a <= Material.AlphaReference))
				DiscardPixel();
			break;
		case CMPSIZE_LESS:
			if (!(Output.Color.a < Material.AlphaReference))
				DiscardPixel();
			break;
		case CMPSIZE_NOTEQUAL:
			if (Output.Color.a == Material.AlphaReference)
				DiscardPixel();
			break;
		case CMPSIZE_EQUAL:
			if (Output.Color.a != Material.AlphaReference)
				DiscardPixel();
			break;
		case CMPSIZE_NEVER:
			DiscardPixel(); break;
	}
	
	// Process clipping planes
	for (int i = 0; i < 8; ++i)
	{
		if (Planes[i].Enabled && ClippingPlane(i, Input.WorldPos))
		{
			DiscardPixel();
			return Output;
		}
	}
	
	// Return the output color
	return Output;
}

