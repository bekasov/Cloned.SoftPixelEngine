/*
 * Lightmap generation OpenCL shader
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

/*
 * ======= Structures =======
 */

struct SKDTreeNode
{
	char Axis;
	float Distance;
	
	float3 BoxMin;
	float3 BoxMax;
	
	int NearChildIndex;
	int FarChildIndex;
	
	int TriangleListIndex;
};


/*
 * ======= Functions =======
 */

int FindKDTreeLeaf(float3 LineStart, float3 LineEnd)
{
	
	
	
}


/*
 * ======= Kernels =======
 */

__kernel void GenerateLightmap(
	int LightmapWidth,
	int LightmapHeight,
	__global float4* LightmapImageBuffer,
	uint TreeNodeCount,
	__global const SKDTreeNode* TreeNodeList,
	uint TriangleCount,
	__global const uint* TriangleList,
	uint VertexCount,
	__global const float3* VertexList)
{
	/* Get lightmap image coordinate */
	int x = get_global_id(0);
	int y = get_global_id(1);
	
	
	
	
	
	
}


// ================================================================================
