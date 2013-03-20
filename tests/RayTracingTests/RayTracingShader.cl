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

#define GlbConstStruct		__global const struct
#define KDTreeNodeGlbPtr	GlbConstStruct KDTreeNode*


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
	
	struct AABBox3D Box;
	
	int NearChildIndex;		//!< KDTreeNode array index for near child (-1 means no children).
	int FarChildIndex;		//!< KDTreeNode array index for far child (-1 means no children).
	
	uint FirstTriangle;		//!< Triangle array start index.
	uint LastTriangle;		//!< Triangle array end index.
};

struct KDTreeNodeStack
{
	KDTreeNodeGlbPtr Stack[TREENODE_STACK_SIZE];
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
	struct Sphere3D Sphere;
	struct SurfaceMaterial Material;
};

struct SurfacePlane
{
	struct Plane3D Plane;
	struct SurfaceMaterial Material;
};


/*
 * ======= Functions =======
 */

/* === Collision detection functions === */

bool CheckRayAABBOverlap1D(
	float start, float dir, float min, float max, float* enter, float* exit)
{
	/* Ray parallel to direction */
	if (dir > -EPSILON && dir < EPSILON)
		return (start >= min && start <= max);
	
	/* Intersection parameters */
	float t0 = (min - start) / dir;
	float t1 = (max - start) / dir;
	
	/* Sort intersections */
	if (t0 > t1)
	{
		float tmp = t0;
		t0 = t1;
		t1 = tmp;
	}
	
	/* Check if intervals are disjoint */
	if (t0 > *exit || t1 < *enter)
		return false;
	
	/* Reduce interval */
	if (t0 > *enter)
		*enter = t0;
	if (t1 < *exit)
		*exit = t1;

	return true;
}

bool CheckRayBoxOverlap(const struct Ray3D* Ray, GlbConstStruct AABBox3D* Box)
{
	float enter = 0.0, exit = 1.0;
	
	if (!CheckRayAABBOverlap1D(Ray->Point.x, Ray->Direction.x, Box->Min.x, Box->Max.x, &enter, &exit))
		return false;
	if (!CheckRayAABBOverlap1D(Ray->Point.y, Ray->Direction.y, Box->Min.y, Box->Max.y, &enter, &exit))
		return false;
	if (!CheckRayAABBOverlap1D(Ray->Point.z, Ray->Direction.z, Box->Min.z, Box->Max.z, &enter, &exit))
		return false;
	
	return true;
}

inline bool CheckKDTreeNode(const struct Ray3D* Ray, KDTreeNodeGlbPtr Node)
{
	return CheckRayBoxOverlap(Ray, &(Node->Box));
}

inline bool IsKDTreeLeaf(KDTreeNodeGlbPtr Node)
{
	return Node->NearChildIndex == -1;
}

void ComputePlane(const struct Triangle3D* Triangle, struct Plane3D* Plane)
{
	Plane->Normal = normalize(cross(Triangle->B - Triangle->A, Triangle->C - Triangle->A));
	Plane->Distance = dot(Plane->Normal, Triangle->A);
}

bool CheckRayPlaneIntersection(const struct Ray3D* Ray, const struct Plane3D* Plane, float3* Intersection)
{
	float t = (Plane->Distance - dot(Plane->Normal, Ray->Point)) / dot(Plane->Normal, Ray->Direction);
	
	if (t >= 0.0 && t <= 1.0)
	{
		*Intersection = Ray->Point + Ray->Direction * t;
		return true;
	}
	
	return false;
}

bool CheckRayTriangleIntersection(const struct Ray3D* Ray, const struct Triangle3D* Triangle, float3* Intersection)
{
	float3 pa = Triangle->A - Ray->Point;
	float3 pb = Triangle->B - Ray->Point;
	float3 pc = Triangle->C - Ray->Point;
	
	/* Check if ray direction is inside the edges bc, ca and ab */
	Intersection->x = dot(pb, cross(Ray->Direction, pc));
	if (Intersection->x < 0.0)
		return false;
	
	Intersection->y = dot(pc, cross(Ray->Direction, pa));
	if (Intersection->y < 0.0)
		return false;
	
	Intersection->z = dot(pa, cross(Ray->Direction, pb));
	if (Intersection->z < 0.0)
		return false;
	
	/* Make ray-plane intersection test */
	struct Plane3D Plane;
	ComputePlane(Triangle, &Plane);
	
	return CheckRayPlaneIntersection(Ray, &Plane, Intersection);
}

bool IntersectionTest(const struct Ray3D* Ray, float3* Intersection)
{
	
	//todo ...
	
	return false;
}


/* === Stack functions === */

void TreeNodePush(struct KDTreeNodeStack* NodeStack, KDTreeNodeGlbPtr Node)
{
	if (NodeStack->Pointer < TREENODE_STACK_SIZE)
	{
		NodeStack->Stack[NodeStack->Pointer] = Node;
		++NodeStack->Pointer;
	}
}

KDTreeNodeGlbPtr TreeNodePop(struct KDTreeNodeStack* NodeStack)
{
	if (NodeStack->Pointer > 0)
	{
		KDTreeNodeGlbPtr Node = NodeStack->Stack[NodeStack->Pointer];
		--NodeStack->Pointer;
		return Node;
	}
	return 0;
}


/*
 * ======= Kernels =======
 */

__kernel void RenderRayTracing(
	/* Input settings */
	int BlockWidth,
	int BlockHeight,
	
	/* Output image buffer */
	int ImageWidth,
	int ImageHeight,
	__write_only image2d_t ResultImage,
	
	/* View transformation */
	float16 ViewMatrix,
	
	/* Tree node hierarchy */
	uint TreeNodeCount,
	GlbConstStruct KDTreeNode* TreeNodeList,
	
	#if 0
	
	/* Light sources */
	uint LightCount,
	GlbConstStruct LightSource* LightSourceList,
	
	#endif
	
	/* Index buffer */
	uint TriangleCount,
	__global const uint* TriangleList,
	
	/* Vertex buffer */
	uint VertexCount,
	GlbConstStruct SurfaceVertex* VertexList
	
	#if 0
	,
	/* Sphere geometries */
	uint SphereCount,
	GlbConstStruct SurfaceSphere* SphereList,
	
	/* Plane geometries */
	uint PlaneCount,
	GlbConstStruct SurfacePlane* PlaneList
	
	#endif
)
{
	/* Get image coordinate */
	int xId = get_global_id(0);
	int yId = get_global_id(1);
	
	for (int y = yId*BlockHeight, yNum = min(yId*BlockHeight + BlockHeight, ImageHeight); y < yNum - 1; ++y)
	for (int x = xId*BlockWidth , xNum = min(xId*BlockWidth  + BlockWidth , ImageWidth ); x < xNum - 1; ++x)
	{
		#if 0
		
		/* Generate view ray */
		struct Ray3D Ray;
		
		Ray.Direction.x = (float)x / (float)ImageWidth - 0.5;
		Ray.Direction.y = (float)y / (float)ImageHeight - 0.5;
		Ray.Direction.z = 1.0;
		Ray.Direction = normalize(Ray.Direction);
		
		//... frustum
		
		/* Varaibles for finding the nearest intersection distance to view */
		float Distance = 0.0, NearestDistance = OMEGA;
		float3 Intersection = 0.0, NearestIntersction = OMEGA;
		uint NearestTriangleIndex = ~0;
		
		/* Make intersection tests with geometry */
		KDTreeNodeGlbPtr Node = &(TreeNodeList[0]);
		
		struct KDTreeNodeStack NodeStack;
		TreeNodePush(&NodeStack, Node);
		
		while (Node)
		{
			if (IsKDTreeLeaf(Node))
			{
				/* Make intersection tests with all triangles of the current tree node leaf */
				for (uint i = Node->FirstTriangle; i <= Node->LastTriangle; i += 3)
				{
					/* Get indices and triangle vertices */
					uint v0 = TriangleList[i    ];
					uint v1 = TriangleList[i + 1];
					uint v2 = TriangleList[i + 2];
					
					GlbConstStruct SurfaceVertex* a = &VertexList[v0];
					GlbConstStruct SurfaceVertex* b = &VertexList[v1];
					GlbConstStruct SurfaceVertex* c = &VertexList[v2];
					
					struct Triangle3D Triangle;
					
					Triangle.A = a->Coord;
					Triangle.B = b->Coord;
					Triangle.C = c->Coord;
					
					/* Make intersection test with current triangle */
					if (CheckRayTriangleIntersection(&Ray, &Triangle, &Intersection))
					{
						Distance = length(Intersection);
						
						if (Distance < NearestDistance)
						{
							/* Found new nearest intersection */
							NearestDistance			= Distance;
							NearestIntersction		= Intersection;
							NearestTriangleIndex	= i;
						}
					}
				}
			}
			else
			{
				/* Traverse next child tree node(s) */
				GlbConstStruct KDTreeNode* NearChild = &TreeNodeList[Node->NearChildIndex];
				GlbConstStruct KDTreeNode* FarChild = &TreeNodeList[Node->FarChildIndex];
				
				if (CheckKDTreeNode(&Ray, NearChild))
					TreeNodePush(&NodeStack, NearChild);
				else if (CheckKDTreeNode(&Ray, FarChild))
					TreeNodePush(&NodeStack, FarChild);
				else
					continue;
			}
			
			/* Get current tree node on the stack */
			Node = TreeNodePop(&NodeStack);
		}
		
		#if 0
		
		/* Make intersection tests with primitives (planes, spheres etc.) */
		for (uint i = 0; i < SphereCount; ++i)
		{
			const struct SurfaceSphere* Obj = SphereList[i];
			
			//...
		}
		
		for (uint i = 0; i < PlaneCount; ++i)
		{
			const struct SurfacePlane* Obj = PlaneList[i];
			
			//...
		}
		
		#endif
		
		#endif
		
		/* Compute final pixel color */
		//float4 Color = (float4)((float3)(NearestDistance*0.1), 1.0);
		float4 Color;
		
		Color.x = (float)x / ImageWidth;
		Color.y = (float)y / ImageHeight;
		Color.z = (float)x / ImageWidth;
		Color.w = (float)y / ImageHeight;
		
		write_imagef(ResultImage, (int2)(x, y), Color);
	}
}



// ================================================================================
