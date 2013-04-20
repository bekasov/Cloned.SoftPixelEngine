/*
 * Deferred shader header file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

/* === Macros === */

#ifndef MAX_LIGHTS
#	define MAX_LIGHTS           35
#endif
#ifndef MAX_EX_LIGHTS
#	define MAX_EX_LIGHTS        15
#endif

#ifndef NUM_JITTERD_OFFSETS
#	define NUM_JITTERD_OFFSETS	20
#endif

#define LIGHT_DIRECTIONAL       0
#define LIGHT_POINT             1
#define LIGHT_SPOT              2

#define AMBIENT_LIGHT_FACTOR    0.0//0.1 //!< Should be in the range [0.0 .. 1.0].
#define LIGHT_CUTOFF			0.0

#define MIN_VARIANCE            1.0
#define VPL_SINGULARITY_CLAMP	0.1

/* === Structures === */

struct SLight
{
    float4 PositionAndInvRadius;    //!< Position (xyz), Inverse Radius (w).
    float3 Color;                   //!< Light color (used for diffuse and specular).
	float Pad0;
    int Type;                       //!< 0 -> Directional light, 1 -> Point light, 2 -> Spot light.
    int ShadowIndex;                //!< Shadow map layer index.
	int UsedForLightmaps;		    //!< Specifies whether this light is used for lightmaps or not.
	int Pad2;
};

struct SLightEx
{
    float4x4 ViewProjection;    //!< Spot-/ directional view-projection matrix.
	#ifdef GLOBAL_ILLUMINATION
	float4x4 InvViewProjection;	//!< Inverse view-projection matrix.
	#endif
    float3 Direction;          	//!< Spot-/ directional light direction.
	float Pad0;
    float SpotTheta;			//!< First spot cone angle (in radian).
    float SpotPhiMinusTheta;	//!< Second minus first spot cone angle (in radian).
	float Pad1[2];
};
