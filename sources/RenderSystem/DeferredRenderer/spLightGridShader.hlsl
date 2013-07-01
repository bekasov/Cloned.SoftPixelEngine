/*
 * Light grid D3D11 shader file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

/*
 * ======= Compute shader: =======
 */

/* === Macros === */

#define EOL 0xFFFFFFFF


/* === Structures === */

struct SLightNode
{
	uint LightID;	//!< SLight index.
	uint LightExID;	//!< SLightEx index.
	uint Next;		//!< Next SLightNode index. 'EOL' if end of linked list.
};


/* === Buffers === */

cbuffer BufferMain : register(b0)
{
	int2 GridSize;
};

RWBuffer<int2> LightGrid : register(u0);
RWStructuredBuffer<SLightNode> TileLightIndexList : register(u1);


/* === Functions === */

[numthreads(1, 1, 1)]
void ComputeMain(uint3 Id : SV_DispatchThreadID)
{
	
	
	
}
