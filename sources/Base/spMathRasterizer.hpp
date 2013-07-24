/*
 * Rasterizer header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_MATH_RASTERIZER_H__
#define __SP_MATH_RASTERIZER_H__


#include "Base/spStandard.hpp"
#include "Base/spMath.hpp"

#include <boost/function.hpp>


namespace sp
{
namespace math
{


//! Callback function interface for rasterization without vertices.
typedef boost::function<void (s32 x, s32 y, void* UserData)> RenderPixelCallback;


/**
Rasterizer vertex class interface. Write your own vertex class and use the same interface.
You do not need to inherit from this class but this is also allowed.
Then use your vertex class to rasterize triangles or other primitives.
\see Rasterizer
*/
class SP_EXPORT RasterizerVertex
{
    
    public:
        
        RasterizerVertex()
        {
        }
        virtual ~RasterizerVertex()
        {
        }
        
        /* Operators */
        
        virtual RasterizerVertex& operator = (const RasterizerVertex &Other) = 0;
        
        virtual RasterizerVertex& operator += (const RasterizerVertex &Other) = 0;
        virtual RasterizerVertex& operator -= (const RasterizerVertex &Other) = 0;
        
        virtual RasterizerVertex& operator *= (f32 Factor) = 0;
        virtual RasterizerVertex& operator /= (f32 Factor) = 0;
        
        /* Functions */
        
        //! Returns the X screen coordinate.
        virtual s32 getScreenCoordX() const = 0;
        //! Returns the Y screen coordinate.
        virtual s32 getScreenCoordY() const = 0;
        
};


//! Namespace for rasterization templates.
namespace Rasterizer
{

//! Only used internally for the rasterizeTriangle template function.
template <class VtxT> void computeRasterScanline(
    const VtxT* (&v)[3], s32 y, s32 yStart, s32 yMiddle, VtxT &a, VtxT &b)
{
    /* Get delta-y as float */
    f32 dy = static_cast<f32>(y - yStart);
    
    /* Compute interpolation factor */
    f32 Factor = dy / (v[2]->getScreenCoordY() - v[0]->getScreenCoordY());
    
    /* Interpolate left vertex: a = v0 + (v2 - v0) * Factor */
    math::lerp(a, *v[0], *v[2], Factor);
    
    if (y < yMiddle)
    {
        /* Compute interpolation factor */
        Factor = dy / (v[1]->getScreenCoordY() - v[0]->getScreenCoordY());
        
        /* Interpolate right vertex: b = v0 + (v1 - v0) * Factor */
        math::lerp(b, *v[0], *v[1], Factor);
    }
    else
    {
        /* Update delta-y */
        dy = static_cast<f32>(y - yMiddle);
        
        /* Compute interpolation factor */
        Factor = dy / (v[2]->getScreenCoordY() - v[1]->getScreenCoordY());
        
        /* Interpolate right vertex: b = v1 + (v2 - v1) * Factor */
        math::lerp(b, *v[1], *v[2], Factor);
    }
}

/**
Rasterizes a triangle onto a 2D screen.
\tparam VtxT Specifies your own vertex class. See the RasterizerVertex class and implement your
own vertex class with the same interface.
\param[in] RenderPixelCallback Specifies the pixel rendering callback. Without this callback
the function renders nothing. Use this to draw a pixel onto a screen or image buffer with the given x and y coordainte.
\param[in] VertexA First vertex for the triangle.
\param[in] VertexB Second vertex for the triangle.
\param[in] VertexC Third vertex for the triangle.
\param[in] UserData Any user data which will be passed to the render callback.
\see RasterizerVertex
*/
template <class VtxT> void rasterizeTriangle(
    const boost::function<void (s32 x, s32 y, const VtxT &Vertex, void* UserData)> &RenderPixelCallback,
    const VtxT &VertexA, const VtxT &VertexB, const VtxT &VertexC, void* UserData = 0)
{
    /* Store vertices as pointers in array */
    const VtxT* v[3] = { &VertexA, &VertexB, &VertexC };
    
    /* Sort the vertices in dependet of the y axis */
    if (v[0]->getScreenCoordY() > v[1]->getScreenCoordY())
        Swap(v[0], v[1]);
    if (v[0]->getScreenCoordY() > v[2]->getScreenCoordY())
        Swap(v[0], v[2]);
    if (v[1]->getScreenCoordY() > v[2]->getScreenCoordY())
        Swap(v[1], v[2]);
    
    /* Get screen coordinates */
    s32 v0x = v[0]->getScreenCoordX();
    s32 v1x = v[1]->getScreenCoordX();
    s32 v2x = v[2]->getScreenCoordX();
    
    /* Set the vertices position */
    s32 yStart  = v[0]->getScreenCoordY();
    s32 yMiddle = v[1]->getScreenCoordY();
    s32 yEnd    = v[2]->getScreenCoordY();
    
    /* Compute the dimensions */
    s32 yMiddleStart    = yMiddle - yStart;
    s32 yEndMiddle      = yEnd - yMiddle;
    s32 yEndStart       = yEnd - yStart;
    
    /* Iteration variables */
    s32 x, y, xStart, xEnd;
    VtxT lside, rside, step, cur;
    
    /* Rater each scanline from top to bottom */
    for (y = yStart; y < yEnd; ++y)
    {
        /* Compute the scanline dimension */
        if (y < yMiddle)
            xStart = v0x + ( v1x - v0x ) * ( y - yStart ) / yMiddleStart;
        else if (y > yMiddle)
            xStart = v1x + ( v2x - v1x ) * ( y - yMiddle ) / yEndMiddle;
        else
            xStart = v1x;
        
        xEnd = v0x + ( v2x - v0x ) * ( y - yStart ) / yEndStart;
        
        /* Compute the polygon sides */
        if (xStart > xEnd)
        {
            std::swap(xStart, xEnd);
            computeRasterScanline(v, y, yStart, yMiddle, lside, rside);
        }
        else
            computeRasterScanline(v, y, yStart, yMiddle, rside, lside);
        
        /* Compute the steps */
        f32 dx = static_cast<f32>(xEnd - xStart);
        
        step = rside;
        step -= lside;
        step /= dx;
        
        cur = lside;
        
        /* Render each pixel in scanline */
        for (x = xStart; x < xEnd; ++x)
        {
            RenderPixelCallback(x, y, cur, UserData);
            
            /* Interpolate vertex between left- and right side of scaline */
            cur += step;
        }
    }
}

/**
Rasterizes a line onto a 2D screen using the "Bresenham" algorithm.
\tparam VtxT Specifies your own vertex class. See the RasterizerVertex class and implement your
own vertex class with the same interface.
\param[in] RenderPixelCallback Specifies the pixel rendering callback. Without this callback
the function renders nothing. Use this to draw a pixel onto a screen or image buffer with the given x and y coordainte.
\param[in] VertexA First vertex for the line.
\param[in] VertexB Second vertex for the line.
\param[in] UserData Any user data which will be passed to the render callback.
\see RasterizerVertex
*/
template <class VtxT> void rasterizeLine(
    const boost::function<void (s32 x, s32 y, const VtxT &Vertex, void* UserData)> &RenderPixelCallback,
    const VtxT &VertexA, const VtxT &VertexB, void* UserData = 0)
{
    /* Store screen coordinates */
    s32 x1 = VertexA.getScreenCoordX();
    s32 y1 = VertexA.getScreenCoordY();
    
    s32 x2 = VertexB.getScreenCoordX();
    s32 y2 = VertexB.getScreenCoordY();
    
    /* Pre-Compuations of the "Bresenham" algorithm */
    s32 dx = x2 - x1;
    s32 dy = y2 - y1;
    
    s32 incx = (dx > 0) ? 1 : (dx < 0) ? -1 : 0;
    s32 incy = (dy > 0) ? 1 : (dy < 0) ? -1 : 0;
    
    if (dx < 0) dx = -dx;
    if (dy < 0) dy = -dy;
    
    s32 pdx, pdy, ddx, ddy, es, el;
    
    if (dx > dy)
    {
        pdx = incx;
        pdy = 0;
        ddx = incx;
        ddy = incy;
        es  = dy;
        el  = dx;
    }
    else
    {
        pdx = 0;
        pdy = incy;
        ddx = incx;
        ddy = incy;
        es  = dx;
        el  = dy;
    }
    
    if (el == 0)
        return;
    
    s32 x   = x1;
    s32 y   = y1;
    s32 err = el/2;
    
    /* Compute the steps */
    VtxT step, cur;
    
    step = VertexB;
    step -= VertexA;
    step /= static_cast<f32>(el);
    
    cur = VertexA;
    
    /* Render each pixel of the line */
    for (s32 t = 0; t < el; ++t)
    {
        /* Render pixel */
        RenderPixelCallback(x, y, cur, UserData);
        
        cur += step;
        
        /* Move to next pixel */
        err -= es;
        if (err < 0)
        {
            err += el;
            x += ddx;
            y += ddy;
        }
        else
        {
            x += pdx;
            y += pdy;
        }
    }
}

/**
Rasterizes a triangle onto a 2D screen.
\param[in] RenderCallback Specifies the pixel rendering callback. Without this callback
the function renders nothing. Use this to draw a pixel onto a screen or image buffer with the given x and y coordainte.
\param[in] PointA Specifies the first 2D point of the triangle.
\param[in] PointB Specifies the second 2D point of the triangle.
\param[in] PointC Specifies the third 2D point of the triangle.
\param[in] UserData Any user data which will be passed to the render callback.
\note This function does not interpolate any values between the triangle points.
For this feature use the template variant.
*/
SP_EXPORT void rasterizeTriangle(
    const RenderPixelCallback &RenderCallback,
    dim::point2di PointA, dim::point2di PointB, dim::point2di PointC,
    void* UserData = 0
);

/**
Rasterizes a circle onto a 2D screen using the "Bresenham" algorithm.
\param[in] RenderCallback Specifies the pixel rendering callback. Without this callback
the function renders nothing. Use this to draw a pixel onto a screen or image buffer with the given x and y coordainte.
\param[in] Position Specifies the 2D position of the circle.
\param[in] Radius Specifies the radius of the circle.
\param[in] UserData Any user data which will be passed to the render callback.
*/
SP_EXPORT void rasterizeCircle(
    const RenderPixelCallback &RenderCallback, const dim::point2di &Position, s32 Radius, void* UserData = 0
);

/**
Rasterizes an ellipse onto a 2D screen using the "Bresenham" algorithm.
\param[in] RenderCallback Specifies the pixel rendering callback. Without this callback
the function renders nothing. Use this to draw a pixel onto a screen or image buffer with the given x and y coordainte.
\param[in] Position Specifies the 2D position of the ellipse.
\param[in] Radius Specifies the radius of the ellipse.
\param[in] UserData Any user data which will be passed to the render callback.
*/
SP_EXPORT void rasterizeEllipse(
    const RenderPixelCallback &RenderCallback, const dim::point2di &Position, const dim::size2di &Radius, void* UserData = 0
);

} // /namespace Rasterizer


} // /namespace math

} // /namespace sp


#endif



// ================================================================================
