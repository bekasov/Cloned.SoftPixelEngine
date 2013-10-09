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

#define half		float
#define half2		vec2
#define half3		vec3
#define half4		vec4

#define float2x2	mat2
#define float3x3	mat3
#define float4x4	mat4
#define float2x3	mat2x3
#define float2x4	mat2x4
#define float3x2	mat3x2
#define float3x4	mat3x4
#define float4x2	mat4x2
#define float4x3	mat4x3

#define double2x2	dmat2
#define double3x3	dmat3
#define double4x4	dmat4
#define double2x3	dmat2x3
#define double2x4	dmat2x4
#define double3x2	dmat3x2
#define double3x4	dmat3x4
#define double4x2	dmat4x2
#define double4x3	dmat4x3

#define bool2x2		bmat2
#define bool3x3		bmat3
#define bool4x4		bmat4
#define bool2x3		bmat2x3
#define bool2x4		bmat2x4
#define bool3x2		bmat3x2
#define bool3x4		bmat3x4
#define bool4x2		bmat4x2
#define bool4x3		bmat4x3

#define int2x2		imat2
#define int3x3		imat3
#define int4x4		imat4
#define int2x3		imat2x3
#define int2x4		imat2x4
#define int3x2		imat3x2
#define int3x4		imat3x4
#define int4x2		imat4x2
#define int4x3		imat4x3

#define uint2x2		umat2
#define uint3x3		umat3
#define uint4x4		umat4
#define uint2x3		umat2x3
#define uint2x4		umat2x4
#define uint3x2		umat3x2
#define uint3x4		umat3x4
#define uint4x2		umat4x2
#define uint4x3		umat4x3

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
#define tex2DGrad(s, t, dx, dy)	texture2DGradARB(s, t, dx, dy)
#define ddx(v)					dFdx(v)
#define ddy(v)					dFdy(v)

// Texture and buffer operations

#define DeclStructuredBuffer(s, n, r)						\
	layout(std430, binding = r) readonly buffer Buffer##n	\
	{														\
		s n[];												\
	}

#define DeclBuffer(t, n, r)									\
	layout(std430, binding = r) readonly buffer Buffer##n	\
	{														\
		t n[];												\
	}

#define DeclRWStructuredBuffer(s, n, r)				\
	layout(std430, binding = r) buffer Buffer##n	\
	{												\
		s n[];										\
	}

#define DeclRWBuffer(t, n, r)						\
	layout(std430, binding = r) buffer Buffer##n	\
	{												\
		t n[];										\
	}

#define DeclConstBuffer(n, r)							layout(std140, binding = r) uniform n
#define RWTexture3DUInt									layout(r32ui) coherent volatile uimage3D

#define groupshared										shared

#define floatBitsToUInt(v)								floatBitsToUint(v)

#define InterlockedAdd(d, v, o)							o = atomicAdd(d, v)
#define InterlockedAnd(d, v, o)							o = atomicAnd(d, v)
#define InterlockedOr(d, v, o)							o = atomicOr(d, v)
#define InterlockedXor(d, v, o)							o = atomicXor(d, v)
#define InterlockedMin(d, v, o)							o = atomicMin(d, v)
#define InterlockedMax(d, v, o)							o = atomicMax(d, v)
#define InterlockedCompareExchange(d, v, o)				o = atomicCompSwap(d, v)
#define InterlockedExchange(d, v, o)					o = atomicExchange(d, v)

#define GroupMemoryBarrier								groupMemoryBarrier

#define InterlockedImageCompareExchange(i, p, c, v, o)	o = imageAtomicCompSwap(i, p, c, v)

#define __DEFINE_MUL_FUNC__(m, v)		\
	inline v mul(m mat, v vec)			\
	{									\
		return mat * vec;				\
	}									\
	inline v mul(v vec, m mat)			\
	{									\
		return transpose(mat) * vec;	\
	}

__DEFINE_MUL_FUNC__(float2x2, float2)
__DEFINE_MUL_FUNC__(float3x3, float3)
__DEFINE_MUL_FUNC__(float4x4, float4)

__DEFINE_MUL_FUNC__(int2x2, int2)
__DEFINE_MUL_FUNC__(int3x3, int3)
__DEFINE_MUL_FUNC__(int4x4, int4)

#undef __DEFINE_MUL_FUNC__

inline float4x4 mul(float4x4 a, float4x4 b)
{
	return a * b;
}
inline int4x4 mul(int4x4 a, int4x4 b)
{
	return a * b;
}
