/*
 * Lightmap generation shader procedures file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

void StackInit(out uint StackPointer)
{
	StackPointer = 0;
}

bool StackEmpty(SIdStack Stack)
{
	return Stack.StackPointer == 0;
}

void StackPush(SIdStack Stack, inout uint StackPointer, uint Id)
{
	Stack.Data[StackPointer] = Id;
	++StackPointer;
}

uint StackPop(SIdStack Stack, inout uint StackPointer)
{
	--StackPointer;
	return Stack.Data[StackPointer];
}

SPlane BuildPlane(STriangle Tri)
{
	SPlane Plane;
	
	float3 U = Tri.B - Tri.A;
	float3 V = Tri.C - Tri.A;
	
	PLANE_NORMAL(Plane) = normalize(cross(U, V));
	PLANE_DISTANCE(Plane) = dot(PLANE_NORMAL(Plane), Tri.A);
	
	return Plane;
}

bool IntersectionRayPlane(SPlane Plane, SRay Ray, out float3 Intersection)
{
	float t = (PLANE_DISTANCE(Plane) - dot(PLANE_NORMAL(Plane), Ray.Origin)) / dot(PLANE_NORMAL(Plane), Ray.Direction);
	
	if (t >= 0.0)
	{
		Intersection = Ray.Origin + Ray.Direction * CAST(float3, t);
		return true;
	}
	
	return false;
}

bool IntersectionRayTriangle(STriangle Tri, SRay Ray, out float3 Intersection)
{
	float3 pa = Tri.A - Ray.Origin;
	float3 pb = Tri.B - Ray.Origin;
	float3 pc = Tri.C - Ray.Origin;
	
	/* Check if ray direction is inside the edges bc, ca and ab */
	Intersection.x = dot(pb, cross(Ray.Direction, pc));
	if (Intersection.x < 0.0)
		return false;
	
	Intersection.y = dot(pc, cross(Ray.Direction, pa));
	if (Intersection.y < 0.0)
		return false;
	
	Intersection.z = dot(pa, cross(Ray.Direction, pb));
	if (Intersection.z < 0.0)
		return false;
	
	return IntersectionRayPlane(BuildPlane(Tri), Ray, Intersection);
}

bool IntersectionLinePlane(SPlane Plane, SLine Line, out float3 Intersection)
{
	float3 Dir = Line.End - Line.Start;
	float t = (PLANE_DISTANCE(Plane) - dot(PLANE_NORMAL(Plane), Line.Start)) / dot(PLANE_NORMAL(Plane), Dir);
	
	if (t >= 0.0 && t <= 1.0)
	{
		Intersection = Line.Start + Dir * CAST(float3, t);
		return true;
	}
	
	return false;
}

bool IntersectionLineTriangle(STriangle Tri, SLine Line, out float3 Intersection)
{
	float3 pq = Line.End - Line.Start;
	float3 pa = Tri.A - Line.Start;
	float3 pb = Tri.B - Line.Start;
	float3 pc = Tri.C - Line.Start;
	
	/* Check if pq is inside the edges bc, ca and ab */
	Intersection.x = dot(pb, cross(pq, pc));
	if (Intersection.x < 0.0)
		return false;
	
	Intersection.y = dot(pc, cross(pq, pa));
	if (Intersection.y < 0.0)
		return false;
	
	Intersection.z = dot(pa, cross(pq, pb));
	if (Intersection.z < 0.0)
		return false;
	
	return IntersectionLinePlane(BuildPlane(Tri), Line, Intersection);
}

bool OverlapLineTriangle(STriangle Tri, SLine Line)
{
	float3 Intersection = CAST(float3, 0.0);
	
	if (IntersectionLineTriangle(Tri, Line, Intersection))
	{
		return
			distance(Intersection, Line.Start) < EPSILON && 
			distance(Intersection, Line.End) < EPSILON;
	}
	
	return false;
}

void StackPushNodeChildren(SIdStack Stack, inout uint StackPointer, SKDTreeNode Node, SLine Line)
{
	/* Get line segment in other representation */
	float3 Vec = Line.End - Line.Start;
	float tmax = length(Vec);
	Vec = normalize(Vec);
	
	int Axis = Node.Axis;
	int First = (Line.Start[Axis] > Node.Distance);
	
	if (Vec[Axis] == 0.0)
	{
		/* Line segment parallel to splitting plane, visit near side only */
		StackPush(Stack, StackPointer, Node.ChildIds[First]);
	}
	else
	{
		float t = (Node.Distance - Line.Start[Axis]) / Vec[Axis];
		
		/* Check if line segment straddles splitting plane */
		if (t >= 0.0 && t <= tmax)
		{
			/* Traverse near side first, then far side */
			StackPush(Stack, StackPointer, Node.ChildIds[First]);
			StackPush(Stack, StackPointer, Node.ChildIds[First ^ 1]);
		}
		else
		{
			/* Just traverse near side */
			StackPush(Stack, StackPointer, Node.ChildIds[First]);
		}
	}
}

/*bool IntersectionRayMesh()
{
	//todo ...
	return false;
}*/

float GetAngle(float3 a, float3 b)
{
    return acos(dot(a, b));
}

float GetSpotLightIntensity(float3 LightDir, SLightSource Light)
{
	/* Compute spot light cone */
	float Angle = GetAngle(LightDir, Light.Direction);
	float ConeAngleLerp = (Angle - Light.SpotTheta) / Light.SpotPhiMinusTheta;
	
	return saturate(1.0 - ConeAngleLerp);
}

void ComputeLightShading(inout float3 Color, SLightSource Light, SLightmapTexel Texel)
{
    /* Compute light direction vector */
    float3 LightDir = CAST(float3, 0.0);
	
    if (Light.Type != LIGHT_DIRECTIONAL)
        LightDir = normalize(Texel.WorldPos - SPHERE_POINT(Light.Sphere));
    else
        LightDir = Light.Direction;
	
    /* Compute phong shading */
    float NdotL = max(0.0, -dot(Texel.Normal, LightDir));
	
    /* Compute light attenuation */
    float Distance = distance(Texel.WorldPos, SPHERE_POINT(Light.Sphere));
	
    float AttnLinear    = Distance * SPHERE_RADIUS(Light.Sphere);
    float AttnQuadratic = AttnLinear * Distance;
	
    float Intensity = 1.0 / (1.0 + AttnLinear + AttnQuadratic);
	
    if (Light.Type == LIGHT_SPOT)
		Intensity *= GetSpotLightIntensity(LightDir, Light);
	
    /* Compute diffuse color */
    Color += Light.Color * CAST(float3, Intensity * NdotL);
}

bool TexelVisibleFromLight(SLightSource Light, SLightmapTexel Texel)
{
	/* Setup line segment */
	SLine Line;
	Line.Start = SPHERE_POINT(Light.Sphere);
	Line.End = Texel.WorldPos;
	
	/* Initialize node ID stack */
	SIdStack Stack;
	StackInit(Stack.StackPointer);
	
	uint Id = 0;
	StackPush(Stack, Stack.StackPointer, Id);
	
	/* Iterate over all affected tree nodes */
	while (!StackEmpty(Stack))
	{
		/* Get next tree node */
		Id = StackPop(Stack, Stack.StackPointer);
		SKDTreeNode Node = NodeList[Id];
		
		/* Check if this is a node leaf */
		if (Node.TriangleStart != ID_NONE)
		{
			/* Iterate over all triangles inside the tree node */
			for (uint i = Node.TriangleStart, n = i + Node.NumTriangles; i < n; ++i)
			{
				/* Get current triangle */
				uint TriIndex = TriangleIdList[i];
				STriangle Tri = TriangleList[TriIndex];
				
				/* Make intersection tests */
				if (OverlapLineTriangle(Tri, Line))
					return false;
			}
		}
		else
		{
			/* Push child nodes which are affected by the line segment */
			StackPushNodeChildren(Stack, Stack.StackPointer, Node, Line);
		}
	}
	
	return true;
}

float3 GetRandomRayDirection(float3x3 NormalMatrix, uint Index)
{
	return MUL(NormalMatrix, RadiosityDirections[Index].xyz);
}

bool SampleLightEnergy(SRay Ray, out float3 Color, out float Distance)
{
	//todo...
	return false;
}

void ComputeRadiosityShading(inout float3 Color, SRay Ray, float3 TexelNormal)
{
	/* Sampel light energy along specified ray */
	float3 SampleColor = CAST(float3, 0.0);
	float SampleDistance = 0.0;
	
	if (SampleLightEnergy(Ray, SampleColor, SampleDistance))
	{
		/* Add light energy to final texel color */
		float NdotL = max(0.0, dot(TexelNormal, Ray.Direction));
		
		Color += SampleColor * CAST(float3, NdotL * RadiosityFactor);
	}
}

void GenerateLightmapTexel(inout float3 Color, SLightSource Light, SLightmapTexel Texel)
{
	/* Compute direct illumination */
	if (TexelVisibleFromLight(Light, Texel))
		ComputeLightShading(Color, Light, Texel);
	
	/* Compute radiosity */
	if (NumRadiosityRays > 0)
	{
		/* Generate normal matrix (tangent space) */
		float3x3 NormalMatrix = float3x3(
			Texel.Tangent,
			cross(Texel.Normal, Texel.Tangent),
			Texel.Normal
		);
		
		/* Sample radiosity rays */
		SRay Ray;
		Ray.Origin = Texel.WorldPos;
		
		for (uint i = 0; i < NumRadiosityRays; ++i)
		{
			Ray.Direction = GetRandomRayDirection(NormalMatrix, i);
			ComputeRadiosityShading(Color, Ray, Texel.Normal);
		}
	}
}



