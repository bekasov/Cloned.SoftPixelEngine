/*
 * Rasterizer file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "Base/spMathRasterizer.hpp"


namespace sp
{
namespace math
{


namespace Rasterizer
{

SP_EXPORT void rasterizeTriangle(
    const RenderPixelCallback &RenderCallback,
    dim::point2di PointA, dim::point2di PointB, dim::point2di PointC,
    void* UserData)
{
    /* Sort the vertices in dependet of the y axis */
    if (PointA.Y > PointB.Y)
        Swap(PointA, PointB);
    if (PointA.Y > PointC.Y)
        Swap(PointA, PointC);
    if (PointB.Y > PointC.Y)
        Swap(PointB, PointC);
    
    /* Set the vertices position */
    s32 yStart  = PointA.Y;
    s32 yMiddle = PointB.Y;
    s32 yEnd    = PointC.Y;
    
    /* Compute the dimensions */
    s32 yMiddleStart    = yMiddle - yStart;
    s32 yEndMiddle      = yEnd - yMiddle;
    s32 yEndStart       = yEnd - yStart;
    
    /* Iteration variables */
    s32 x, y, xStart, xEnd;
    
    /* Rater each scanline from top to bottom */
    for (y = yStart; y < yEnd; ++y)
    {
        /* Compute the scanline dimension */
        if (y < yMiddle)
            xStart = PointA.X + ( PointB.X - PointA.X ) * ( y - yStart ) / yMiddleStart;
        else if (y > yMiddle)
            xStart = PointB.X + ( PointC.X - PointB.X ) * ( y - yMiddle ) / yEndMiddle;
        else
            xStart = PointB.X;
        
        xEnd = PointA.X + ( PointC.X - PointA.X ) * ( y - yStart ) / yEndStart;
        
        /* Compute the polygon sides */
        if (xStart > xEnd)
            math::Swap(xStart, xEnd);
        
        /* Render each pixel in scanline */
        for (x = xStart; x < xEnd; ++x)
            RenderCallback(x, y, UserData);
    }
}

SP_EXPORT void rasterizeCircle(
    const RenderPixelCallback &RenderCallback,
    const dim::point2di &Position, s32 Radius, void* UserData)
{
    /* Temporary memory */
    s32 r   = Abs(Radius);
    s32 f   = 1 - r;
    s32 dx  = 0;
    s32 dy  = -2 * r;
    s32 x   = 0;
    s32 y   = r;
    
    /* Draw the border pixels */
    RenderCallback(Position.X, Position.Y + r, UserData);
    RenderCallback(Position.X, Position.Y - r, UserData);
    RenderCallback(Position.X + r, Position.Y, UserData);
    RenderCallback(Position.X - r, Position.Y, UserData);
    
    while (x < y)
    {
        if (f >= 0)
        {
            --y;
            dy += 2;
            f += dy;
        }
        
        ++x;
        dx += 2;
        f += dx + 1;
        
        /* Along the border draw further pixels */
        RenderCallback(Position.X + x, Position.Y + y, UserData);
        RenderCallback(Position.X - x, Position.Y + y, UserData);
        RenderCallback(Position.X + x, Position.Y - y, UserData);
        RenderCallback(Position.X - x, Position.Y - y, UserData);
        RenderCallback(Position.X + y, Position.Y + x, UserData);
        RenderCallback(Position.X - y, Position.Y + x, UserData);
        RenderCallback(Position.X + y, Position.Y - x, UserData);
        RenderCallback(Position.X - y, Position.Y - x, UserData);
    }
}

SP_EXPORT void rasterizeEllipse(
    const RenderPixelCallback &RenderCallback, const dim::point2di &Position,
    const dim::size2di &Radius, void* UserData)
{
    /* Temporary memory */
    s32 a   = Abs(Radius.Width);
    s32 b   = Abs(Radius.Height);
    s32 xc  = Position.X;
    s32 yc  = Position.Y;
    s32 a2  = a*a;
    s32 b2  = b*b;
    s32 fa2 = 4*a2;
    s32 fb2 = 4*b2;
    
    s32 x, y, sigma;
    
    /* Draw the ellipse using the "Brensenham Algorithm" */
    for (x = 0, y = b, sigma = 2*b2 + a2*(1-2*b); b2*x <= a2*y; ++x)
    {
        RenderCallback(xc + x, yc + y, UserData);
        RenderCallback(xc - x, yc + y, UserData);
        RenderCallback(xc + x, yc - y, UserData);
        RenderCallback(xc - x, yc - y, UserData);
        
        if (sigma >= 0)
        {
            sigma += fa2*(1-y);
            --y;
        }
        
        sigma += b2*(4*x+6);
    }
    
    for (x = a, y = 0, sigma = 2*a2 + b2*(1-2*a); a2*y <= b2*x; ++y)
    {
        RenderCallback(xc + x, yc + y, UserData);
        RenderCallback(xc - x, yc + y, UserData);
        RenderCallback(xc + x, yc - y, UserData);
        RenderCallback(xc - x, yc - y, UserData);
        
        if (sigma >= 0)
        {
            sigma += fb2*(1-x);
            --x;
        }
        
        sigma += a2*(4*y+6);
    }
}

} // /namespace Rasterizer


} // /namespace math

} // /namespace sp



// ================================================================================
