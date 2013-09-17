/*
 * Sparse voxel octree procedures file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

/* === Macros === */

#define AXIS_X_POSITIVE 0 // +x
#define AXIS_X_NEGATIVE 1 // -x
#define AXIS_Y_POSITIVE 2 // +y
#define AXIS_Y_NEGATIVE 3 // -y
#define AXIS_Z_POSITIVE 4 // +z
#define AXIS_Z_NEGATIVE 5 // -z


/* === Functions === */

int GetDominantAxis(float3 Vec)
{
    /* Get absolute vector */
    float3 AbsVec = abs(Vec);

    /* Find dominant axis */
    if (AbsVec.x >= AbsVec.y && AbsVec.x >= AbsVec.z)
        return (Vec.x > 0.0 ? AXIS_X_POSITIVE : AXIS_X_NEGATIVE);
    if (AbsVec.y >= AbsVec.x && AbsVec.y >= AbsVec.z)
        return (Vec.y > 0.0 ? AXIS_Y_POSITIVE : AXIS_Y_NEGATIVE);
    return (Vec.z > 0.0 ? AXIS_Z_POSITIVE : AXIS_Z_NEGATIVE);
}

int GetDominantAxisAbs(float3 Vec)
{
    /* Get absolute vector */
    Vec = abs(Vec);

    /* Find dominant axis */
    if (Vec.x >= Vec.y && Vec.x >= Vec.z)
        return AXIS_X_POSITIVE;
    if (Vec.y >= Vec.x && Vec.y >= Vec.z)
        return AXIS_Y_POSITIVE;
    return AXIS_Z_POSITIVE;
}

void ImageAtomicFloatAdd(RWTexture3DUInt Image, int3 Coord, float Value)
{
	uint NewValue = floatBitsToUInt(Value);
	uint PrevValue = 0;
	uint CurValue;
	
	/* Loop as long as destination value gets changed by other threads */
	while ( InterlockedImageCompareExchange(Image, Coord, PrevValue, NewValue, CurValue), CurValue != PrevValue )
	{
		PrevValue = CurValue;
		NewValue = floatBitsToUInt((Value + uintBitsToFloat(CurValue)));
	}
}

float4 ConvU32ToFloat4(uint Value)
{
	return float4(
		CAST(float, (Value & 0x000000FF)       ),
		CAST(float, (Value & 0x0000FF00) >>  8u),
		CAST(float, (Value & 0x00FF0000) >> 16u),
		CAST(float, (Value & 0xFF000000) >> 24u)
	);
}

uint ConvFloat4ToU32(float4 Value)
{
	return
		(CAST(uint, Value.w) & 0x000000FF) << 24u |
		(CAST(uint, Value.z) & 0x000000FF) << 16u |
		(CAST(uint, Value.y) & 0x000000FF) <<  8u |
		(CAST(uint, Value.x) & 0x000000FF);
}

void ImageAtomicRGBA8Avg(RWTexture3DUInt Image, int3 Coord, float4 Value)
{
	Value.rgb *= 255.0f; // Optimise following calculations
	
	uint NewVal = ConvFloat4ToU32(Value);
	
	uint PrevStoredVal = 0;
	uint CurStoredVal;
	
	/* Loop with limited iterations to allow shader compiler to unroll loop */
	for (uint i = 0; i < 8; ++i)
	{
		/* Loop as long as destination value gets changed by other threads */
		InterlockedImageCompareExchange(Image, Coord, PrevStoredVal, NewVal, CurStoredVal);
		
		if (CurStoredVal != PrevStoredVal)
			break;
		
		PrevStoredVal = CurStoredVal;
		
		float4 RVal = ConvU32ToFloat4(CurStoredVal);
		RVal.xyz = (RVal.xyz * RVal.w); // Denormalize
		
		float4 CurValF = RVal + Value; // Add new value
		CurValF.xyz /= (CurValF.w); // Renormalize
		
		NewVal = ConvFloat4ToU32(CurValF);
	}
}
