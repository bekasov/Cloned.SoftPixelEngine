/*
 * GLSL shader core file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#define PI						3.141592654

#define float2					vec2
#define float3					vec3
#define float4					vec4

#define int2					ivec2
#define int3					ivec3
#define int4					ivec4

#define float2x2				mat2
#define float3x3				mat3
#define float4x4				mat4

#define MUL(m, v)				(m) * (v)
#define MUL_TRANSPOSED(m, v)	transpose(m) * (v)
#define MUL_NORMAL(n)			(n).xyz = float3x3(Tangent, Binormal, Normal) * (n).xyz
#define CAST(t, v)				t(v)
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
#define ddx						dFdx
#define ddy						dFdy
