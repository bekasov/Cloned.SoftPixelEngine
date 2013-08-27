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

//#define USE_DEPTH_EXTENT
#ifdef USE_DEPTH_EXTENT
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
	float3 ViewPosition			: packoffset(c4);	//!< Camera position (object-space).
	uint NumLights				: packoffset(c4.w);
	float4 NearPlane			: packoffset(c5);
	float4 FarPlane				: packoffset(c6);
	#ifdef USE_DEPTH_EXTENT
	float4x4 ViewMatrix			: packoffset(c7);	//!< View matrix (view-space).
	#endif
};

//#define _DEB_USE_LIGHT_TEXBUFFER_
#ifdef _DEB_USE_LIGHT_TEXBUFFER_

Buffer<float4> PointLightsPositionAndRadius : register(t0);

#else

cbuffer BufferLight : register(b2)
{
	float4 PointLightsPositionAndRadius[MAX_LIGHTS];
};

#endif

#ifdef USE_DEPTH_EXTENT

Texture2D DepthTexture : register(t1);

groupshared uint ZMax;
groupshared uint ZMin;

#endif

//#define _DEB_USE_GROUP_SHARED_
#ifdef _DEB_USE_GROUP_SHARED_
groupshared uint LocalLightList[MAX_LIGHTS];
groupshared uint LocalLightCounter;
#endif

RWBuffer<uint> LightGrid : register(u0);
RWStructuredBuffer<SLightNode> TileLightIndexList : register(u1);


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
	
	return
		/* Check if the sphere's origin is not too distant from the frustum planes */
		ViewSpherePos.z > Near - SPHERE_RADIUS(Sphere) &&
		ViewSpherePos.z < Far + SPHERE_RADIUS(Sphere) &&
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

void InsertLightIntoTile(uint TileIndex, uint LightID)
{
	/* Setup link node */
	SLightNode Link;
	Link.LightID = LightID;
	
	/* Insert light into list */
	uint LinkID = TileLightIndexList.IncrementCounter();
	
	InterlockedExchange(
		LightGrid[TileIndex],
		LinkID,
		Link.Next
	);
	
	TileLightIndexList[LinkID] = Link;
}

#ifdef _DEB_USE_GROUP_SHARED_

void InsertLightIntoLocalList(uint LightID)
{
	/* Increment group shared light counter */
	uint DestIndex = 0;
	InterlockedAdd(LocalLightCounter, 1, DestIndex);
	
	/* Insert light index into group shared list */
	if (DestIndex < MAX_LIGHTS)
		LocalLightList[DestIndex] = LightID;
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
	
	//return normalize(ViewRay.xyz);
	return ViewRay.xyz;
}

#ifdef USE_DEPTH_EXTENT

float3 ProjectViewRay(uint2 PixelPos, float Depth)
{
	float4 ViewRay = float4(
		((float)PixelPos.x) * InvResolution.y,
		1.0 - ((float)PixelPos.y) * InvResolution.x,
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
	float PixelDepth = DepthTexture.Load(uint3(PixelPos.x, PixelPos.y, 0)).x;
	
	float3 ViewPos = ProjectViewRay(PixelPos, PixelDepth);
	
	/* Compute new depth extents */
	uint Z = asuint(ViewPos.z);
	
	/*if (PixelDepth != 1.0)
	{
		InterlockedMax(ZMax, Z);
		InterlockedMin(ZMin, Z);
	}*/
}

#endif


/* === Main compute shader === */

[numthreads(THREAD_GROUP_NUM_X, THREAD_GROUP_NUM_Y, 1)]
void ComputeMain(
	uint3 GroupId : SV_GroupID,
	uint3 LocalId : SV_GroupThreadID,
	uint3 GlobalId : SV_DispatchThreadID,
	uint LocalIndex : SV_GroupIndex)
{
	#ifdef USE_DEPTH_EXTENT
	
	/* Initialize depth extents */
	/*if (LocalIndex == 0)
	{
		ZMax = 0;
		ZMin = 0xFFFFFFFF;
	}
	GroupMemoryBarrierWithGroupSync();*/
	
	/* Compute min- and max depth extent */
	for (uint j = 0; j < DEPTH_EXTENT_SIZE; ++j)
	{
		ComputeMinMaxExtents(
			uint2(
				GlobalId.x * DEPTH_EXTENT_NUM_X + (j % DEPTH_EXTENT_NUM_X),
				GlobalId.y * DEPTH_EXTENT_NUM_Y + (j / DEPTH_EXTENT_NUM_X)
			)
		);
	}
	GroupMemoryBarrierWithGroupSync();
	
	/* Get final min- and max depth extens as floats */
	float Near = 0.0;//asfloat(ZMin);
	float Far = 0.0;//asfloat(ZMax);
	
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
	
	/* Get start and step index */
	uint Start = LocalId.y * THREAD_GROUP_NUM_X + LocalId.x;
	
	uint TileIndex = GroupId.y * NumTiles.x + GroupId.x;
	
    //#define _CULL_
    #ifdef _CULL_
    if ( PLANE_DISTANCE(Frustum.Left) > 0.0 &&
         PLANE_DISTANCE(Frustum.Right) > 0.0 &&
         PLANE_DISTANCE(Frustum.Top) > 0.0 &&
         PLANE_DISTANCE(Frustum.Bottom) > 0.0 )
    {
    #endif

	/* Insert all tile effecting lights into the list */
	for (uint i = Start; i < NumLights; i += THREAD_GROUP_SIZE)
	{
		#ifdef USE_DEPTH_EXTENT
		if (CheckSphereFrustumIntersection(PointLightsPositionAndRadius[i], Frustum, Near, Far))
		#else
		if (CheckSphereFrustumIntersection(PointLightsPositionAndRadius[i], Frustum))
		#endif
		{
			InsertLightIntoTile(TileIndex, i);
		}
	}

    #ifdef _CULL_
    }
    #endif
}


/* === Initialization compute shader === */

[numthreads(1, 1, 1)]
void ComputeInitMain(uint3 Id : SV_GroupID)
{
	LightGrid[Id.y * NumTiles.x + Id.x] = EOL;
}
