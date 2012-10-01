/*
 * Ray tracing OpenCL shader (01/10/2012)
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

/*
 * ======= Macros =======
 */

#define ROUNDING_ERROR 0.000001


/*
 * ======= Structures =======
 */

struct AABBox3D
{
	float3 Min, Max;
};

struct Ray3D
{
	float3 Point, Direction;
};

struct Line3D
{
	float3 Start, End;
};

struct Plane3D
{
	float3 Normal;
	float Distance;
};

struct Triangle3D
{
	float3 A, B, C;
};

struct KDTreeNode
{
	char Axis;				//!< 0 -> X Axis; 1 -> Y Axis; 2 -> Z Axis;
	float Distance;
	
	AABBox3D Box;
	
	int NearChildIndex;		//!< KDTreeNode array index for near child (-1 means no children)
	int FarChildIndex;		//!< KDTreeNode array index for far child (-1 means no children)
	
	int TriangleListIndex;	//!< Triangle array index
};


/*
 * ======= Functions =======
 */

bool CheckRayAABBOverlap1D(
	float start, float dir, float min, float max, float &enter, float &exit)
{
	/* Ray parallel to direction */
	if (Abs(dir) < ROUNDING_ERROR)
		return (start >= min && start <= max);
	
	/* Intersection parameters */
	float t0 = (min - start) / dir;
	float t1 = (max - start) / dir;
	
	/* Sort intersections */
	if (t0 > t1)
		Swap(t0, t1);
	
	/* Check if intervals are disjoint */
	if (t0 > exit || t1 < enter)
		return false;
	
	/* Reduce interval */
	if (t0 > enter)
		enter = t0;
	if (t1 < exit)
		exit = t1;

	return true;
}

bool CheckRayBoxOverlap(const Ray3D &Ray, const AABBox3D &Box)
{
	float enter = 0.0, exit = 1.0;
	
	if (!checkRayAABBOverlap1D(Ray.Point.x, Ray.Direction.x, Box.Min.x, Box.Max.x, enter, exit))
		return false;
	if (!checkRayAABBOverlap1D(Ray.Point.y, Ray.Direction.y, Box.Min.y, Box.Max.y, enter, exit))
		return false;
	if (!checkRayAABBOverlap1D(Ray.Point.z, Ray.Direction.z, Box.Min.z, Box.Max.z, enter, exit))
		return false;
	
	return true;
}

inline bool CheckKDTreeNode(const Ray3D &Ray, const KDTreeNode &Node)
{
	return CheckRayBoxOverlap(Ray, Node.Box);
}

inline bool IsKDTreeLeaf(const KDTreeNode &Node)
{
	return Node.NearChildIndex == -1;
}

void ComputePlane(const Triangle3D &Triangle, Plane3D &Plane)
{
	Plane.Normal = normalize(cross(Triangle.B - Triangle.A, Triangle.C - Triangle.A));
	Plane.Distance = dot(Plane.Normal, Triangle.A);
}

bool CheckRayPlaneIntersection(const Ray3D &Ray, const Plane3D &Plane)
{
	float t = (Plane.Distance - dot(Normal, Ray.Start)) / dot(Plane.Normal, Ray.Direction);
	
	if (t >= 0.0 && t <= 1.0)
	{
		Intersection = Ray.Point + Ray.Direction * t
		return true;
	}
	
	return false;
}

bool CheckRayTriangleIntersection(const Ray3D &Ray, const Triangle3D &Triangle, float3 &Intersection)
{
	float3 pa = Triangle.A - Ray.Point;
	float3 pb = Triangle.B - Ray.Point;
	float3 pc = Triangle.C - Ray.Point;
	
	/* Check if ray direction is inside the edges bc, ca and ab */
	Intersection.X = dot(pb, cross(Ray.Direction, pc));
	if (Intersection.X < 0.0)
		return false;
	
	Intersection.Y = dot(pc, cross(Ray.Direction, pa));
	if (Intersection.Y < 0.0)
		return false;
	
	Intersection.Z = dot(pa, cross(Ray.Direction, pb));
	if (Intersection.Z < 0.0)
		return false;
	
	/* Make ray-plane intersection test */
	Plane3D Plane;
	ComputePlane(Triangle, Plane);
	
	return CheckRayPlaneIntersection(Ray, Plane, Intersection);
}


/*
 * ======= Kernels =======
 */

__kernel void RenderRayTracing(
	int ImageWidth,
	int ImageHeight,
	__global float4* ImageBuffer,
	
	float4x4 ViewMatrix,
	
	uint TreeNodeCount,
	__global const KDTreeNode* TreeNodeList,
	
	uint TriangleCount,
	__global const uint* TriangleList,
	
	uint VertexCount,
	__global const float3* VertexList)
{
	/* Get image coordinate */
	int x = get_global_id(0);
	int y = get_global_id(1);
	
	/* Generate view ray */
	Ray3D Ray;
	
	Ray.Direction.x = float(x) / float(ImageWidth) - 0.5;
	Ray.Direction.y = float(y) / float(ImageHeight) - 0.5;
	Ray.Direction.z = 1.0;
	Ray.Direction = normalize(Ray.Direction);
	
	//... frustum
	
	/* Find all tree node leafs */
	const KDTreeNode* Node = TreeNodeList;
	
	while (Node)
	{
		if (CheckKDTreeNode(Ray, *Node))
		{
			if (IsKDTreeLeaf(*Node))
			{
				/* Process ray-tracing */
				
				
			}
			else
			{
				
				
				
			}
		}
		
		
	}
	
	
	
	
	
	
}


// ================================================================================
