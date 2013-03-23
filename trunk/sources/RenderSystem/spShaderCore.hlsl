/*
 * HLSL shader core file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#define MUL(m, v)				mul(m, v)
#define MUL_TRANSPOSED(v, m)	mul(v, m)
#define CAST(t, v)				((t)(v))

#define mod(a, b)				fmod(a, b)

#define tex2DArrayLod(s, t)		tex2Dlod(s, t)
#define tex2DGrad(s, t, x, y)	tex2Dgrad(s, t, x, y)
