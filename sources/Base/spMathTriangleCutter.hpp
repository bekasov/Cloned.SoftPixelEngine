/*
 * Triangle cutter header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_MATH_TRIANGLECUTTER_H__
#define __SP_MATH_TRIANGLECUTTER_H__


#include "Base/spStandard.hpp"
#include "Base/spMath.hpp"

#include <list>


namespace sp
{
namespace math
{


//! Possible results of a triangle clip.
enum ETriangleCutResults
{
    TRIANGLECUT_NONE,               //!< The triangle is not cut.
    TRIANGLECUT_QUAD_AND_TRIANGLE,  //!< The cut brought out a quadrangle and a triangle.
    TRIANGLECUT_TWO_TRIANGLES,      //!< The cut brought out two triangles.
};


struct STriangleCut
{
    dim::triangle3df TrianglePrimary;
    dim::triangle3df TriangleSecondaryA;
    dim::triangle3df TriangleSecondaryB;
};


/**
\deprecated
\todo Move these functions to the "MeshModifier" namespace
*/
namespace TriangleCutter
{

SP_EXPORT ETriangleCutResults cutTriangle(
    const dim::triangle3df &Triangle, const dim::plane3df &ClipPlane, STriangleCut &Cut
);

SP_EXPORT bool cutTriangle(
    const dim::triangle3df &Triangle, const dim::aabbox3df &ClipBox, std::list<dim::triangle3df> &Cut
);

} // /namespace TriangleCutter


} // /namespace math

} // /namespace sp


#endif



// ================================================================================
