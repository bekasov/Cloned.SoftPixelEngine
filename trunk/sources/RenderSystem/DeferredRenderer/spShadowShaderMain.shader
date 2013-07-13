/*
 * Shadow pixel shader main file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#if defined(USE_RSM)

/* Sample color texture */
float4 Diffuse = tex2D(DiffuseMap, TEX_COORD);

/* Perform alpha-test */
clip(Diffuse.a - 0.5);

#elif defined(USE_TEXTURE)

/* Perform alpha-test */
clip(tex2D(AlphaMap, TEX_COORD).a - 0.5);

#endif

/* Compute depth distance */
float Depth = distance(WORLD_POS, ViewPosition);

#ifdef USE_VSM

float2 Moments = ComputeMoments(Depth);

DEPTH_DIST.r = Moments.x;
DEPTH_DIST.a = Moments.y;

#else

DEPTH_DIST = Depth;

#endif
