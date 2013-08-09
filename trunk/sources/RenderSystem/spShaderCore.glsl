/*
 * GLSL shader core file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

// Base math constants

#define PI			3.14159265359
#define E			2.71828182846

// Base data types

#define float2		vec2
#define float3		vec3
#define float4		vec4

#define double2		dvec2
#define double3		dvec3
#define double4		dvec4

#define bool2		bvec2
#define bool3		bvec3
#define bool4		bvec4

#define int2		ivec2
#define int3		ivec3
#define int4		ivec4

#define uint2		uvec2
#define uint3		uvec3
#define uint4		uvec4

#define float2x2	mat2
#define float3x3	mat3
#define float4x4	mat4

#define half		float
#define half2		vec2
#define half3		vec3
#define half4		vec4

// Function extensions

#define MUL(m, v)				(m) * (v)
#define MUL_TRANSPOSED(m, v)	transpose(m) * (v)
#define MUL_NORMAL(n)			(n).xyz = float3x3(Tangent, Binormal, Normal) * (n).xyz
#define CAST(t, v)				(t(v))
#define SAMPLER2D(n, i)			uniform sampler2D n

#define saturate(v)				clamp(v, 0.0, 1.0)
#define clip(v)					if (v < 0.0) { discard; }
#define frac(v)					fract(v)
#define lerp(x, y, s)			mix(x, y, s)

#define tex2D					texture2D
#define tex2Dgrad				texture2DGradARB
#define tex2DArrayLod(s, t)		texture2DArrayLod(s, (t).xyz, (t).w)
#define tex2DArray(s, t)		texture2DArray(s, t)
#define tex2DGrad(s, t, x, y)	texture2DGradARB(s, t, x, y)
#define ddx(v)					dFdx(v)
#define ddy(v)					dFdy(v)

// Texture and buffer operations

#define RWTexture3DUInt									layout (r32ui) coherent volatile uimage3D

#define floatBitsToUInt(v)								floatBitsToUint(v)

#define InterlockedAdd(d, v, o)							o = atomicAdd(d, v)
#define InterlockedAnd(d, v, o)							o = atomicAnd(d, v)
#define InterlockedOr(d, v, o)							o = atomicOr(d, v)
#define InterlockedXor(d, v, o)							o = atomicXor(d, v)
#define InterlockedMin(d, v, o)							o = atomicMin(d, v)
#define InterlockedMax(d, v, o)							o = atomicMax(d, v)
#define InterlockedCompareExchange(d, v, o)				o = atomicCompSwap(d, v)
#define InterlockedExchange(d, v, o)					o = atomicExchange(d, v)

#define InterlockedImageCompareExchange(i, p, c, v, o)	o = imageAtomicCompSwap(i, p, c, v)
