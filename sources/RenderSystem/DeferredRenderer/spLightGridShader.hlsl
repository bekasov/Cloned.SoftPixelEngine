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
	uint2 TileCount;
	uint2 Pad0;
	float2 GridSize;
	//float4 NearPlane;
	//float4 FarPlane;
};

cbuffer BufferFrame : register(b1)
{
	float4x4 CameraMatrix;		//!< Camera matrix (object-space).
	float4x4 InvViewProjection;	//!< Inverse view-projection matrix (without camera position).
	uint LightCount;
};

cbuffer BufferLight : register(b2)
{
	float4 PointLightsPositionAndRadius[MAX_LIGHTS];
};

//StructuredBuffer<SFrustum> TileFrustums : register(t0);

RWBuffer<uint> LightGrid : register(u0);
RWStructuredBuffer<SLightNode> TileLightIndexList : register(u1);


/* === Functions === */

float4 BuildPlane(float3 A, float3 B, float3 C)
{
	float4 Plane;
	PLANE_NORMAL(Plane) = normalize(cross(B - A, C - A));
	PLANE_DISTANCE(Plane) = dot(PLANE_NORMAL(Plane), A);
	return Plane;
}

void BuildFrustum(out SFrustum Frustum, float3 LT, float3 RT, float3 RB, float3 LB)
{
	float3 ViewPos = mul(CameraMatrix, float4(0.0, 0.0, 0.0, 1.0)).xyz;//CameraMatrix[3].xyz;
	
	LT += ViewPos;
	RT += ViewPos;
	RB += ViewPos;
	LB += ViewPos;
	
	Frustum.Left	= BuildPlane(ViewPos, LB, LT);
	Frustum.Right	= BuildPlane(ViewPos, RT, RB);
	Frustum.Top		= BuildPlane(ViewPos, LT, RT);
	Frustum.Bottom	= BuildPlane(ViewPos, RB, LB);
}

float GetPlanePointDistance(float4 Plane, float3 Point)
{
	return (dot(PLANE_NORMAL(Plane), Point) - Plane.w) / dot(PLANE_NORMAL(Plane), PLANE_NORMAL(Plane));
}

bool CheckSphereFrustumIntersection(float4 Sphere, SFrustum Frustum)
{
	return
		GetPlanePointDistance(Frustum.Left,		Sphere.xyz) <= SPHERE_RADIUS(Sphere) &&
		GetPlanePointDistance(Frustum.Right,	Sphere.xyz) <= SPHERE_RADIUS(Sphere) &&
		GetPlanePointDistance(Frustum.Top,		Sphere.xyz) <= SPHERE_RADIUS(Sphere) &&
		GetPlanePointDistance(Frustum.Bottom,	Sphere.xyz) <= SPHERE_RADIUS(Sphere);
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

void VecFrustum(inout float4 v)
{
    v.x = (v.x - 0.5) * 2.0;
    v.y = (v.y - 0.5) * 2.0;
}

float3 ComputeViewRay(float2 Pos)
{
	float4 ViewRay = float4(Pos.x, 1.0 - Pos.y, 0.0, 1.0);
	
	VecFrustum(ViewRay);
	ViewRay = mul(InvViewProjection, ViewRay);
	
	return ViewRay.xyz;
}


/* === Main compute shader === */

[numthreads(1, 1, 1)]
void ComputeMain(uint3 Id : SV_DispatchThreadID)
{
	#if 1
	
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
	
	#else
	
	/* Get frustum for the current tile */
	//!TODO! -> this should be used! Only build frustums once on the CPU!
	SFrustum Frustum = TileFrustums[Id.y * TileCount.x + Id.x];
	
	//todo -> transform each light with the ViewMatrix
	
	#endif
	
	/* Insert all tile effecting lights into the list */
	for (uint i = 0; i < LightCount; ++i)
	{
		#if 1//!!!
		if (i > 500)
			break;
		#endif
		
		if (CheckSphereFrustumIntersection(PointLightsPositionAndRadius[i], Frustum))
			InsertLightIntoTile(Id.xy, i);
	}
}


/* === Initialization compute shader === */

[numthreads(1, 1, 1)]
void ComputeInitMain(uint3 Id : SV_DispatchThreadID)
{
	LightGrid[Id.y * TileCount.x + Id.x] = EOL;
}
