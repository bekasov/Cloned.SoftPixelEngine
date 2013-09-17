/*
 * Light grid D3D11 compute shader file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "spDeferredShaderHeader.shader"

/*
 * ======= Compute shader: =======
 */

/* === Macros === */

#define PLANE_NORMAL(p)		p.xyz
#define PLANE_DISTANCE(p)	p.w
#define SPHERE_POINT(s)		s.xyz
#define SPHERE_RADIUS(s)	s.w

#define THREAD_GROUP_NUM_X	8
#define THREAD_GROUP_NUM_Y	8
#define THREAD_GROUP_SIZE	(THREAD_GROUP_NUM_X * THREAD_GROUP_NUM_Y)

#define LIGHT_GRID_SIZE		float2(32.0, 32.0)

#define USE_DEPTH_EXTENT
#ifdef USE_DEPTH_EXTENT
//#	define _DEB_DEPTH_EXTENT_
#	define DEPTH_EXTENT_NUM_X	4
#	define DEPTH_EXTENT_NUM_Y	4
#	define DEPTH_EXTENT_SIZE	(DEPTH_EXTENT_NUM_X * DEPTH_EXTENT_NUM_Y)
#endif


/* === Structures === */

//! Partially frustum (near- and far plane removed)
struct SFrustum
{
	float4 Left;
	float4 Right;
	float4 Top;
	float4 Bottom;
};

/**
Plane -> float4 (xyz for normal and w for distance)
Sphere -> float4 (xyz for position and w for radius)
*/


/* === Buffers === */

cbuffer BufferMain : register(b0)
{
	uint2 NumTiles			: packoffset(c0);
	float2 InvNumTiles		: packoffset(c0.z);
	float2 InvResolution	: packoffset(c1);
	float2 Pad0				: packoffset(c1.z);
};

cbuffer BufferFrame : register(b1)
{
	float4x4 InvViewProjection	: packoffset(c0);	//!< Inverse view-projection matrix (without camera position).
	float4x4 ViewMatrix			: packoffset(c4);	//!< View matrix (view-space).
	float4 NearPlane			: packoffset(c8);
	float4 FarPlane				: packoffset(c9);
	float3 ViewPosition			: packoffset(c10);	//!< Camera position (object-space).
	uint NumLights				: packoffset(c10.w);
};

//#define _DEB_USE_LIGHT_TEXBUFFER_
#ifdef _DEB_USE_LIGHT_TEXBUFFER_

#	ifdef USE_DEPTH_EXTENT
Buffer<float4> PointLightsPositionAndRadius : register(t1);
#	else
Buffer<float4> PointLightsPositionAndRadius : register(t0);
#	endif

#else

cbuffer BufferLight : register(b2)
{
	float4 PointLightsPositionAndRadius[MAX_LIGHTS];
};

#endif

#ifdef USE_DEPTH_EXTENT

Texture2D<half4> DepthTexture : register(t0);

#	ifdef _DEB_DEPTH_EXTENT_
RWBuffer<float2> _debDepthExt_In_ : register(u3);
#	endif

groupshared uint ZMax;
groupshared uint ZMin;

#endif

//#define _DEB_USE_GROUP_SHARED_
//#define _DEB_USE_GROUP_SHARED_OPT_
#ifdef _DEB_USE_GROUP_SHARED_

groupshared uint LocalLightIdList[MAX_LIGHTS];
groupshared uint LocalLightCounter;
groupshared uint LightIndexStart;

#endif

RWBuffer<uint> LightGrid : register(u0);

#if defined(_DEB_USE_GROUP_SHARED_) && defined(_DEB_USE_GROUP_SHARED_OPT_)
RWBuffer<uint> GlobalLightIdList : register(u1);
#else
RWStructuredBuffer<SLightNode> GlobalLightIdList : register(u1);
#endif

#ifdef _DEB_USE_GROUP_SHARED_
RWBuffer<uint> GlobalLightCounter : register(u2);
#endif


/* === Functions === */

/**
Normally one calculates a plane's normal with the following equation:
n = || (b - a) x (c - a) ||
In this case we already have the directions (b - a) and (c - a)
through the parameters U and V.
P is the origin point of the plane (a in the upper equation).
*/
void BuildPlane(out float4 Plane, float3 P, float3 U, float3 V)
{
	PLANE_NORMAL(Plane) = normalize(cross(U, V));
	PLANE_DISTANCE(Plane) = dot(PLANE_NORMAL(Plane), P);
}

void BuildFrustum(out SFrustum Frustum, float3 LT, float3 RT, float3 RB, float3 LB)
{
	BuildPlane(Frustum.Left,	ViewPosition, LB, LT);
	BuildPlane(Frustum.Right,	ViewPosition, RT, RB);
	BuildPlane(Frustum.Top,		ViewPosition, LT, RT);
	BuildPlane(Frustum.Bottom,	ViewPosition, RB, LB);
}

float GetPlanePointDistance(float4 Plane, float3 Point)
{
	return dot(PLANE_NORMAL(Plane), Point) - PLANE_DISTANCE(Plane);
}

#ifdef USE_DEPTH_EXTENT

bool CheckSphereFrustumIntersection(float4 Sphere, SFrustum Frustum, float Near, float Far)
{
	/* Get sphere position in view space */
	float3 ViewSpherePos = mul(ViewMatrix, float4(SPHERE_POINT(Sphere), 1.0)).xyz;
	float SphereDist = length(ViewSpherePos);
	
	#if 1//!!!
	
	float4 TempNearPlane = NearPlane;
	float4 TempFarPlane = FarPlane;
	
	PLANE_DISTANCE(TempNearPlane) -= Near;
	PLANE_DISTANCE(TempFarPlane) = -PLANE_DISTANCE(NearPlane) + Far;
	
	#endif
	
	return
		/* Check if the sphere's origin is not too distant from the frustum planes */
		#if 0
		GetPlanePointDistance(NearPlane,		SPHERE_POINT(Sphere)) <= SPHERE_RADIUS(Sphere) &&
		#elif 0
		SphereDist > Near - SPHERE_RADIUS(Sphere) &&
		SphereDist < Far + SPHERE_RADIUS(Sphere) &&
		#else
		GetPlanePointDistance(TempNearPlane,	SPHERE_POINT(Sphere)) <= SPHERE_RADIUS(Sphere) &&
		GetPlanePointDistance(TempFarPlane,		SPHERE_POINT(Sphere)) <= SPHERE_RADIUS(Sphere) &&
		#endif
		GetPlanePointDistance(Frustum.Left,		SPHERE_POINT(Sphere)) <= SPHERE_RADIUS(Sphere) &&
		GetPlanePointDistance(Frustum.Right,	SPHERE_POINT(Sphere)) <= SPHERE_RADIUS(Sphere) &&
		GetPlanePointDistance(Frustum.Top,		SPHERE_POINT(Sphere)) <= SPHERE_RADIUS(Sphere) &&
		GetPlanePointDistance(Frustum.Bottom,	SPHERE_POINT(Sphere)) <= SPHERE_RADIUS(Sphere);
}

#else

bool CheckSphereFrustumIntersection(float4 Sphere, SFrustum Frustum)
{
	return
		/* Check if the sphere's origin is not too distant from the frustum planes */
		GetPlanePointDistance(NearPlane,		SPHERE_POINT(Sphere)) <= SPHERE_RADIUS(Sphere) &&
		GetPlanePointDistance(Frustum.Left,		SPHERE_POINT(Sphere)) <= SPHERE_RADIUS(Sphere) &&
		GetPlanePointDistance(Frustum.Right,	SPHERE_POINT(Sphere)) <= SPHERE_RADIUS(Sphere) &&
		GetPlanePointDistance(Frustum.Top,		SPHERE_POINT(Sphere)) <= SPHERE_RADIUS(Sphere) &&
		GetPlanePointDistance(Frustum.Bottom,	SPHERE_POINT(Sphere)) <= SPHERE_RADIUS(Sphere);
}

#endif

#ifdef _DEB_USE_GROUP_SHARED_

void InsertLightIntoLocalList(uint LightID)
{
	/* Increment group shared light counter */
	uint DestIndex = 0;
	InterlockedAdd(LocalLightCounter, 1, DestIndex);
	
	/* Insert light index into group shared list */
	LocalLightIdList[DestIndex] = LightID;
}

#else

void InsertLightIntoTile(uint TileIndex, uint LightID)
{
	/* Setup link node */
	SLightNode Link;
	Link.LightID = LightID;
	
	/* Insert light into list */
	uint LinkID = GlobalLightIdList.IncrementCounter();
	
	InterlockedExchange(
		LightGrid[TileIndex],
		LinkID,
		Link.Next
	);
	
	GlobalLightIdList[LinkID] = Link;
}

#endif

void VecFrustum(inout float2 v)
{
	v = (v - 0.5) * 2.0;
}

float3 ProjectViewRay(float2 Pos)
{
	float4 ViewRay = float4(Pos.x, 1.0 - Pos.y, 1.0, 1.0);
	
	VecFrustum(ViewRay.xy);
	ViewRay = mul(InvViewProjection, ViewRay);
	
	return ViewRay.xyz;
}

#ifdef USE_DEPTH_EXTENT

float3 ProjectViewRay(uint2 PixelPos, float Depth)
{
	float4 ViewRay = float4(
		      ((float)PixelPos.x) * InvResolution.x,
		1.0 - ((float)PixelPos.y) * InvResolution.y,
		Depth,
		1.0
	);
	
	VecFrustum(ViewRay.xy);
	ViewRay = mul(InvViewProjection, ViewRay);
	
	return ViewRay.xyz;
}

void ComputeMinMaxExtents(uint2 PixelPos)
{
	/* Project view ray from pixel position */
	float PixelDepth = (float)DepthTexture[PixelPos].w;
	
	/* Convert linear depth into perspective depth */
	#if 1//!!!TODO: optimize this!!!
	float3 WorldPos = ViewPosition + normalize(ProjectViewRay(PixelPos, 1.0)) * (float3)PixelDepth;
	PixelDepth = -GetPlanePointDistance(NearPlane, WorldPos);
	#endif
	
	/* Compute new depth extents */
	uint Z = asuint(PixelDepth);
	
	if (PixelDepth != 1.0)
	{
		InterlockedMax(ZMax, Z);
		InterlockedMin(ZMin, Z);
	}
}

#endif


/* === Main compute shader === */

[numthreads(THREAD_GROUP_NUM_X, THREAD_GROUP_NUM_Y, 1)]
void ComputeMain(
	uint3 GroupId : SV_GroupID,
	//uint3 LocalId : SV_GroupThreadID,
	uint3 GlobalId : SV_DispatchThreadID,
	uint LocalIndex : SV_GroupIndex)
{
	#ifdef USE_DEPTH_EXTENT
	
	/* Initialize depth extents */
	if (LocalIndex == 0)
	{
		ZMax = 0;
		ZMin = 0xFFFFFFFF;
		#ifdef _DEB_USE_GROUP_SHARED_
		LocalLightCounter = 0;
		#endif
	}
	GroupMemoryBarrierWithGroupSync();
	
	/* Compute min- and max depth extent */
	[unroll]
	for (uint k = 0; k < DEPTH_EXTENT_SIZE; ++k)
	{
		ComputeMinMaxExtents(
			uint2(
				GlobalId.x * DEPTH_EXTENT_NUM_X + (k % DEPTH_EXTENT_NUM_X),
				GlobalId.y * DEPTH_EXTENT_NUM_Y + (k / DEPTH_EXTENT_NUM_X)
			)
		);
	}
	GroupMemoryBarrierWithGroupSync();
	
	/* Get final min- and max depth extens as floats */
	float Near = asfloat(ZMin);
	float Far = asfloat(ZMax);
	
	/* Convert to near and far values to clipping planes */
	#	if 0//???
	Near = ProjectViewRay(GroupId.xy * uint2(32, 32), Near).z;
	Far = ProjectViewRay(GroupId.xy * uint2(32, 32), Far).z;
	#	endif
	
	#endif
	
	/* Build frustum for the current tile */
	float4 ViewArea = float4(
		(float)(GroupId.x) * InvNumTiles.x,
		(float)(GroupId.y) * InvNumTiles.y,
		(float)(GroupId.x + 1) * InvNumTiles.x,
		(float)(GroupId.y + 1) * InvNumTiles.y
	);
	
	float3 LT = ProjectViewRay(ViewArea.xy);
	float3 RT = ProjectViewRay(ViewArea.zy);
	float3 RB = ProjectViewRay(ViewArea.zw);
	float3 LB = ProjectViewRay(ViewArea.xw);
	
	SFrustum Frustum;
	BuildFrustum(Frustum, LT, RT, RB, LB);
	
	/* Get tile index */
	uint TileIndex = GroupId.y * NumTiles.x + GroupId.x;
	
	#	ifdef _DEB_DEPTH_EXTENT_
	if (LocalIndex == 0)
		_debDepthExt_In_[TileIndex] = float2(Near, Far);
	#	endif
	
	#if defined(_DEB_USE_GROUP_SHARED_) && !defined(USE_DEPTH_EXTENT)
	/* Initialize local list counter */
	if (LocalIndex == 0)
		LocalLightCounter = 0;
	GroupMemoryBarrierWithGroupSync();
	#endif
	
	/* Insert all tile effecting lights into the list */
	for (uint i = LocalIndex; i < NumLights; i += THREAD_GROUP_SIZE)
	{
		#ifdef USE_DEPTH_EXTENT
		if (CheckSphereFrustumIntersection(PointLightsPositionAndRadius[i], Frustum, Near, Far))
		#else
		if (CheckSphereFrustumIntersection(PointLightsPositionAndRadius[i], Frustum))
		#endif
		{
			#ifdef _DEB_USE_GROUP_SHARED_
			InsertLightIntoLocalList(i);
			#else
			InsertLightIntoTile(TileIndex, i);
			#endif
		}
	}
	
	#ifdef _DEB_USE_GROUP_SHARED_
	
	/* Setup light-grid index for current tile */
	GroupMemoryBarrierWithGroupSync();
	
	uint StartOffset = 0;
	uint NumLights = LocalLightCounter;
	
	if (LocalIndex == 0)
	{
		if (NumLights > 0)
			InterlockedAdd(GlobalLightCounter[0], NumLights + 1, StartOffset);
		
		#ifdef _DEB_USE_GROUP_SHARED_OPT_
		LightGrid[TileIndex] = StartOffset;
		GlobalLightIdList[StartOffset + NumLights] = EOL;
		#else
		LightGrid[TileIndex] = (NumLights > 0 ? StartOffset : EOL);
		#endif
		
		LightIndexStart = StartOffset;
	}
	GroupMemoryBarrierWithGroupSync();
	
	/* Insert the local list list into the global light list */
	StartOffset = LightIndexStart;
	
	for (uint j = LocalIndex; j < NumLights; j += THREAD_GROUP_SIZE)
	{
		#ifndef _DEB_USE_GROUP_SHARED_OPT_
		SLightNode Link;
		Link.LightID = LocalLightIdList[j];
		Link.Next = (j + 1 < NumLights ? (StartOffset + j + 1) : EOL);
		
		GlobalLightIdList[StartOffset + j] = Link;
		#else
		GlobalLightIdList[StartOffset + j] = LocalLightIdList[j];
		#endif
	}
	
	#endif
}


/* === Initialization compute shader === */

[numthreads(1, 1, 1)]
void ComputeInitMain(uint3 Id : SV_GroupID)
{
	/* Clear light-grid offsets */
	LightGrid[Id.y * NumTiles.x + Id.x] = EOL;
	
	#ifdef _DEB_USE_GROUP_SHARED_
	
	/* Clear global index counter */
	uint GlobalHashId = Id.x + Id.y + Id.z;
	if (GlobalHashId == 0)
		GlobalLightCounter[0] = 0;
	
	#endif
}
