/*
 * Shadow shader procedures file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifdef USE_VSM

float2 ComputeMoments(float Depth)
{
    float2 Moments;
    
    // First moment is the depth itself
    Moments.x = Depth;
    
    // Compute partial derivatives of depth
    float dx = ddx(Depth);
    float dy = ddy(Depth);
    
    Moments.y = Depth*Depth + 0.25 * (dx*dx + dy*dy);
    
    return Moments;
}

#endif
