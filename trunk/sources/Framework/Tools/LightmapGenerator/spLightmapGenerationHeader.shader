/*
 * Lightmap generation shader header file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

/* === Macros === */

#define ID_NONE						0xFFFFFFFF
#define EPSILON						0.0001

#define PLANE_NORMAL(Plane)			((Plane).xyz)
#define PLANE_DISTANCE(Plane)		((Plane).w)

#define SPHERE_POINT(Sphere)		((Sphere).xyz)
#define SPHERE_INV_RADIUS(Sphere)	((Sphere).w)

#define SPlane						float4

#define MAX_STACK_SIZE				64

/*#define THREAD_GROUP_NUM_X			8
#define THREAD_GROUP_NUM_Y			8
#define THREAD_GROUP_SIZE			(THREAD_GROUP_NUM_X * THREAD_GROUP_NUM_Y)*/

#define LIGHT_DIRECTIONAL			0
#define LIGHT_POINT					1
#define LIGHT_SPOT					2

#define KDTREE_XAXIS				0
#define KDTREE_YAXIS				1
#define KDTREE_ZAXIS				2

#ifndef MAX_NUM_RADIOSITY_RAYS
#	define MAX_NUM_RADIOSITY_RAYS	4096
#endif


/* === Structures === */

struct STriangle
{
	float3 A, B, C;
};

struct SRay
{
	float3 Origin, Direction;
};

struct SLine
{
	float3 Start, End;
};

struct SKDTreeNode
{
	/* Node construction */
	int Axis;
	float Distance;
	
	/* Content */
	uint TriangleStart;	//!< Index to 'TriangleIdList' or ID_NONE
	uint NumTriangles;	//!< Length in 'TriangleIdList' or 0
	
	/* Children pointers */
	uint ChildIds[2];	//!< Index to 'NodeList' or ID_NONE
};

struct SLightSource
{
	int Type;
	float4 Sphere;
	float3 Color;
	float3 Direction;
	float SpotTheta;
	float SpotPhiMinusTheta;
};

struct SLightmapTexel
{
	float3 WorldPos;
	float3 Normal;
	float3 Tangent;
};

struct SIdStack
{
	uint Data[MAX_STACK_SIZE];
	uint Pointer;
};
