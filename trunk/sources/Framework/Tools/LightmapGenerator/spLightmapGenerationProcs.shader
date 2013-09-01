/*
 * Lightmap generation shader procedures file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#if 1

//!!! USED FOR 'groupshared SIdStack Stack'
void StackInit(uint StackOffset)
{
	Stack.Pointer = 0;
}

bool StackEmpty(uint StackOffset)
{
	return Stack.Pointer == 0;
}

void StackPush(uint StackOffset, uint Id)
{
	Stack.Data[Stack.Pointer] = Id;
	++Stack.Pointer;
}

uint StackPop(uint StackOffset)
{
	--Stack.Pointer;
	return Stack.Data[Stack.Pointer];
}

#else

//!!! USED FOR 'RWBuffer<uint> StackBuffer'
void StackInit(uint StackOffset)
{
	StackBuffer[StackOffset] = 0;
}

bool StackEmpty(uint StackOffset)
{
	return StackBuffer[StackOffset] == 0;
}

void StackPush(uint StackOffset, uint Id)
{
	uint Pointer = StackBuffer[StackOffset];
	StackBuffer[StackOffset + Pointer + 1] = Id;
	++StackBuffer[StackOffset];
}

uint StackPop(uint StackOffset)
{
	--StackBuffer[StackOffset];
	uint Pointer = StackBuffer[StackOffset];
	return StackBuffer[StackOffset + Pointer + 1];
}

#endif

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
	Intersection = CAST(float3, 0.0);
	
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
	Intersection = CAST(float3, 0.0);
	
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

void StackPushNodeChildren(SKDTreeNode Node, SLine Line, uint StackOffset)
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
		StackPush(StackOffset, Node.ChildIds[First]);
	}
	else
	{
		float t = (Node.Distance - Line.Start[Axis]) / Vec[Axis];
		
		/* Check if line segment straddles splitting plane */
		if (t >= 0.0 && t <= tmax)
		{
			/* Traverse near side first, then far side */
			StackPush(StackOffset, Node.ChildIds[First]);
			StackPush(StackOffset, Node.ChildIds[First ^ 1]);
		}
		else
		{
			/* Just traverse near side */
			StackPush(StackOffset, Node.ChildIds[First]);
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
	
    float AttnLinear    = Distance * SPHERE_INV_RADIUS(Light.Sphere);
    float AttnQuadratic = AttnLinear * Distance;
	
    float Intensity = 1.0 / (1.0 + AttnLinear + AttnQuadratic);
	
    if (Light.Type == LIGHT_SPOT)
		Intensity *= GetSpotLightIntensity(LightDir, Light);
	
    /* Compute diffuse color */
    Color += Light.Color * CAST(float3, Intensity * NdotL);
}

bool TexelVisibleFromLight(SLightSource Light, SLightmapTexel Texel, uint StackOffset)
{
	/* Setup line segment */
	SLine Line;
	Line.Start = SPHERE_POINT(Light.Sphere);
	Line.End = Texel.WorldPos;
	
	/* Transform line with inverse world matrix */
	Line.Start = MUL(InvWorldMatrix, float4(Line.Start, 1.0)).xyz;
	Line.End = MUL(InvWorldMatrix, float4(Line.End, 1.0)).xyz;
	
	/* Initialize node ID stack */
	StackInit(StackOffset);
	
	uint Id = 0;
	StackPush(StackOffset, Id);
	
	/* Iterate over all affected tree nodes */
	//[allow_uav_condition]
	while (!StackEmpty(StackOffset))
	{
		/* Get next tree node */
		Id = StackPop(StackOffset);
		SKDTreeNode Node = NodeList[Id];
		
		/* Check if this node contains triangle data */
		if (Node.NumTriangles > 0)
		{
			/* Iterate over all triangles inside the tree node */
			//[allow_uav_condition]
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
		
		/* Check if this node is a leaf */
		if (Node.ChildIds[0] != ID_NONE && Node.ChildIds[1] != ID_NONE)
		{
			/* Push child nodes which are affected by the line segment */
			StackPushNodeChildren(Node, Line, StackOffset);
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
	Color = CAST(float3, 0.0);
	Distance = 0.0;
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

void GenerateLightmapTexel(inout float3 Color, SLightSource Light, SLightmapTexel Texel, uint StackOffset)
{
	/* Compute direct illumination */
	if (TexelVisibleFromLight(Light, Texel, StackOffset))
		ComputeLightShading(Color, Light, Texel);
}
