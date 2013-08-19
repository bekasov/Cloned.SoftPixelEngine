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
	uint2 TileCount	: packoffset(c0);
	float2 GridSize	: packoffset(c0.z);
};

cbuffer BufferFrame : register(b1)
{
	float4x4 InvViewProjection	: packoffset(c0);	//!< Inverse view-projection matrix (without camera position).
	float3 ViewPosition			: packoffset(c4);	//!< Camera matrix (object-space).
	uint LightCount				: packoffset(c4.w);
	float4 NearPlane			: packoffset(c5);
	float4 FarPlane				: packoffset(c6);
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

void InsertLightIntoTile(uint2 TileIndex, uint LightID)
{
	/* Setup link node */
	SLightNode Link;
	Link.LightID = LightID;
	
	/* Insert light into list */
	uint LinkID = TileLightIndexList.IncrementCounter();
	
	InterlockedExchange(
		LightGrid[TileIndex.y * TileCount.x + TileIndex.x],
		LinkID,
		Link.Next
	);
	
	TileLightIndexList[LinkID] = Link;
}

void VecFrustum(inout float2 v)
{
	v = (v - 0.5) * 2.0;
}

float3 ComputeViewRay(float2 Pos)
{
	float4 ViewRay = float4(Pos.x, 1.0 - Pos.y, 1.0, 1.0);
	
	VecFrustum(ViewRay.xy);
	ViewRay = mul(InvViewProjection, ViewRay);
	
	return normalize(ViewRay.xyz);
}


/* === Main compute shader === */

[numthreads(THREAD_GROUP_NUM_X, THREAD_GROUP_NUM_Y, 1)]
void ComputeMain(uint3 Id : SV_GroupID, uint3 LocalId : SV_GroupThreadID)
{
	/* Build frustum for the current tile */
	float2 InvTileCount = float2(
		1.0 / (float)TileCount.x,
		1.0 / (float)TileCount.y
	);
	
	float4 ViewArea = float4(
		(float)(Id.x) * InvTileCount.x,
		(float)(Id.y) * InvTileCount.y,
		(float)(Id.x + 1) * InvTileCount.x,
		(float)(Id.y + 1) * InvTileCount.y
	);
	
	float3 LT = ComputeViewRay(ViewArea.xy);
	float3 RT = ComputeViewRay(ViewArea.zy);
	float3 RB = ComputeViewRay(ViewArea.zw);
	float3 LB = ComputeViewRay(ViewArea.xw);
	
	SFrustum Frustum;
	BuildFrustum(Frustum, LT, RT, RB, LB);
	
	/* Get start and step index */
	uint Start = LocalId.y*THREAD_GROUP_NUM_X + LocalId.x;
	
    //#define _CULL_
    #ifdef _CULL_
    if ( PLANE_DISTANCE(Frustum.Left) > 0.0 &&
         PLANE_DISTANCE(Frustum.Right) > 0.0 &&
         PLANE_DISTANCE(Frustum.Top) > 0.0 &&
         PLANE_DISTANCE(Frustum.Bottom) > 0.0 )
    {
    #endif

	/* Insert all tile effecting lights into the list */
	for (uint i = Start; i < LightCount; i += THREAD_GROUP_SIZE)
	{
		if (CheckSphereFrustumIntersection(PointLightsPositionAndRadius[i], Frustum))
			InsertLightIntoTile(Id.xy, i);
	}

    #ifdef _CULL_
    }
    #endif
}


/* === Initialization compute shader === */

[numthreads(1, 1, 1)]
void ComputeInitMain(uint3 Id : SV_GroupID)
{
	LightGrid[Id.y * TileCount.x + Id.x] = EOL;
}
