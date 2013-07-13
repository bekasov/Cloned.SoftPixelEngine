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

#define AMBIENT_LIGHT_FACTOR    0.0/*0.1*/	//!< Should be in the range [0.0 .. 1.0].
#define LIGHT_CUTOFF			0.0

#define MIN_VARIANCE            1.0
#define EOL						0xFFFFFFFF	//!< Id for 'end-of-linked-list'

#define VPL_SINGULARITY_CLAMP	0.1
#define VPL_COUNT				100

#ifdef TILED_SHADING
#	ifndef TILED_LIGHT_GRID_NUM_X
#		define TILED_LIGHT_GRID_NUM_X	8
#	endif
#	ifndef TILED_LIGHT_GRID_NUM_Y
#		define TILED_LIGHT_GRID_NUM_Y	8
#	endif
#	ifndef TILED_LIGHT_GRID_WIDTH
#		define TILED_LIGHT_GRID_WIDTH	32
#	endif
#	ifndef TILED_LIGHT_GRID_HEIGHT
#		define TILED_LIGHT_GRID_HEIGHT	32
#	endif
#endif

/* === Structures === */

struct SLightNode
{
	uint LightID;	//!< SLight index.
	uint Next;		//!< Next SLightNode index. 'EOL' if end of linked list.
};

#if 0

//!TODO! -> use this for 'StructuredBuffer<SLight> LightList' in future
struct SLight
{
	float4 PositionAndRadius;
	
	float3 Color;
	float SpotTheta;
	
	float3 Direction;
	float SpotPhiMinusTheta;
	
	float4x4 ViewProjection;
	float4x4 InvViewProjection;
	
	int Type;
	int ShadowMapIndex;
	int UsedForLightmaps;
};

#endif

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
