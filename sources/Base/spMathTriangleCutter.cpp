/*
 * Triangle cutter file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "Base/spMathTriangleCutter.hpp"


namespace sp
{
namespace math
{


namespace TriangleCutter
{

SP_EXPORT ETriangleCutResults cutTriangle(
    const dim::triangle3df &Triangle, const dim::plane3df &ClipPlane, STriangleCut &Cut)
{
    /* Temporary variables */
    u32 ClipPoint = 0, ClipEdge = 0;
    dim::vector3df Intersection;
    const dim::vector3df* Pos = &Triangle.PointA;
    dim::line3df Edge;
    
    /* Primary and secondary coordinates */
    dim::vector3df PrimPos[4], ScndPos[4];
    u32 PrimIndex = 0, ScndIndex = 0;
    bool BuildPrim = true;
    
    /* Pass each triangle edge */
    PrimPos[PrimIndex++] = Pos[0];
    
    for (s32 i = 0; i < 3; ++i)
    {
        /* Get current triangle edge */
        Edge.Start  = Pos[i];
        Edge.End    = (i < 2 ? Pos[i + 1] : Pos[0]);
        
        /* Check for an intersection with the clipping plane */
        if (math::Abs(ClipPlane.getPointDistance(Edge.End)) <= math::ROUNDING_ERROR)
        {
            /* Setup result */
            if (ClipPoint == 1)
                return TRIANGLECUT_NONE;
            
            ++ClipPoint;
            
            /* Add the start and the end to the respective triangle */
            PrimPos[PrimIndex++] = Edge.End;
            ScndPos[ScndIndex++] = Edge.End;
            
            BuildPrim = !BuildPrim;
            
            /* Boost corner index */
            ++i;
            continue;
        }
        else if ( ( ClipPlane.checkLineIntersection(Edge.Start, Edge.End, Intersection) ||
                    ClipPlane.checkLineIntersection(Edge.End, Edge.Start, Intersection) ) &&
                    math::Abs(ClipPlane.getPointDistance(Edge.Start)) > math::ROUNDING_ERROR )
        {
            /* Setup result */
            ++ClipEdge;
            
            /* Add the start and the end to the respective triangle */
            PrimPos[PrimIndex++] = Intersection;
            ScndPos[ScndIndex++] = Intersection;
            
            BuildPrim = !BuildPrim;
        }
        
        /* Build the new triangle */
        if (BuildPrim)
        {
            if (PrimIndex < 4)
                PrimPos[PrimIndex++] = Edge.End;
        }
        else
        {
            if (ScndIndex < 4)
                ScndPos[ScndIndex++] = Edge.End;
        }
    }
    
    /* Build final triangles */
    if (ClipPoint == 1 && ClipEdge >= 1)
    {
        Cut.TrianglePrimary     = dim::triangle3df(PrimPos[0], PrimPos[1], PrimPos[2]);
        Cut.TriangleSecondaryA  = dim::triangle3df(ScndPos[0], ScndPos[1], ScndPos[2]);
        
        if (Cut.TriangleSecondaryA.empty() || Cut.TrianglePrimary.empty())
            return TRIANGLECUT_NONE;
        
        return TRIANGLECUT_TWO_TRIANGLES;
    }
    
    if (ClipEdge == 2)
    {
        if (PrimIndex < ScndIndex)
        {
            Cut.TrianglePrimary     = dim::triangle3df(PrimPos[0], PrimPos[1], PrimPos[2]);
            Cut.TriangleSecondaryA  = dim::triangle3df(ScndPos[0], ScndPos[1], ScndPos[2]);
            Cut.TriangleSecondaryB  = dim::triangle3df(ScndPos[0], ScndPos[2], ScndPos[3]);
        }
        else
        {
            Cut.TrianglePrimary     = dim::triangle3df(ScndPos[0], ScndPos[1], ScndPos[2]);
            Cut.TriangleSecondaryA  = dim::triangle3df(PrimPos[0], PrimPos[1], PrimPos[2]);
            Cut.TriangleSecondaryB  = dim::triangle3df(PrimPos[0], PrimPos[2], PrimPos[3]);
        }
        
        if (Cut.TriangleSecondaryA.empty() || Cut.TriangleSecondaryB.empty() || Cut.TrianglePrimary.empty())
            return TRIANGLECUT_NONE;
        
        return TRIANGLECUT_QUAD_AND_TRIANGLE;
    }
    
    return TRIANGLECUT_NONE;
}

SP_EXPORT bool cutTriangle(
    const dim::triangle3df &Triangle, const dim::aabbox3df &ClipBox, std::list<dim::triangle3df> &Cut)
{
    
    //todo
    
    return false;
}

} // /namespace TriangleCutter


} // /namespace math

} // /namespace sp



// ================================================================================
