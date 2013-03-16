//
// OpenCL Compute Shader
//
// Copyright (c) 2012 - Lukas Hermanns
//


/* === Structures === */

struct SMatrix4x4
{
	float4 v0, v1, v2, v3;
};


/* === Functions === */

void MultVec3Mat4x4(struct SMatrix4x4 Mat, __global float* Out, __global float* In)
{
	Out[0] = In[0]*Mat.v0.x + In[1]*Mat.v1.x + In[2]*Mat.v2.x + Mat.v3.x;
	Out[1] = In[0]*Mat.v0.y + In[1]*Mat.v1.y + In[2]*Mat.v2.y + Mat.v3.y;
	Out[2] = In[0]*Mat.v0.z + In[1]*Mat.v1.z + In[2]*Mat.v2.z + Mat.v3.z;
}


/* === Kernels === */

__kernel void MainKernel(
	__global float* VertexBuffer,
	uint VertexCount,
	uint OffsetSize,
	struct SMatrix4x4 WorldMatrix)
{
	/* Get kernel instance ID */
	int Id = get_global_id(0);
	
	int i = Id*OffsetSize;
	int c = min(i + OffsetSize, VertexCount);
	
	VertexBuffer += i*9;
	
	while (i < c)
	{
		__global float* RealPos		= VertexBuffer;
		__global float* Size		= VertexBuffer+3;
		__global float* FinalPos	= VertexBuffer+5;
		
		float Speed = FinalPos[3];
		
		Size[0] -= 0.01*Speed;
		Size[1] += 0.005*Speed;
		
		FinalPos[1] += 0.02*Speed;
		
		if (Size[0] < 0.0)
		{
			Size[0] = 0.7;
			Size[1] = 0.0;
			
			MultVec3Mat4x4(WorldMatrix, FinalPos, RealPos);
			
			FinalPos[1] -= 1.0;
		}
		
		VertexBuffer += 9;
		++i;
	}
	
	
}


// ================================================================================
