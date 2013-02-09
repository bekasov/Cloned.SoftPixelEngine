/*
 * Ray tracing OpenCL shader (01/10/2012)
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

/*
 * ======= Macros =======
 */

#define EPSILON				0.000001
#define OMEGA				999999.0

#define TREENODE_STACK_SIZE	64


/*
 * ======= Enumerations =======
 */

enum LightModels
{
	LIGHT_DIRECTIONAL,
	LIGHT_POINT,
	LIGHT_SPOT,
};


/*
 * ======= Structures =======
 */

/* === Base structures === */

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

struct Sphere3D
{
	float3 Position;
	float Radius;
};


/* === Tree hierarchy structures === */

struct KDTreeNode
{
	char Axis;				//!< 0 -> X Axis; 1 -> Y Axis; 2 -> Z Axis;
	float Distance;
	
	AABBox3D Box;
	
	int NearChildIndex;		//!< KDTreeNode array index for near child (-1 means no children).
	int FarChildIndex;		//!< KDTreeNode array index for far child (-1 means no children).
	
	uint FirstTriangle;		//!< Triangle array start index.
	uint LastTriangle;		//!< Triangle array end index.
};

struct KDTreeNodeStack
{
	const KDTreeNode* Stack[TREENODE_STACK_SIZE];
	uint Pointer;
};


/* === Surface structures === */

struct LightSource
{
	int Model;			//!< Light model (enum LightModels).
	
	float3 Position;
	float3 Attn;
	float3 Color;
	float3 Direction;	//!< Light direction used for directional- and spot light.
	
	float Theta;
	float Phi;
};

struct SurfaceMaterial
{
	float4 Diffuse;
	float4 Ambient;
	float4 Specular;
	float4 Emission;
	float Shininess;
	float Reflection;
	float Refraction;
};

struct SurfaceVertex
{
	float3 Coord;
	float3 Normal;
	//float3 Tangent;
	//float3 Binormal;
	float2 TexCoord;
};

struct SurfaceSphere
{
	Sphere3D Sphere;
	SurfaceMaterial Material;
};

struct SurfacePlane
{
	Plane3D Plane;
	SurfaceMaterial Material;
};


/*
 * ======= Functions =======
 */

/* === Collision detection functions === */

bool CheckRayAABBOverlap1D(
	float start, float dir, float min, float max, float &enter, float &exit)
{
	/* Ray parallel to direction */
	if (Abs(dir) < EPSILON)
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

bool IntersectionTest(const Ray3D &Ray, float3 &Intersection)
{
	
	
	
	
	return false;
}


/* === Stack functions === */

void TreeNodePush(KDTreeNodeStack &NodeStack, const KDTreeNode* Node)
{
	if (NodeStack.Pointer < TREENODE_STACK_SIZE)
	{
		NodeStack.Stack[NodeStack.Pointer] = Node;
		++NodeStack.Pointer;
	}
}

const KDTreeNode* TreeNodePop(KDTreeNodeStack &NodeStack)
{
	if (NodeStack.Pointer > 0)
	{
		const KDTreeNode* Node = NodeStack.Stack[NodeStack.Pointer];
		--NodeStack.Pointer;
		return Node;
	}
	return 0;
}


/*
 * ======= Kernels =======
 */

__kernel void RenderRayTracing(
	/* Output image buffer */
	int ImageWidth,
	int ImageHeight,
	__global float4* ImageBuffer,
	
	/* View transformation */
	float4x4 ViewMatrix,
	
	/* Tree node hierarchy */
	uint TreeNodeCount,
	__global const KDTreeNode* TreeNodeList,
	
	/* Light sources */
	uint LightCount,
	__global const LightSource* LightSourceList,
	
	/* Index buffer */
	uint TriangleCount,
	__global const uint* TriangleList,
	
	/* Vertex buffer */
	uint VertexCount,
	__global const SurfaceVertex* VertexList,
	
	/* Sphere geometries */
	uint SphereCount,
	__global const SurfaceSphere* SphereList,
	
	/* Plane geometries */
	uint PlaneCount,
	__global const SurfacePlane* PlaneList)
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
	
	/* Varaibles for finding the nearest intersection distance to view */
	float Distance = 0.0, NearestDistance = OMEGA;
	float3 Intersection = 0.0, NearestIntersction = OMEGA;
	
	/* Make intersection tests with geometry */
	const KDTreeNode* Node = TreeNodeList;
	
	KDTreeNodeStack NodeStack;
	TreeNodePush(NodeStack, Node);
	
	while (Node)
	{
		if (IsKDTreeLeaf(*Node))
		{
			/* Make intersection tests with all triangles of the current tree node leaf */
			for (uint i = Node->FirstTriangle; i <= Node->LastTriangle; i += 3)
			{
				/* Get indices and triangle vertices */
				uint v0 = TriangleList[i    ];
				uint v1 = TriangleList[i + 1];
				uint v2 = TriangleList[i + 2];
				
				const SurfaceVertex* a = VertexList[v0];
				const SurfaceVertex* b = VertexList[v1];
				const SurfaceVertex* c = VertexList[v2];
				
				Triangle3D Triangle;
				
				Triangle.A = a->Coord;
				Triangle.B = b->Coord;
				Triangle.C = c->Coord;
				
				/* Make intersection test with current triangle */
				if (CheckRayTriangleIntersection(Ray, Triangle, Intersection))
				{
					Distance = length(Intersection);
					
					if (Distance < NearestDistance)
					{
						/* Found new nearest intersection */
						NearestDistance		= Distance;
						NearestIntersction	= Intersection;
					}
				}
			}
		}
		else
		{
			/* Traverse next child tree node(s) */
			const KDTreeNode* NearChild = TreeNodeList + Node->NearChildIndex;
			const KDTreeNode* FarChild = TreeNodeList + Node->FarChildIndex;
			
			if (CheckKDTreeNode(Ray, *NearChild))
				TreeNodePush(NodeStack, NearChild);
			else if (CheckKDTreeNode(Ray, *FarChild))
				TreeNodePush(NodeStack, FarChild);
			else
				continue;
		}
		
		/* Get current tree node on the stack */
		Node = TreeNodePop(NodeStack);
	}
	
	/* Make intersection tests with primitives (planes, spheres etc.) */
	for (uint i = 0; i < SphereCount; ++i)
	{
		const SurfaceSphere* Obj = SphereList[i];
		
		//...
	}
	
	for (uint i = 0; i < PlaneCount; ++i)
	{
		const SurfacePlane* Obj = PlaneList[i];
		
		//...
	}
	
	
	
}



// ================================================================================
