/*
 * Collision library header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_MATH_COLLISIONLIB_H__
#define __SP_MATH_COLLISIONLIB_H__


#include "Base/spStandard.hpp"
#include "Base/spMath.hpp"


namespace sp
{
namespace math
{


//! Here you will find each function for collision detection, distance computation and intersection tests.
namespace CollisionLibrary
{

/**
Computes the closest point onto given triangle from an other point to this trianlge.
\param Triangle: 3D triangle for the computation.
\param Point: 3D point to which the closest point is to be computed.
\return Closest 3D point from "Point" to "Triangle".
*/
SP_EXPORT dim::vector3df getClosestPoint(const dim::triangle3df &Triangle, const dim::vector3df &Point);

/**
Computes the closest point from an other point to a triangle if this computed point lies onto the triangle.
\param PointOnTriangle: Resulting 3D point.
\return True if the point lies onto the triangle. In this case "Point" and "PointOnTrianlge" offer
a normal vector to "Trianlge".
*/
SP_EXPORT bool getClosestPointStraight(
    const dim::triangle3df &Triangle, const dim::vector3df &Point, dim::vector3df &PointOnTriangle
);

/**
Computes the closest point from an other point to a box.
\param Box: 3D AABB (Axis-Aliend-Bounding-Box) for the computation.
\param Point: 3D point to which the closest point shall be computed.
\return Closest 3D point from "Point" to "Box" (or rather to a Box's surface).
*/
SP_EXPORT dim::vector3df getClosestPoint(const dim::aabbox3df &Box, const dim::vector3df &Point);

/**
Same like the function above but computes additional the normalized normal vector
by "Point" and the resulting 3D point.
*/
SP_EXPORT dim::vector3df getClosestPoint(
    const dim::aabbox3df &Box, const dim::vector3df &Point, dim::vector3df &Normal
);

/**
Computes the closest point from an other point to a box.
\param Box: 3D OBB (Oriented-Bounding-Box) for the computation.
\param Point: 3D point to which the closest point shall be computed.
\return Closest 3D point from "Point" to "Box" (or rather to a Box's surface).
*/
SP_EXPORT dim::vector3df getClosestPoint(const dim::obbox3df &Box, const dim::vector3df &Point);

/**
Computes the closest line between the given triangle and line.
\param Triangle: 3D triangle for the computation.
\param Line: 3D line to which the closest point is to be computed.
\return Closest 3D line from "Line" to "Triangle" where the start point is onto the triangle.
*/
SP_EXPORT dim::line3df getClosestLine(const dim::triangle3df &Triangle, const dim::line3df &Line);

/**
Computes the closest line between the given triangle and line.
\param Triangle: 3D triangle for the computation.
\param Line: 3D line to which the closest point is to be computed.
\param LineToTriangle: Resulting 3D line.
\return True if the line's start or end point lies onto the triangle. Otherwise false and the resulting 3d line is empty.
*/
SP_EXPORT bool getClosestLineStraight(const dim::triangle3df &Triangle, const dim::line3df &Line, dim::line3df &LineToTriangle);

/**
Computes the closest line from the given quadrangle and line.
\param Quadrangle: 3D quadrangle for the computation.
\param Line: 3D line to which the clsoest point is to be computed.
\return Closest 3D line from "Line" to "Quadrangle" where the start point is onto the quadrangle.
\todo This function has not been tested yet!
*/
SP_EXPORT dim::line3df getClosestLine(const dim::quadrangle3df &Quadrangle, const dim::line3df &Line);

//! Computes the squared distance between the given box and point.
SP_EXPORT f32 getPointBoxDistanceSq(const dim::obbox3df &Box, const dim::vector3df &Point);

//! Computes the distance between the given box and point.
inline f32 getPointBoxDistance(const dim::obbox3df &Box, const dim::vector3df &Point)
{
    return sqrt(getPointBoxDistanceSq(Box, Point));
}

/**
Computes the squared distance between the given box and line.
\todo This function has not been tested yet!
*/
SP_EXPORT f32 getLineBoxDistanceSq(const dim::aabbox3df &Box, const dim::line3df &Line);

//! Computes the distance between the given box and line.
inline f32 getLineBoxDistance(const dim::aabbox3df &Box, const dim::line3df &Line)
{
    return sqrt(getLineBoxDistanceSq(Box, Line));
}

/**
Computes the squared distance between two lines. If you want to have the real distance
take the square-root of the return value.
\param LineA: First line.
\param LineB: Second line.
\param PointP: First point of the closest distance (or rather line).
\param PointQ: Second point of the closest distance (or rather line).
\return Closest distance of the two lines or rather the length of the closest line PQ.
*/
SP_EXPORT f32 getLineLineDistanceSq(
    const dim::line3df &LineA, const dim::line3df &LineB, dim::vector3df &PointP, dim::vector3df &PointQ
);

//! Returns the square root of the result of the getLineLineDistanceSq function.
inline f32 getLineLineDistance(
    const dim::line3df &LineA, const dim::line3df &LineB, dim::vector3df &PointP, dim::vector3df &PointQ)
{
    return sqrt(getLineLineDistanceSq(LineA, LineB, PointP, PointQ));
}

/**
Tests if the two lines are intersecting.
\param LinesA: First line.
\param LinesB: Second line.
\param Intersection: Resulting intersection point of the two lines if an intersection has been deteced.
\return True if an intersection has been detected.
*/
SP_EXPORT bool getLineLineIntersection(
    const dim::line3df &LineA, const dim::line3df &LineB, dim::vector3df &Intersection
);

/**
Computes the intersection of two lines (or rather rays because an intersection will always be detected).
\param A: Start point of the first lines.
\param B: End point of the first lines.
\param C: Start point of the second lines.
\param D: End point of the second lines.
\return Intersection point of the two lines (or rather rays).
*/
SP_EXPORT dim::point2df get2DLineLineIntersectionStraight(
    const dim::point2df &A, const dim::point2df &B, const dim::point2df &C, const dim::point2df &D
);

/* === Intersection tests === */

/**
Tests if a lines intersects with a triangle.
\param Triangle: 3D triangle for the test.
\param Line: 3D line for the test.
\param Intersection: Resulting 3D intersection point.
\return True if an intersection has been detected.
*/
SP_EXPORT bool checkLineTriangleIntersection(
    const dim::triangle3df &Triangle, const dim::line3df &Line, dim::vector3df &Intersection
);

/**
Tests if a line intersects with a sphere.
\param Line: 3D line for the test.
\param SpherePosition: Sphere's 3D position.
\param SphereRadius: Sphere's radius.
\param Intersection: Resulting 3D intersection point.
\return True if an intersection has been detected.
*/
SP_EXPORT bool checkLineSphereIntersection(
    const dim::line3df &Line, const dim::vector3df &SpherePosition, const f32 SphereRadius, dim::vector3df &Intersection
);

/**
Tests if a line intersects with an axis aligned bounding box.
\param Line: 3D line for the test.
\param Box: 3D AABB (Axis Aligned Bounding Box) for the test. In this case you can not
transform the box (e.g. for rotations). Make an inverse transformation to the line.
\param Intersection: Resulting 3D intersection point.
\return True if an intersection has been detected.
*/
SP_EXPORT bool checkLineBoxIntersection(
    const dim::line3df &Line, const dim::aabbox3df &Box, dim::vector3df &Intersection
);

/**
Tests if a triangle intersects with an other triangle.
\param TriangleA: First 3D triangle for the test.
\param TriangleB: Second 3D triangle for the test.
\param Intersection: Resulting 3D line (or rather "intersection cut").
\return True if an intersection has been detected.
*/
SP_EXPORT bool checkTriangleTriangleIntersection(
    const dim::triangle3df &TriangleA, const dim::triangle3df &TriangleB, dim::line3df &Intersection
);

/* === Overlap tests === */

//! Returns true if an intersection between "Line" and "Box" has been detected.
SP_EXPORT bool checkLineBoxOverlap(
    const dim::line3df &Line, const dim::aabbox3df &Box
);

//! Returns true if an intersection between "Plane" and "Box" has been detected.
SP_EXPORT bool checkPlaneBoxOverlap(
    const dim::plane3df &Plane, const dim::aabbox3df &Box
);

//! Returns true if an intersection between "Triangle" and "Box" has been detected.
SP_EXPORT bool checkTriangleBoxOverlap(
    const dim::triangle3df &Triangle, const dim::aabbox3df &Box
);

/**
\return True if an intersection between "BoxA" and "BoxB" has been detected.
OOBs (orientated-bounding-boxes) may be transformed for intersection tests
(With AABBs (axis-aliend-bounding-boxes) you have to transform the opposit intersection
object by the inverse transformation).
*/
SP_EXPORT bool checkOBBoxOBBoxOverlap(
    const dim::obbox3df &BoxA, const dim::obbox3df &BoxB
);

} // /namespace CollisionLibrary


} // /namespace math

} // /namespace sp


#endif



// ================================================================================
