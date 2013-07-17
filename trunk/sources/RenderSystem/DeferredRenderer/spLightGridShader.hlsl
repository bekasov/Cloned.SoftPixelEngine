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

cbuffer BufferLight : register(b2)
{
	float4 PointLightsPositionAndRadius[MAX_LIGHTS];
};

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

#if 1

float GetRayPointDistance(float3 Start, float3 Dir, float3 Point)
{
	/* Get closest point on ray */
	float Len = dot(Dir, Point - Start);
	
	//if (Len < 0.0)
	//	return 999999.0;
	
	float3 ClosestPoint = Start + Dir * Len;
	
	/* Return distance between closest and given point */
	return distance(ClosestPoint, Point);
}

#else

/**
The ray direction must be normalized. I've found the function here:
http://answers.unity3d.com/questions/344630/how-would-i-find-the-closest-vector3-point-to-a-gi.html
*/
float GetRayPointDistance(float3 Start, float3 Dir, float3 Point)
{
	float3 u = Point - Start;
	float3 v = u - Dir;
	return length(cross(u, v));
}

#endif

bool CheckSphereFrustumIntersection(
	float4 Sphere, SFrustum Frustum, float3 LT, float3 RT, float3 RB, float3 LB)
{
	return
		/* Check if the sphere's origin is not too distant from the frustum planes */
		GetPlanePointDistance(NearPlane,		SPHERE_POINT(Sphere)) <= SPHERE_RADIUS(Sphere) &&
		GetPlanePointDistance(Frustum.Left,		SPHERE_POINT(Sphere)) <= SPHERE_RADIUS(Sphere) &&
		GetPlanePointDistance(Frustum.Right,	SPHERE_POINT(Sphere)) <= SPHERE_RADIUS(Sphere) &&
		GetPlanePointDistance(Frustum.Top,		SPHERE_POINT(Sphere)) <= SPHERE_RADIUS(Sphere) &&
		GetPlanePointDistance(Frustum.Bottom,	SPHERE_POINT(Sphere)) <= SPHERE_RADIUS(Sphere) &&
		
		/* Check if the sphere's origin is not too distant from all four frustum rays */
		!(
			GetRayPointDistance(ViewPosition, LT, SPHERE_POINT(Sphere)) > SPHERE_RADIUS(Sphere) &&
			GetRayPointDistance(ViewPosition, RT, SPHERE_POINT(Sphere)) > SPHERE_RADIUS(Sphere) &&
			GetRayPointDistance(ViewPosition, RB, SPHERE_POINT(Sphere)) > SPHERE_RADIUS(Sphere) &&
			GetRayPointDistance(ViewPosition, LB, SPHERE_POINT(Sphere)) > SPHERE_RADIUS(Sphere)
		);
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
	float4 ViewRay = float4(Pos.x, 1.0 - Pos.y, 0.0, 1.0);
	
	VecFrustum(ViewRay.xy);
	ViewRay = mul(InvViewProjection, ViewRay);
	
	return normalize(ViewRay.xyz);
}


/* === Main compute shader === */

[numthreads(1, 1, 1)]
void ComputeMain(uint3 Id : SV_DispatchThreadID)
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
	
	/* Insert all tile effecting lights into the list */
	for (uint i = 0; i < LightCount; ++i)
	{
		if (CheckSphereFrustumIntersection(PointLightsPositionAndRadius[i], Frustum, LT, RT, RB, LB))
			InsertLightIntoTile(Id.xy, i);
	}
}


/* === Initialization compute shader === */

[numthreads(1, 1, 1)]
void ComputeInitMain(uint3 Id : SV_DispatchThreadID)
{
	LightGrid[Id.y * TileCount.x + Id.x] = EOL;
}
