/*
 * Collision library file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "Base/spMathCollisionLibrary.hpp"


namespace sp
{
namespace math
{


namespace CollisionLibrary
{

/*
 * Static functions
 */

static bool checkRayAABBOverlap1D(f32 start, f32 dir, f32 min, f32 max, f32 &enter, f32 &exit)
{
    /* Ray parallel to direction */
    if (Abs(dir) < ROUNDING_ERROR64)
        return (start >= min && start <= max);
    
    /* Intersection parameters */
    f32 t0 = (min - start) / dir;
    f32 t1 = (max - start) / dir;
    
    /* Sort intersections */
    if (t0 > t1)
        Swap(t0, t1);
    
    /* Check if intervals are disjoint */
    if (t0 > exit || t1 < enter)
        return false;
    
    /* Reduce interval */
    if (t0 > enter)
        enter = t0;
    if (t1 < exit)
        exit = t1;
    
    return true;
}


/*
 * Collision detection
 */

/* Closest point on triangle */

SP_EXPORT dim::vector3df getClosestPoint(
    const dim::triangle3df &Triangle, const dim::vector3df &Point)
{
    dim::vector3df ab(Triangle.PointB - Triangle.PointA);
    dim::vector3df ac(Triangle.PointC - Triangle.PointA);
    
    // Check if P is in vertex region outside A
    dim::vector3df ap = Point - Triangle.PointA;
    f32 d1 = ab.dot(ap);
    f32 d2 = ac.dot(ap);
    if (d1 <= 0.0f && d2 <= 0.0f)
        return Triangle.PointA;
    
    // Check if P is in vertex region outside B
    dim::vector3df bp = Point - Triangle.PointB;
    f32 d3 = ab.dot(bp);
    f32 d4 = ac.dot(bp);
    if (d3 >= 0.0f && d4 <= d3)
        return Triangle.PointB;
    
    // Check if P is in edge region of AB, if so return projection of P onto AB
    f32 vc = d1*d4 - d3*d2;
    if (vc <= 0.0f && d1 > 0.0f && d3 <= 0.0f)
    {
        f32 v = d1 / (d1 - d3);
        return Triangle.PointA + ab * v;
    }
    
    // Check if P is in vertex region outside C
    dim::vector3df cp = Point - Triangle.PointC;
    f32 d5 = ab.dot(cp);
    f32 d6 = ac.dot(cp);
    if (d6 >= 0.0f && d5 <= d6)
        return Triangle.PointC;
    
    // Check if P is in edge region of AC, if so return projection of P onto AC
    f32 vb = d5*d2 - d1*d6;
    if (vb <= 0.0f && d2 > 0.0f && d6 <= 0.0f)
    {
        f32 w = d2 / (d2 - d6);
        return Triangle.PointA + ac * w;
    }
    
    // Check if P is in edge region of BC, if so return projection of P onto BC
    f32 va = d3*d6 - d5*d4;
    if (va <= 0.0f && (d4 - d3) > 0.0f && (d5 - d6) >= 0.0f)
    {
        f32 w = (d4 - d3) / ((d4 - d3) + (d5 - d6));
        return Triangle.PointB + (Triangle.PointC - Triangle.PointB) * w;
    }
    
    // P is inside face region. Compute Q through its barycentric coordinates (u, v, w)
    f32 denom = 1.0f / (va + vb + vc);
    f32 v = vb * denom;
    f32 w = vc * denom;
    
    return Triangle.PointA + ab * v + ac * w;
}

SP_EXPORT bool getClosestPointStraight(
    const dim::triangle3df &Triangle, const dim::vector3df &Point, dim::vector3df &PointOnTriangle)
{
    dim::vector3df ab(Triangle.PointB - Triangle.PointA);
    dim::vector3df ac(Triangle.PointC - Triangle.PointA);
    
    // Check if P is in vertex region outside A
    dim::vector3df ap = Point - Triangle.PointA;
    f32 d1 = ab.dot(ap);
    f32 d2 = ac.dot(ap);
    if (d1 <= 0.0f && d2 <= 0.0f)
        return false;
    
    // Check if P is in vertex region outside B
    dim::vector3df bp = Point - Triangle.PointB;
    f32 d3 = ab.dot(bp);
    f32 d4 = ac.dot(bp);
    if (d3 >= 0.0f && d4 <= d3)
        return false;
    
    // Check if P is in edge region of AB, if so return projection of P onto AB
    f32 vc = d1*d4 - d3*d2;
    if (vc <= 0.0f && d1 >= 0.0f && d3 <= 0.0f)
        return false;
    
    // Check if P is in vertex region outside C
    dim::vector3df cp = Point - Triangle.PointC;
    f32 d5 = ab.dot(cp);
    f32 d6 = ac.dot(cp);
    if (d6 >= 0.0f && d5 <= d6)
        return false;
    
    // Check if P is in edge region of AC, if so return projection of P onto AC
    f32 vb = d5*d2 - d1*d6;
    if (vb <= 0.0f && d2 >= 0.0f && d6 <= 0.0f)
        return false;
    
    // Check if P is in edge region of BC, if so return projection of P onto BC
    f32 va = d3*d6 - d5*d4;
    if (va <= 0.0f && (d4 - d3) >= 0.0f && (d5 - d6) >= 0.0f)
        return false;
    
    // P is inside face region. Compute Q through its barycentric coordinates (u, v, w)
    f32 denom = 1.0f / (va + vb + vc);
    f32 v = vb * denom;
    f32 w = vc * denom;
    
    PointOnTriangle = Triangle.PointA + ab * v + ac * w;
    
    return true;
}


/* Closest point on box */

SP_EXPORT dim::vector3df getClosestPoint(const dim::aabbox3df &Box, const dim::vector3df &Point)
{
    f32 v;
    dim::vector3df PointOnBox;
    
    // For each coordinate axis, if the point coordinate value is outside the box,
    // clamp it to the box, else keep it as is
    for (s32 i = 0; i < 3; ++i)
    {
        v = Point[i];
        
        if (v < Box.Min[i]) v = Box.Min[i];
        if (v > Box.Max[i]) v = Box.Max[i];
        
        PointOnBox[i] = v;
    }
    
    return PointOnBox;
}

SP_EXPORT dim::vector3df getClosestPoint(
    const dim::aabbox3df &Box, const dim::vector3df &Point, dim::vector3df &Normal)
{
    const dim::vector3df PointOnBox(getClosestPoint(Box, Point));
    
    // Calculate normal on the box (resulting from the closest point to the given point)
    Normal = (Point - PointOnBox).normalize();
    
    return PointOnBox;
}

SP_EXPORT dim::vector3df getClosestPoint(const dim::obbox3df &Box, const dim::vector3df &Point)
{
    const dim::vector3df Difference(Point - Box.Center);
    
    dim::vector3df PointOnBox(Box.Center);
    
    /* For each OBB axis */
    for (s32 i = 0; i < 3; ++i)
    {
        f32 Distance = Difference.dot(Box.Axis[i]);
        
        math::Clamp(Distance, -Box.HalfSize[i], Box.HalfSize[i]);
        
        PointOnBox += Box.Axis[i] * Distance;
    }
    
    return PointOnBox;
}


/* Closest line from triangle to other line */

SP_EXPORT dim::line3df getClosestLine(const dim::triangle3df &Triangle, const dim::line3df &Line)
{
    // Temporary vectors
    dim::line3df EdgeLineAB, EdgeLineBC, EdgeLineCA;
    
    const dim::plane3df Plane(Triangle);
    
    // Get closest point between line and all three triangle edges
    const f32 EdgeDistAB = getLineLineDistanceSq(dim::line3df(Triangle.PointA, Triangle.PointB), Line, EdgeLineAB.Start, EdgeLineAB.End);
    const f32 EdgeDistBC = getLineLineDistanceSq(dim::line3df(Triangle.PointB, Triangle.PointC), Line, EdgeLineBC.Start, EdgeLineBC.End);
    const f32 EdgeDistCA = getLineLineDistanceSq(dim::line3df(Triangle.PointC, Triangle.PointA), Line, EdgeLineCA.Start, EdgeLineCA.End);
    
    // Get closest points between line start/end and triangle's plane
    const dim::vector3df PlanePointA(Plane.getClosestPoint(Line.Start));
    const dim::vector3df PlanePointB(Plane.getClosestPoint(Line.End));
    
    const f32 PlaneDistA = getDistanceSq(PlanePointA, Line.Start);
    const f32 PlaneDistB = getDistanceSq(PlanePointB, Line.End);
    
    // Determine which point is closest to line
    f32 Dist = 999999.f;
    const dim::vector3df* ClosestPointA = 0;
    const dim::vector3df* ClosestPointB = 0;
    
    if (Triangle.isPointInside(PlanePointA))
    {
        ClosestPointA = &PlanePointA;
        ClosestPointB = &Line.Start;
        Dist = PlaneDistA;
    }
    if (Triangle.isPointInside(PlanePointB) && PlaneDistB < Dist)
    {
        ClosestPointA = &PlanePointB;
        ClosestPointB = &Line.End;
        Dist = PlaneDistB;
    }
    if (EdgeDistAB < Dist)
    {
        ClosestPointA = &EdgeLineAB.Start;
        ClosestPointB = &EdgeLineAB.End;
        Dist = EdgeDistAB;
    }
    if (EdgeDistBC < Dist)
    {
        ClosestPointA = &EdgeLineBC.Start;
        ClosestPointB = &EdgeLineBC.End;
        Dist = EdgeDistBC;
    }
    if (EdgeDistCA < Dist)
    {
        ClosestPointA = &EdgeLineCA.Start;
        ClosestPointB = &EdgeLineCA.End;
        Dist = EdgeDistCA;
    }
    
    return dim::line3df(*ClosestPointA, *ClosestPointB);
}


/* Closest line from quadrangle to other line */

SP_EXPORT dim::line3df getClosestLine(const dim::quadrangle3df &Quadrangle, const dim::line3df &Line)
{
    // Temporary vectors
    dim::line3df EdgeLineAB, EdgeLineBC, EdgeLineCD, EdgeLineDA;
    
    const dim::plane3df Plane(Quadrangle);
    
    // Get closest point between line and all three triangle edges
    const f32 EdgeDistAB = getLineLineDistanceSq(dim::line3df(Quadrangle.PointA, Quadrangle.PointB), Line, EdgeLineAB.Start, EdgeLineAB.End);
    const f32 EdgeDistBC = getLineLineDistanceSq(dim::line3df(Quadrangle.PointB, Quadrangle.PointC), Line, EdgeLineBC.Start, EdgeLineBC.End);
    const f32 EdgeDistCD = getLineLineDistanceSq(dim::line3df(Quadrangle.PointC, Quadrangle.PointD), Line, EdgeLineCD.Start, EdgeLineCD.End);
    const f32 EdgeDistDA = getLineLineDistanceSq(dim::line3df(Quadrangle.PointD, Quadrangle.PointA), Line, EdgeLineDA.Start, EdgeLineDA.End);
    
    // Get closest points between line start/end and triangle's plane
    const dim::vector3df PlanePointA(Plane.getClosestPoint(Line.Start));
    const dim::vector3df PlanePointB(Plane.getClosestPoint(Line.End));
    
    const f32 PlaneDistA = getDistanceSq(PlanePointA, Line.Start);
    const f32 PlaneDistB = getDistanceSq(PlanePointB, Line.End);
    
    // Determine which point is closest to line
    f32 Dist = 999999.f;
    const dim::vector3df* ClosestPointA = 0;
    const dim::vector3df* ClosestPointB = 0;
    
    if (Quadrangle.isPointInside(PlanePointA))
    {
        ClosestPointA = &PlanePointA;
        ClosestPointB = &Line.Start;
        Dist = PlaneDistA;
    }
    if (Quadrangle.isPointInside(PlanePointB) && PlaneDistB < Dist)
    {
        ClosestPointA = &PlanePointB;
        ClosestPointB = &Line.End;
        Dist = PlaneDistB;
    }
    if (EdgeDistAB < Dist)
    {
        ClosestPointA = &EdgeLineAB.Start;
        ClosestPointB = &EdgeLineAB.End;
        Dist = EdgeDistAB;
    }
    if (EdgeDistBC < Dist)
    {
        ClosestPointA = &EdgeLineBC.Start;
        ClosestPointB = &EdgeLineBC.End;
        Dist = EdgeDistBC;
    }
    if (EdgeDistCD < Dist)
    {
        ClosestPointA = &EdgeLineCD.Start;
        ClosestPointB = &EdgeLineCD.End;
        Dist = EdgeDistCD;
    }
    if (EdgeDistDA < Dist)
    {
        ClosestPointA = &EdgeLineDA.Start;
        ClosestPointB = &EdgeLineDA.End;
        Dist = EdgeDistDA;
    }
    
    return dim::line3df(*ClosestPointA, *ClosestPointB);
}


/* Distance & intersection */

SP_EXPORT f32 getPointBoxDistanceSq(const dim::obbox3df &Box, const dim::vector3df &Point)
{
    const dim::vector3df Direction(Point - Box.Center);
    
    f32 SqDist = 0.0f;
    
    for (s32 i = 0; i < 3; ++i)
    {
        f32 Distance = Direction.dot(Box.Axis[i]);
        f32 Excess = 0.0f;
        
        if (Distance < -Box.HalfSize[i])
            Excess = Distance + Box.HalfSize[i];
        else if (Distance > Box.HalfSize[i])
            Excess = Distance - Box.HalfSize[i];
        
        SqDist += Excess * Excess;
    }
    
    return SqDist;
}

SP_EXPORT f32 getLineBoxDistanceSq(const dim::aabbox3df &Box, const dim::line3df &Line)
{
    dim::line3df ClosestLines[6];
    
    ClosestLines[0] = getClosestLine(Box.getLeftQuad   (), Line);
    ClosestLines[1] = getClosestLine(Box.getRightQuad  (), Line);
    ClosestLines[2] = getClosestLine(Box.getTopQuad    (), Line);
    ClosestLines[3] = getClosestLine(Box.getBottomQuad (), Line);
    ClosestLines[4] = getClosestLine(Box.getFrontQuad  (), Line);
    ClosestLines[5] = getClosestLine(Box.getBackQuad   (), Line);
    
    f32 Dist = math::OMEGA, TempDist;
    
    for (s32 i = 0; i < 6; ++i)
    {
        TempDist = math::getDistanceSq(ClosestLines[i].Start, ClosestLines[i].End);
        if (TempDist < Dist)
            Dist = TempDist;
    }
    
    return Dist;
}

SP_EXPORT f32 getLineLineDistanceSq(
    const dim::line3df &LineA, const dim::line3df &LineB, dim::vector3df &PointP, dim::vector3df &PointQ)
{
    dim::vector3df d1(LineA.getDirection()); // Direction vector of segment S1
    dim::vector3df d2(LineB.getDirection()); // Direction vector of segment S2
    dim::vector3df r(LineA.Start - LineB.Start);
    
    f32 a = d1.dot(d1); // Squared length of segment S1, always nonnegative
    f32 e = d2.dot(d2); // Squared length of segment S2, always nonnegative
    f32 f = d2.dot(r);
    
    f32 s, t;
    
    // Check if either or both segments degenerate into points
    if (a <= ROUNDING_ERROR && e <= ROUNDING_ERROR)
    {
        // Both segments degenerate into points
        s = t = 0.0f;
        PointP = LineA.Start;
        PointQ = LineB.Start;
        return (PointP - PointQ).dot(PointP - PointQ);
    }
    
    if (a <= ROUNDING_ERROR)
    {
        // First segment degenerates into a point
        s = 0.0f;
        t = f / e; // s = 0 => t = (b*s + f) / e = f / e
        Clamp(t, 0.0f, 1.0f);
    }
    else
    {
        f32 c = d1.dot(r);
        
        if (e <= ROUNDING_ERROR)
        {
            // Second segment degenerates into a point
            t = 0.0f;
            s = -c / a;
            Clamp(s, 0.0f, 1.0f); // t = 0 => s = (b*t - c) / a = -c / a
        }
        else
        {
            // The general nondegenerate case starts here
            f32 b = d1.dot(d2);
            f32 denom = a*e - b*b; // Always nonnegative
            
            // If segments not parallel, compute closest point on L1 to L2, and
            // clamp to segment S1. Else pick arbitrary s (here 0)
            if (denom != 0.0f)
            {
                s = (b*f - c*e) / denom;
                Clamp(s, 0.0f, 1.0f);
            }
            else
                s = 0.0f;
            
            // Compute point on L2 closest to S1(s) using
            // t = Dot((P1+D1*s)-P2,D2) / Dot(D2,D2) = (b*s + f) / e
            t = (b*s + f) / e;
            
            // If t in [0,1] done. Else clamp t, recompute s for the new value
            // of t using s = Dot((P2 + D2*t) - P1, D1) / Dot(D1, D1)= (t*b - c) / a
            // and clamp s to [0, 1]
            if (t < 0.0f)
            {
                t = 0.0f;
                s = -c / a;
                Clamp(s, 0.0f, 1.0f);
            }
            else if (t > 1.0f)
            {
                t = 1.0f;
                s = (b - c) / a;
                Clamp(s, 0.0f, 1.0f);
            }
        }
    }
    
    PointP = LineA.Start + d1 * s;
    PointQ = LineB.Start + d2 * t;
    
    return (PointP - PointQ).dot(PointP - PointQ);
}

SP_EXPORT bool getLineLineIntersection(
    const dim::line3df &LineA, const dim::line3df &LineB, dim::vector3df &Intersection)
{
    dim::vector3df PointP, PointQ;
    
    if (getLineLineDistanceSq(LineA, LineB, PointP, PointQ) <= ROUNDING_ERROR && getDistanceSq(PointP, PointQ) <= ROUNDING_ERROR)
    {
        Intersection = (PointP + PointQ) / 2;
        return true;
    }
    
    return false;
}


/* 2D Straight line-line intersection */

SP_EXPORT dim::point2df get2DLineLineIntersectionStraight(
    const dim::point2df &A, const dim::point2df &B, const dim::point2df &C, const dim::point2df &D)
{
    return dim::point2df(
        (  ( A.X*B.Y - A.Y*B.X )*( C.X - D.X ) - ( A.X - B.X )*( C.X*D.Y - C.Y*D.X ) )  /
           ( ( A.X - B.X )*( C.Y - D.Y ) - ( A.Y - B.Y )*( C.X - D.X )  ),
        (  ( A.X*B.Y - A.Y*B.X )*( C.Y - D.Y ) - ( A.Y - B.Y )*( C.X*D.Y - C.Y*D.X ) )  /
           ( ( A.X - B.X )*( C.Y - D.Y ) - ( A.Y - B.Y )*( C.X - D.X )  )
    );
}


/*
 * === Intersection tests ===
 */

/* Line-triangle intersection */

SP_EXPORT bool checkLineTriangleIntersection(
    const dim::triangle3df &Triangle, const dim::line3df &Line, dim::vector3df &Intersection)
{
    dim::vector3df pq(Line.End - Line.Start);
    dim::vector3df pa(Triangle.PointA - Line.Start);
    dim::vector3df pb(Triangle.PointB - Line.Start);
    dim::vector3df pc(Triangle.PointC - Line.Start);
    
    // Check if pq is inside the edges bc, ca and ab
    Intersection.X = pb.dot(pq.cross(pc));
    if (Intersection.X < 0.0f)
        return false;
    
    Intersection.Y = pc.dot(pq.cross(pa));
    if (Intersection.Y < 0.0f)
        return false;
    
    Intersection.Z = pa.dot(pq.cross(pb));
    if (Intersection.Z < 0.0f)
        return false;
    
    return dim::plane3df(Triangle).checkLineIntersection(Line.Start, Line.End, Intersection);
}


/* Line-sphere intersection */

SP_EXPORT bool checkLineSphereIntersection(
    const dim::line3df &Line, const dim::vector3df &SpherePosition, const f32 SphereRadius, dim::vector3df &Intersection)
{
    // Temporary variables
    dim::vector3df d(Line.getDirection().normalize());
    dim::vector3df m(Line.Start - SpherePosition);
    f32 t;
    
    // Compute the scalar products
    f32 b = m.dot(d);
    f32 c = m.dot(m) - SphereRadius*SphereRadius;
    
    // Exit if r's origin outside s (c > 0) and r pointing away from s (b > 0)
    if (c > 0.0f && b > 0.0f)
        return false;
    
    f32 discr = b*b - c;
    
    // A negative discriminant corresponds to ray missing sphere
    if (discr < 0.0f)
        return false;
    
    // Ray now found to intersect sphere, compute smallest t value of intersection
    t = -b - sqrt(discr);
    
    // If t is negative, ray started inside sphere so clamp t to 0 (zero)
    if (t < 0.0f)
        t = 0.0f;
    
    // Compute the intersection point
    Intersection = Line.Start + d * t;
    
    // An intersection has been detected
    return true;
}


/* Line-box intersection */

SP_EXPORT bool checkLineBoxIntersection(
    const dim::line3df &Line, const dim::aabbox3df &Box, dim::vector3df &Intersection)
{
    // Temporary variables
    dim::vector3df Direction(Line.getDirection().normalize());
    
    f32 p[3] = { Line.Start.X, Line.Start.Y, Line.Start.Z };
    f32 d[3] = { Direction.X, Direction.Y, Direction.Z };
    f32 min[3] = { Box.Min.X, Box.Min.Y, Box.Min.Z };
    f32 max[3] = { Box.Max.X, Box.Max.Y, Box.Max.Z };
    
    f32 tmin = 0.0f;
    f32 tmax = 1000000.0f;
    
    // Loop for all three slabs
    for (s32 i = 0; i < 3; ++i)
    {
        if (Abs(d[i]) < ROUNDING_ERROR)
        {
            // Ray is parallel tp slab. No hit if origin not within slab
            if (p[i] < min[i] || p[i] > max[i])
                return false;
        }
        else
        {
            // Compute intersection t value of ray with near and far plane of slab
            f32 ood = 1.0f / d[i];
            f32 t1 = (min[i] - p[i]) * ood;
            f32 t2 = (max[i] - p[i]) * ood;
            
            // Make t1 be intersection with near plane, t2 with far plane
            if (t1 > t2)
                Swap(t1, t2);
            
            // Compute the intersection of slab intersection intervals
            tmin = Max(tmin, t1);
            tmax = Min(tmax, t2);
            
            // Exit with no collision as soon as slab intersection becomes empty
            if (tmin > tmax)
                return false;
        }
    }
    
    // Compute the intersection point
    Intersection = Line.Start + Direction * tmin;
    
    // An intersection has been detected
    return true;
}


/* Triangle-triangle intersection */

SP_EXPORT bool checkTriangleTriangleIntersection(
    const dim::triangle3df &TriangleA, const dim::triangle3df &TriangleB, dim::line3df &Intersection)
{
    /* Internal macros */
    #define mcrAddPoint                                     \
        if (TestIndex < 2 && !Point[0].equal(TempPoint))    \
        {                                                   \
            Point[TestIndex] = TempPoint;                   \
            ++TestIndex;                                    \
        }
    #define mcrFinalCheck                       \
        if (TestIndex > 1)                      \
        {                                       \
            Intersection.Start  = Point[0];     \
            Intersection.End    = Point[1];     \
            return true;                        \
        }
    #define mcrIntersect(t, a, b)                                   \
        Line = dim::line3df(a, b);                                  \
        if (checkLineTriangleIntersection(t, Line, TempPoint))      \
        {                                                           \
            mcrAddPoint                                             \
        }                                                           \
        else                                                        \
        {                                                           \
            Line = dim::line3df(b, a);                              \
            if (checkLineTriangleIntersection(t, Line, TempPoint))  \
                mcrAddPoint                                         \
        }
    #define mcrIntersectA(a, b) \
        mcrIntersect(TriangleA, TriangleB.a, TriangleB.b)
    #define mcrIntersectB(a, b) \
        mcrIntersect(TriangleB, TriangleA.a, TriangleA.b)
    
    /* Temporary variables */
    dim::line3df Line;
    
    dim::vector3df TempPoint;
    
    s32 TestIndex = 0;
    dim::vector3df Point[2];
    
    /* Get all intersections with the triangle A and the three edges of triangle B  */
    mcrIntersectA(PointA, PointB);
    mcrIntersectA(PointB, PointC);
    mcrIntersectA(PointC, PointA);
    
    /* Check for intersection of triangle A */
    mcrFinalCheck;
    
    /* Get all intersections with the triangle B and the three edges of triangle A  */
    mcrIntersectB(PointA, PointB);
    mcrIntersectB(PointB, PointC);
    mcrIntersectB(PointC, PointA);
    
    /* Check for intersection of triangle B */
    mcrFinalCheck;
    
    /* Terminate the macros */
    #undef mcrAddPoint
    #undef mcrFinalCheck
    #undef mcrIntersect
    #undef mcrIntersectA
    #undef mcrIntersectB
    
    /* No intersection has been detected */
    return false;
}


/*
 * === Overlap tests ===
 */

/* Line-/ plane-/ triangle- box overlap tests */

SP_EXPORT bool checkLineBoxOverlap(
    const dim::line3df &Line, const dim::aabbox3df &Box)
{
    // Check if the line is inside the box
    if ( ( Line.Start.X >= Box.Min.X && Line.Start.Y >= Box.Min.Y && Line.Start.Z >= Box.Min.Z && 
           Line.Start.X <  Box.Max.X && Line.Start.Y <  Box.Max.Y && Line.Start.Z <  Box.Max.Z ) ||
         ( Line.End.X   >= Box.Min.X && Line.End.Y   >= Box.Min.Y && Line.End.Z   >= Box.Min.Z && 
           Line.End.X   <  Box.Max.X && Line.End.Y   <  Box.Max.Y && Line.End.Z   <  Box.Max.Z ) )
    {
        return true;
    }
    
    // Check the ray-AABB intersection
    f32 enter = 0.0f, exit = 1.0f;
    dim::vector3df dir(Line.getDirection());
    
    if (!checkRayAABBOverlap1D(Line.Start.X, dir.X, Box.Min.X, Box.Max.X, enter, exit))
        return false;
    if (!checkRayAABBOverlap1D(Line.Start.Y, dir.Y, Box.Min.Y, Box.Max.Y, enter, exit))
        return false;
    if (!checkRayAABBOverlap1D(Line.Start.Z, dir.Z, Box.Min.Z, Box.Max.Z, enter, exit))
        return false;
    
    return true;
}

SP_EXPORT bool checkPlaneBoxOverlap(
    const dim::plane3df &Plane, const dim::aabbox3df &Box)
{
    // These two lines not necessary with a (center, extents) AABB representation
    dim::vector3df c((Box.Min + Box.Max) * 0.5f);   // Compute AABB center
    dim::vector3df e(Box.Max - c);                  // Compute positive extents
    
    // Compute the projection interval readius of b onto L(t) = b.c + t * p.n
    f32 r = e.X * Abs(Plane.Normal.X) + e.Y * Abs(Plane.Normal.Y) + e.Z * Abs(Plane.Normal.Z);
    
    // Compute distance of box center from plane
    f32 s = Plane.Normal.dot(c) - Plane.Distance;
    
    // Intersection occurs when distance s falls within [-r, +r] interval
    return Abs(s) <= r;
}

SP_EXPORT bool checkTriangleBoxOverlap(
    const dim::triangle3df &Triangle, const dim::aabbox3df &Box)
{
    // First checking the simple tests
    if ( ( Triangle.PointA >= Box.Min && Triangle.PointA <= Box.Max ) ||
         ( Triangle.PointB >= Box.Min && Triangle.PointB <= Box.Max ) ||
         ( Triangle.PointC >= Box.Min && Triangle.PointC <= Box.Max ) )
    {
        return true;
    }
    
    // Create a temporary triangle box
    dim::aabbox3df TriangleBox(Triangle.getBox());
    
    // Check if the two boxes do intersect
    return TriangleBox.Min <= Box.Max && TriangleBox.Max >= Box.Min && checkPlaneBoxOverlap(Triangle, Box);
}


/* Obb-Obb overlap test */

SP_EXPORT bool checkOBBoxOBBoxOverlap(
    const dim::obbox3df &BoxA, const dim::obbox3df &BoxB)
{
    s32 i, j;
    f32 ra, rb;
    f32 R[3][3], AbsR[3][3];
    
    // Compute rotation matrix expressing BoxB in BoxA's coordinate frame
    for (i = 0; i < 3; ++i)
    {
        for (j = 0; j < 3; ++j)
            R[i][j] = BoxA.Axis[i].dot(BoxB.Axis[j]);
    }
    
    // Compute translation vector t
    dim::vector3df t = BoxB.Center - BoxA.Center;
    
    // Bring translation into BoxA's coordinate frame
    t = dim::vector3df(t.dot(BoxA.Axis.X), t.dot(BoxA.Axis.Y), t.dot(BoxA.Axis.Z));
    
    // Compute common subexpressions. Add in an epsilon term to counteract arithmetic errors
    // when two edges are parallel and their cross product is (near) null
    for (i = 0; i < 3; ++i)
    {
        for (j = 0; j < 3; ++j)
            AbsR[i][j] = math::Abs(R[i][j]) + math::ROUNDING_ERROR;
    }
    
    // Test axes L = A0, L = A1, L = A2
    for (i = 0; i < 3; ++i)
    {
        ra = BoxA.HalfSize[i];
        rb = BoxB.HalfSize.X * AbsR[i][0] + BoxB.HalfSize.Y * AbsR[i][1] + BoxB.HalfSize.Z * AbsR[i][2];
        if (math::Abs(t[i]) > ra + rb) return false;
    }
    
    // Test axes L = B0, L = B1, L = B2
    for (i = 0; i < 3; ++i)
    {
        ra = BoxA.HalfSize.X * AbsR[0][i] + BoxA.HalfSize.Y * AbsR[1][i] + BoxA.HalfSize.Z * AbsR[2][i];
        rb = BoxB.HalfSize[i];
        if (math::Abs(t.X * R[0][i] + t.Y * R[1][i] + t.Z * R[2][i]) > ra + rb) return false;
    }
    
    // Test axis L = A0 x B0
    ra = BoxA.HalfSize.Y * AbsR[2][0] + BoxA.HalfSize.Z * AbsR[1][0];
    rb = BoxB.HalfSize.Y * AbsR[0][2] + BoxB.HalfSize.Z * AbsR[0][1];
    if (math::Abs(t.Z * R[1][0] - t.Y * R[2][0]) > ra + rb) return false;
    
    // Test axis L = A0 x B1
    ra = BoxA.HalfSize[1] * AbsR[2][1] + BoxA.HalfSize[2] * AbsR[1][1];
    rb = BoxB.HalfSize[0] * AbsR[0][2] + BoxB.HalfSize[2] * AbsR[0][0];
    if (math::Abs(t[2] * R[1][1] - t[1] * R[2][1]) > ra + rb) return false;
    
    // Test axis L = A0 x B2
    ra = BoxA.HalfSize[1] * AbsR[2][2] + BoxA.HalfSize[2] * AbsR[1][2];
    rb = BoxB.HalfSize[0] * AbsR[0][1] + BoxB.HalfSize[1] * AbsR[0][0];
    if (math::Abs(t[2] * R[1][2] - t[1] * R[2][2]) > ra + rb) return false;
    
    // Test axis L = A1 x B0
    ra = BoxA.HalfSize[0] * AbsR[2][0] + BoxA.HalfSize[2] * AbsR[0][0];
    rb = BoxB.HalfSize[1] * AbsR[1][2] + BoxB.HalfSize[2] * AbsR[1][1];
    if (math::Abs(t[0] * R[2][0] - t[2] * R[0][0]) > ra + rb) return false;
    
    // Test axis L = A1 x B1
    ra = BoxA.HalfSize[0] * AbsR[2][1] + BoxA.HalfSize[2] * AbsR[0][1];
    rb = BoxB.HalfSize[0] * AbsR[1][2] + BoxB.HalfSize[2] * AbsR[1][0];
    if (math::Abs(t[0] * R[2][1] - t[2] * R[0][1]) > ra + rb) return false;
    
    // Test axis L = A1 x B2
    ra = BoxA.HalfSize[0] * AbsR[2][2] + BoxA.HalfSize[2] * AbsR[0][2];
    rb = BoxB.HalfSize[0] * AbsR[1][1] + BoxB.HalfSize[1] * AbsR[1][0];
    if (math::Abs(t[0] * R[2][2] - t[2] * R[0][2]) > ra + rb) return false;
    
    // Test axis L = A2 x B0
    ra = BoxA.HalfSize[0] * AbsR[1][0] + BoxA.HalfSize[1] * AbsR[0][0];
    rb = BoxB.HalfSize[1] * AbsR[2][2] + BoxB.HalfSize[2] * AbsR[2][1];
    if (math::Abs(t[1] * R[0][0] - t[0] * R[1][0]) > ra + rb) return false;
    
    // Test axis L = A2 x B1
    ra = BoxA.HalfSize[0] * AbsR[1][1] + BoxA.HalfSize[1] * AbsR[0][1];
    rb = BoxB.HalfSize[0] * AbsR[2][2] + BoxB.HalfSize[2] * AbsR[2][0];
    if (math::Abs(t[1] * R[0][1] - t[0] * R[1][1]) > ra + rb) return false;
    
    // Test axis L = A2 x B2
    ra = BoxA.HalfSize[0] * AbsR[1][2] + BoxA.HalfSize[1] * AbsR[0][2];
    rb = BoxB.HalfSize[0] * AbsR[2][1] + BoxB.HalfSize[1] * AbsR[2][0];
    if (math::Abs(t[1] * R[0][2] - t[0] * R[1][2]) > ra + rb) return false;
    
    // Since no separating axis found, the OBBs must be intersecting
    return true;
}

} // /namespace CollisionLibrary


} // /namespace math

} // /namespace sp



// ================================================================================
