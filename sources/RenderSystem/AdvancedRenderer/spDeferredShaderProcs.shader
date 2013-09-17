/*
 * Deferred shader procedures file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

float GetAngle(in float3 a, in float3 b)
{
    return acos(dot(a, b));
}

float GetSpotLightIntensity(in float3 LightDir, in SLightEx LightEx)
{
	/* Compute spot light cone */
	float Angle = GetAngle(LightDir, LightEx.Direction);
	float ConeAngleLerp = (Angle - LightEx.SpotTheta) / LightEx.SpotPhiMinusTheta;
	
	return saturate(1.0 - ConeAngleLerp);
}

#ifdef SHADOW_MAPPING

/**
Chebyshev inequality function for VSM (variance shadow maps)
see GPUGems3 at nVIDIA for more details:
http://http.developer.nvidia.com/GPUGems3/gpugems3_ch08.html
*/
float ChebyshevUpperBound(in float2 Moments, in float t)
{
    /* One-tailed inequality valid if t > Moments.x */
	float p = step(t, Moments.x);
	
    /* Compute variance */
    float Variance = Moments.y - (Moments.x*Moments.x);
    Variance = max(Variance, MIN_VARIANCE);
    
    /* Compute probabilistic upper bound. */
    float d = t - Moments.x;
    float p_max = Variance / (Variance + d*d);
    
    return max(p, p_max);
}

float LinStep(in float min, in float max, in float v)
{
    return saturate((v - min) / (max - min));
}

float ReduceLightBleeding(in float p_max, in float Amount)
{
    /* Remove the [0, amount] ail and linearly rescale [amount, 1] */
    return LinStep(Amount, 1.0, p_max);
}

float ShadowContribution(in float2 Moments, in float LightDistance)
{
    /* Compute the Chebyshev upper bound */
    float p_max = ChebyshevUpperBound(Moments, LightDistance);
    return ReduceLightBleeding(p_max, 0.6);
}

//! World position projection function.
float4 Projection(in float4x4 ProjectionMatrix, in float4 WorldPos)
{
    float4 ProjectedPoint = MUL(ProjectionMatrix, WorldPos);

    ProjectedPoint.xy = (ProjectedPoint.xy / CAST(float2, ProjectedPoint.w) + CAST(float2, 1.0)) * CAST(float2, 0.5);

    return ProjectedPoint;
}

#	ifdef GLOBAL_ILLUMINATION

//! Virtual point light shading function.
bool ComputeVPLShading(in float3 WorldPos, in float3 Normal, in float3 IndirectPoint, inout float IntensityIL)
{
	/* Check if VPL is visible to pixel */
	float3 IndirectDir = IndirectPoint - WorldPos;
	
	if (dot(Normal, IndirectDir) <= 0.0)
		return false;
	
	/* Compute light attenuation */
	float DistanceIL = distance(WorldPos, IndirectPoint);
	
	float AttnLinearIL    = DistanceIL;// ... * VPLRadius;
	float AttnQuadraticIL = AttnLinearIL * DistanceIL;
	
	IntensityIL = saturate(1.0 / (1.0 + AttnLinearIL + AttnQuadraticIL));// - LIGHT_CUTOFF);
	
	/* Compute phong shading for indirect light */
	float NdotIL = saturate(dot(Normal, normalize(IndirectDir)));
	
	/* Clamp intensity to avoid singularities in VPLs */
	IntensityIL = min(VPL_SINGULARITY_CLAMP, IntensityIL * NdotIL) * GIReflectivity;
	
	return true;
}

//! Global illumination function for spot lights.
void ComputeVPLShadingSpotLight(
    in SLight Light, in SLightEx LightEx, in float3 WorldPos, in float3 Normal, inout float3 Diffuse)
{
	/* Compute VPLs (virtual point lights) */
	float3 IndirectTexCoord = float3(0.0, 0.0, float(Light.ShadowIndex));
	
	for (int i = 0; i < VPL_COUNT; ++i)
	{
		/* Get VPL offset */
		IndirectTexCoord.xy = VPLOffsets[i].xy;
		
		/* Sample indirect light distance */
		float IndirectDist = tex2DArray(DirLightShadowMaps, IndirectTexCoord).r;
		
		/* Get the indirect light's position */
		float4 LightRay = float4(IndirectTexCoord.x*2.0 - 1.0, 1.0 - IndirectTexCoord.y*2.0, 1.0, 1.0);
		LightRay = normalize(MUL(LightEx.InvViewProjection, LightRay));
		float3 IndirectPoint = Light.PositionAndInvRadius.xyz + LightRay.xyz * CAST(float3, IndirectDist);
		
		/* Shade indirect light */
		float IntensityIL = 0.0;
		
		if (ComputeVPLShading(WorldPos, Normal, IndirectPoint, IntensityIL))
		{
			/* Sample indirect light color */
			float3 IndirectColor = tex2DArray(DirLightDiffuseMaps, IndirectTexCoord).rgb;
			
			/* Apply VPL shading */
			IndirectColor *= CAST(float3, IntensityIL);
			Diffuse += IndirectColor;
		}
	}
}

//! Low-resolution light shading function for global illumination.
void ComputeLowResLightShadingVPL(
    in SLight Light, in SLightEx LightEx, in float3 WorldPos,
	in float3 Normal, inout float3 Diffuse)
{
    /* Compute diffuse color */
    Diffuse = CAST(float3, 0.0);
    float3 Specular = CAST(float3, 0.0);
	
    /* Apply shadow */
    if (Light.ShadowIndex != -1)
	{
		if (Light.Type == LIGHT_POINT)
		{
			//todo
		}
		else if (Light.Type == LIGHT_SPOT)
			ComputeVPLShadingSpotLight(Light, LightEx, WorldPos, Normal, Diffuse);
	}
	
	Diffuse *= Light.Color;
}

#	endif

#endif

//! Main light shading function.
void ComputeLightShading(
    in SLight Light, in SLightEx LightEx,
    in float3 WorldPos, in float3 Normal, in float Shininess, in float3 ViewRay,
	#ifdef HAS_LIGHT_MAP
	inout float3 StaticDiffuseColor, inout float3 StaticSpecularColor,
	#endif
    inout float3 DiffuseColor, inout float3 SpecularColor)
{
    /* Compute light direction vector */
    float3 LightDir = CAST(float3, 0.0);
	
    if (Light.Type != LIGHT_DIRECTIONAL)
        LightDir = normalize(WorldPos - Light.PositionAndInvRadius.xyz);
    else
        LightDir = LightEx.Direction;
	
    /* Compute phong shading */
    float NdotL = max(AMBIENT_LIGHT_FACTOR, dot(Normal, -LightDir));
	
    /* Compute light attenuation */
    float Distance = distance(WorldPos, Light.PositionAndInvRadius.xyz);
	
    float AttnLinear    = Distance * Light.PositionAndInvRadius.w;
    float AttnQuadratic = AttnLinear * Distance;
	
    float Intensity = (1.0 + LIGHT_CUTOFF) / (1.0 + AttnLinear + AttnQuadratic) - LIGHT_CUTOFF;
	
    if (Light.Type == LIGHT_SPOT)
		Intensity *= GetSpotLightIntensity(LightDir, LightEx);
	
    /* Compute diffuse color */
    float3 Diffuse = CAST(float3, Intensity * NdotL);
	
    /* Compute specular color */
    float3 Reflection = normalize(reflect(LightDir, Normal));
	
    float NdotHV = -dot(ViewRay, Reflection);
	
    float3 Specular = Light.Color * CAST(float3, Intensity * pow(max(0.0, NdotHV), Shininess));
	
    #ifdef SHADOW_MAPPING
	
    /* Apply shadow */
    if (Light.ShadowIndex != -1)
	{
		if (Light.Type == LIGHT_POINT)
		{
			//todo
		}
		else if (Light.Type == LIGHT_SPOT)
		{
			/* Get shadow map texture coordinate */
			float4 ShadowTexCoord = Projection(LightEx.ViewProjection, float4(WorldPos, 1.0));
			
			if ( ShadowTexCoord.x >= 0.0 && ShadowTexCoord.x <= 1.0 &&
				 ShadowTexCoord.y >= 0.0 && ShadowTexCoord.y <= 1.0 &&
				 ShadowTexCoord.z > 0.0 )
			{
				/* Adjust texture coordinate */
				ShadowTexCoord.y = 1.0 - ShadowTexCoord.y;
				ShadowTexCoord.z = float(Light.ShadowIndex);
				ShadowTexCoord.w = 2.0;//Distance*0.25;
				
				/* Sample moments from shadow map */
				float2 Moments = tex2DArrayLod(DirLightShadowMaps, ShadowTexCoord).ra;
				
				/* Compute shadow contribution */
				float Shadow = ShadowContribution(Moments, Distance);
				
				Diffuse *= CAST(float3, Shadow);
				Specular *= CAST(float3, Shadow);
			}
			
			#if defined(GLOBAL_ILLUMINATION) && !defined(USE_LOWRES_VPL_SHADING)
			
			if (GIReflectivity > 0.0)
				ComputeVPLShadingSpotLight(Light, LightEx, WorldPos, Normal, Diffuse);
			
			#endif
		}
	}
	
    #endif
	
	Diffuse *= Light.Color;
	
    /* Add light color */
	#ifdef HAS_LIGHT_MAP
	
	if (Light.UsedForLightmaps != 0)
	{
		StaticDiffuseColor += Diffuse;
		StaticSpecularColor += Specular;
	}
	else
	{
		DiffuseColor += Diffuse;
		SpecularColor += Specular;
	}
	
	#else
	
    DiffuseColor += Diffuse;
    SpecularColor += Specular;
	
	#endif
}

#ifdef TILED_SHADING

uint2 GetTilePos(float4 PixelPos)
{
	return uint2(
		CAST(uint, PixelPos.x) / TILED_LIGHT_GRID_WIDTH,
		CAST(uint, PixelPos.y) / TILED_LIGHT_GRID_HEIGHT
	);
}

#endif
