/*
 * GBuffer pixel shader main file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef NORMAL_MAPPING

/* Sample diffuse color */
DiffuseAndSpecular = tex2D(DiffuseMap, TexCoord);

#   ifdef HAS_SPECULAR_MAP
/* Sample specular color */
DiffuseAndSpecular.a = tex2D(SpecularMap, TexCoord).r * SpecularFactor;
#   endif

NormalAndDepth.xyz = Normal;

#else

#   ifdef PARALLAX_MAPPING
if (!EnablePOM)
{
#   endif
	
	/* Sample diffuse color */
	DiffuseAndSpecular = tex2D(DiffuseMap, TexCoord);
	
	/* Perform alpha-test clipping */
	clip(DiffuseAndSpecular.a - 0.5);
	
	/* Sample normal color */
	NormalAndDepth.xyz = tex2D(NormalMap, TexCoord).rgb;
	
	#ifdef HAS_SPECULAR_MAP
	/* Sample specular color */
	DiffuseAndSpecular.a = tex2D(SpecularMap, TexCoord).r * SpecularFactor;
	#endif
	
#   ifdef PARALLAX_MAPPING
}
else
{
	/* Compute parallax-occlusion displacement */
	float ParallaxLimit = (length(ViewVertexDirAndDepth.xy) / ViewVertexDirAndDepth.z) * HeightMapScale;
	
	float2 Offset       = normalize(-ViewVertexDirAndDepth.xy) * ParallaxLimit;
	
	float Interpolation = saturate(ParallaxViewRange / ViewVertexDirAndDepth.w);
	int NumSamples      = int(mix(float(MinSamplesPOM), float(MaxSamplesPOM), Interpolation));
	
	float StepSize      = 1.0 / float(NumSamples);
	
	float2 dx           = ddx(TexCoord);
	float2 dy           = ddy(TexCoord);
	
	float2 OffsetStep   = Offset * StepSize;
	
	float2 CurOffset    = 0.0;
	float2 PrevOffset   = 0.0;
	float CurSample     = 0.0;
	float PrevSample    = 0.0;
	
	float StepHeight    = 1.0;
	int SampleIndex     = 0;
	
	while (SampleIndex < NumSamples)
	{
		#ifdef NORMALMAP_XYZ_H
		CurSample = tex2DGrad(NormalMap, TexCoord + CurOffset, dx, dy).a;
		#else
		CurSample = tex2DGrad(HeightMap, TexCoord + CurOffset, dx, dy).r;
		#endif
		
		if (CurSample > StepHeight)
		{
			float Ua = (PrevSample - (StepHeight + StepSize)) / (StepSize + (CurSample - PrevSample));
			CurOffset = PrevOffset + OffsetStep * Ua;
			break;
		}
		else
		{
			++SampleIndex;
			StepHeight -= StepSize;
			PrevOffset = CurOffset;
			CurOffset += OffsetStep;
			PrevSample = CurSample;
		}
	}
	
	/* Sample diffuse color */
	DiffuseAndSpecular = tex2DGrad(DiffuseMap, TexCoord + CurOffset, dx, dy);
	
	/* Sample normal color */
	NormalAndDepth.xyz = tex2DGrad(NormalMap, TexCoord + CurOffset, dx, dy).rgb;
	
	/* Perform alpha-test clipping */
	//clip(DiffuseAndSpecular.a - 0.5);
	
	#ifdef HAS_SPECULAR_MAP
	/* Sample specular color */
	DiffuseAndSpecular.a = tex2DGrad(SpecularMap, TexCoord + CurOffset, dx, dy).r * SpecularFactor;
	#endif
}

#   endif // /PARALLAX_MAPPING

/* Transform final normal vector into tangent-space */
NormalAndDepth.xyz = NormalAndDepth.xyz * 2.0 - 1.0;
MUL_NORMAL(NormalAndDepth.xyz);

#endif // /NORMAL_MAPPING

#ifndef HAS_SPECULAR_MAP
DiffuseAndSpecular.a = SpecularFactor;
#endif

/* Compute depth distance between view position and pixel position */
NormalAndDepth.a = distance(ViewPosition, WorldPos);
