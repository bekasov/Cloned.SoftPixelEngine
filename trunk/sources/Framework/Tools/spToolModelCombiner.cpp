/*
 * Model combiner file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "Framework/Tools/spToolModelCombiner.hpp"

#ifdef SP_COMPILE_WITH_MESHBOOLEANOPERATOR


#include "Base/spMathCollisionLibrary.hpp"


namespace sp
{
namespace tool
{


/*
 * Internal functions
 */

bool cmpVector(const dim::vector3df &VecA, const dim::vector3df &VecB)
{
    if (!math::Equal(VecA.X, VecB.X))
        return VecA.X < VecB.X;
    else if (!math::Equal(VecA.Y, VecB.Y))
        return VecA.Y < VecB.Y;
    return VecA.Z < VecB.Z;
}

bool cmpPlane(const dim::plane3df &PlaneA, const dim::plane3df &PlaneB)
{
    if (!math::Equal(PlaneA.Distance, PlaneB.Distance))
        return PlaneA.Distance < PlaneB.Distance;
    return cmpVector(PlaneA.Normal, PlaneB.Normal);
}

bool cmpModelTrianglePlane(const MeshBooleanOperator::STriangle &obj1, const MeshBooleanOperator::STriangle &obj2)
{
    return cmpPlane(obj1.Plane, obj2.Plane);
}

bool cmpModelCutLinePlane(const MeshBooleanOperator::SLine &obj1, const MeshBooleanOperator::SLine &obj2)
{
    return cmpPlane(obj1.Plane, obj2.Plane);
}

bool cmpModelCutVertexPosition(const MeshBooleanOperator::SVertex &obj1, const MeshBooleanOperator::SVertex &obj2)
{
    return cmpVector(obj1.Position, obj2.Position);
}

bool cmpTrianglePointDistance(
    const MeshBooleanOperator::SModel::STrianglePointDistance &obj1,
    const MeshBooleanOperator::SModel::STrianglePointDistance &obj2)
{
    return obj1.Distance < obj2.Distance;
}


/*
 * MeshBooleanOperator class
 */

bool MeshBooleanOperator::CutFrontSide_   = false;
f32 MeshBooleanOperator::Precision_       = 0.00001f;

MeshBooleanOperator::MeshBooleanOperator() :
    Method_         (COMBINATION_UNION  ),
    Model_          (0                  ),
    OppositModel_   (0                  )
{
}
MeshBooleanOperator::~MeshBooleanOperator()
{
}

void MeshBooleanOperator::combineModels(
    scene::Mesh* MeshA, scene::Mesh* MeshB, const EModelCombinations Method)
{
    SModel ModelA(MeshA);
    SModel ModelB(MeshB);
    
    const bool FrontSideA = (Method == COMBINATION_UNION || Method == COMBINATION_DIFFERENCE);
    const bool FrontSideB = (Method == COMBINATION_UNION || Method == COMBINATION_DIFFERENCEINV);
    
    cutModel(&ModelA, &ModelB, FrontSideA);
    cutModel(&ModelB, &ModelA, FrontSideB);
    
    CutFrontSide_ = FrontSideA;
    ModelA.build();
    
    CutFrontSide_ = FrontSideB;
    ModelB.build();
}

void MeshBooleanOperator::setPrecision(const f32 Precision)
{
    Precision_ = Precision;
}
f32 MeshBooleanOperator::getPrecision()
{
    return Precision_;
}


/*
 * ======= Private: =======
 */

void MeshBooleanOperator::cutModel(SModel* ModA, SModel* ModB, bool CutFrontSide)
{
    if (!ModA || !ModB)
        return;
    
    Model_          = ModA;
    OppositModel_   = ModB;
    CutFrontSide_   = CutFrontSide;
    
    Model_->createFaces();
    Model_->computeCutLines(OppositModel_);
    Model_->createVertices(OppositModel_);
    Model_->generateDeltaConnections();
}


/*
 * SVertex structure
 */

MeshBooleanOperator::SVertex::SVertex()
    : Surface(0), Index(0), Fog(0.0f)
{
}
MeshBooleanOperator::SVertex::SVertex(SModel* Mod, u32 VertexSurface, u32 VertexIndex)
{
    set(Mod, VertexSurface, VertexIndex);
}
MeshBooleanOperator::SVertex::SVertex(const SVertex &other)
{
    *this = other;
}
MeshBooleanOperator::SVertex::~SVertex()
{
}

bool MeshBooleanOperator::SVertex::operator == (const SVertex &other) const
{
    for (s32 i = 0; i < MAX_COUNT_OF_TEXTURES; ++i)
    {
        if (!TexCoord[i].equal(other.TexCoord[i]))
            return false;
    }
    
    return (
        Surface     == other.Surface    &&
        Index       == other.Index      &&
        Position    == other.Position   &&
        Normal      == other.Normal     &&
        Color       == other.Color      &&
        Fog         == other.Fog
    );
}

bool MeshBooleanOperator::SVertex::operator != (const SVertex &other) const
{
    return !(*this == other);
}

MeshBooleanOperator::SVertex& MeshBooleanOperator::SVertex::operator = (const SVertex &other)
{
    Surface     = other.Surface;
    Index       = other.Index;
    
    Position    = other.Position;
    Normal      = other.Normal;
    Color       = other.Color;
    Fog         = other.Fog;
    
    for (s32 i = 0; i < MAX_COUNT_OF_TEXTURES; ++i)
        TexCoord[i] = other.TexCoord[i];
    
    return *this;
}

#define mcrVertexOperator(x)                                    \
    SVertex Vertex;                                             \
    for (s32 i = 0; i < MAX_COUNT_OF_TEXTURES; ++i)             \
        Vertex.TexCoord[i] = TexCoord[i] x other.TexCoord[i];   \
    for (s32 i = 0; i < 4; ++i)                                 \
        Vertex.Color[i] = Color[i] x other.Color[i];            \
    Vertex.Fog = Fog x other.Fog;                               \
    return Vertex;

MeshBooleanOperator::SVertex MeshBooleanOperator::SVertex::operator + (const SVertex &other) const
{
    mcrVertexOperator(+);
}
MeshBooleanOperator::SVertex MeshBooleanOperator::SVertex::operator - (const SVertex &other) const
{
    mcrVertexOperator(-);
}
MeshBooleanOperator::SVertex MeshBooleanOperator::SVertex::operator * (const SVertex &other) const
{
    mcrVertexOperator(*);
}
MeshBooleanOperator::SVertex MeshBooleanOperator::SVertex::operator / (const SVertex &other) const
{
    mcrVertexOperator(/);
}

#undef mcrVertexOperator

#define mcrVertexOperator(x)                            \
    SVertex Vertex;                                     \
    for (s32 i = 0; i < MAX_COUNT_OF_TEXTURES; ++i)     \
        Vertex.TexCoord[i] = TexCoord[i] x Size;        \
    for (s32 i = 0; i < 4; ++i)                         \
        Vertex.Color[i] = (u8)( (f32)Color[i] x Size ); \
    Vertex.Fog = Fog x Size;                            \
    return Vertex;

MeshBooleanOperator::SVertex MeshBooleanOperator::SVertex::operator + (const f32 Size) const
{
    mcrVertexOperator(+);
}
MeshBooleanOperator::SVertex MeshBooleanOperator::SVertex::operator - (const f32 Size) const
{
    mcrVertexOperator(-);
}
MeshBooleanOperator::SVertex MeshBooleanOperator::SVertex::operator * (const f32 Size) const
{
    mcrVertexOperator(*);
}
MeshBooleanOperator::SVertex MeshBooleanOperator::SVertex::operator / (const f32 Size) const
{
    mcrVertexOperator(/);
}

#undef mcrVertexOperator


MeshBooleanOperator::SVertex& MeshBooleanOperator::SVertex::set(
    SModel* Mod, u32 VertexSurface, u32 VertexIndex)
{
    Surface = VertexSurface;
    Index   = VertexIndex;
    
    video::MeshBuffer* CurSurface = Mod->Mesh->getMeshBuffer(Surface);
    
    Position    = CurSurface->getVertexCoord(Index);
    Normal      = CurSurface->getVertexNormal(Index);
    Color       = CurSurface->getVertexColor(Index);
    Fog         = CurSurface->getVertexFog(Index);
    
    Position    = Mod->Matrix * Position;
    Normal      = (Mod->NormalMatrix * Normal).normalize();
    
    for (s32 i = 0; i < MAX_COUNT_OF_TEXTURES; ++i)
        TexCoord[i] = CurSurface->getVertexTexCoord(Index, i);
    
    return *this;
}

MeshBooleanOperator::SVertex& MeshBooleanOperator::SVertex::interpolate(
    const dim::vector3df &Pos, const SVertex &A, const SVertex &B, const SVertex &C)
{
    // Temporary variables
    const dim::vector3df u(B.Position - A.Position);
    const dim::vector3df v(C.Position - A.Position);
    
    const f32 lenu = u.getLength();
    const f32 lenv = v.getLength();
    
    // Compute the first coordinates
    dim::vector3df pa, pb;
    
    math::CollisionLibrary::getLineLineIntersection(
        dim::line3df(A.Position, B.Position), dim::line3df(Pos, Pos - v), pa
    );
    math::CollisionLibrary::getLineLineIntersection(
        dim::line3df(A.Position, C.Position), dim::line3df(Pos, Pos - u), pb
    );
    
    const dim::point2df map(
        math::getDistance(Pos, pb), math::getDistance(Pos, pa)
    );
    
    // Interpolate between the three vertices
    *this = A + ( B - A ) * (map.X / lenu) + ( C - A ) * (map.Y / lenv);
    
    Position = Pos;
    Normal = A.Normal;
    
    return *this;
}


/*
 * STriangle structure
 */

MeshBooleanOperator::STriangle::STriangle()
    : Surface(0), Face(0)
{
    for (s32 i = 0; i < 3; ++i)
    {
        Indices[i]  = 0;
        Vertices[i] = 0;
    }
}
MeshBooleanOperator::STriangle::~STriangle()
{
}

void MeshBooleanOperator::STriangle::computeCutLines(SModel* Mod, SModel* OppositMod)
{
    // Temporary variables
    dim::triangle3df OppositTriangle;
    dim::line3df Intersection;
    
    SVertex A, B, C;
    
    scene::Mesh* OpMesh = OppositMod->Mesh;
    video::MeshBuffer* CurSurface = 0;
    
    // Loop for each surface/ triangel
    for (u32 s = 0, i; s < OpMesh->getMeshBufferCount(); ++s)
    {
        CurSurface = OpMesh->getMeshBuffer(s);
        
        for (i = 0; i < CurSurface->getTriangleCount(); ++i)
        {
            OppositTriangle = OppositMod->Matrix * CurSurface->getTriangleCoords(i);
            
            // Make a triangle-triangle intersection & check if the intersection is valid
            if (math::CollisionLibrary::checkTriangleTriangleIntersection(Triangle, OppositTriangle, Intersection))
            {
                A.set(Mod, Surface, Indices[0]);
                B.set(Mod, Surface, Indices[1]);
                C.set(Mod, Surface, Indices[2]);
                
                Face->addCutLine(A, B, C, Intersection, dim::plane3df(OppositTriangle));
            }
        }
    }
}

bool MeshBooleanOperator::STriangle::checkNormalEquality(const dim::vector3df &Normal) const
{
    return Triangle.getNormal().normalize().equal(Normal, Precision_);
}

bool MeshBooleanOperator::STriangle::checkCollision(const STriangle &OpTriangle) const
{
    return checkCollisionCorners(OpTriangle) || checkCollisionEdges(OpTriangle) || checkCollisionOverlap(OpTriangle);
}

bool MeshBooleanOperator::STriangle::checkCollisionCorners(const STriangle &OpTriangle) const
{
    // Temporary constants
    const dim::triangle3df* OpTri(&OpTriangle.Triangle);
    
    const dim::vector3df* a1 = &Triangle.PointA;
    const dim::vector3df* b1 = &Triangle.PointB;
    const dim::vector3df* c1 = &Triangle.PointC;
    
    const dim::vector3df* a2 = &OpTriangle.Triangle.PointA;
    const dim::vector3df* b2 = &OpTriangle.Triangle.PointB;
    const dim::vector3df* c2 = &OpTriangle.Triangle.PointC;
    
    // Check if the triangle is exactly onto this one
    if ( ( a1->equal(*a2, Precision_) && b1->equal(*b2, Precision_) && c1->equal(*c2, Precision_) ) ||
         ( a1->equal(*b2, Precision_) && b1->equal(*c2, Precision_) && c1->equal(*a2, Precision_) ) ||
         ( a1->equal(*c2, Precision_) && b1->equal(*a2, Precision_) && c1->equal(*b2, Precision_) ) )
    {
        return true;
    }
    
    // Check if points are inside the triangle
    if ( ( checkPointInside(Triangle, *a2) || checkPointInside(Triangle, *b2) || checkPointInside(Triangle, *c2) ) ||
         ( checkPointInside(*OpTri, *a1) || checkPointInside(*OpTri, *b1) || checkPointInside(*OpTri, *c1) ) )
    {
        return true;
    }
    
    return false;
}

bool MeshBooleanOperator::STriangle::checkCollisionEdges(const STriangle &OpTriangle) const
{
    // Temporary constants
    const dim::line3df EdgeA1(Triangle.PointA, Triangle.PointB);
    const dim::line3df EdgeB1(Triangle.PointB, Triangle.PointC);
    const dim::line3df EdgeC1(Triangle.PointC, Triangle.PointA);
    
    const dim::line3df EdgeA2(OpTriangle.Triangle.PointA, OpTriangle.Triangle.PointB);
    const dim::line3df EdgeB2(OpTriangle.Triangle.PointB, OpTriangle.Triangle.PointC);
    const dim::line3df EdgeC2(OpTriangle.Triangle.PointC, OpTriangle.Triangle.PointA);
    
    // Check each edge with each opposit edge
    if ( ( SLine::checkLineLineIntersection(EdgeA1, EdgeA2) ||
           SLine::checkLineLineIntersection(EdgeA1, EdgeB2) ||
           SLine::checkLineLineIntersection(EdgeA1, EdgeC2) ) ||
         ( SLine::checkLineLineIntersection(EdgeB1, EdgeA2) ||
           SLine::checkLineLineIntersection(EdgeB1, EdgeB2) ||
           SLine::checkLineLineIntersection(EdgeB1, EdgeC2) ) ||
         ( SLine::checkLineLineIntersection(EdgeC1, EdgeA2) ||
           SLine::checkLineLineIntersection(EdgeC1, EdgeB2) ||
           SLine::checkLineLineIntersection(EdgeC1, EdgeC2) ) )
    {
        return true;
    }
    
    return false;
}

bool MeshBooleanOperator::STriangle::checkCollisionOverlap(const STriangle &OpTriangle) const
{
    // Check if the triangle's center is inside the other (A to B and B to A) to avoid an overlaped delta connection
    if ( checkPointInside(Triangle, OpTriangle.Triangle.getCenter()) ||
         checkPointInside(OpTriangle.Triangle, Triangle.getCenter()) )
    {
        return true;
    }
    
    return false;
}

bool MeshBooleanOperator::STriangle::checkLineEdgesContact(const dim::line3df &Line) const
{
    // Temporary constants
    const bool EqualAP = Triangle.PointA.equal(Line.Start, Precision_);
    const bool EqualAQ = Triangle.PointA.equal(Line.End, Precision_);
    
    const bool EqualBP = Triangle.PointB.equal(Line.Start, Precision_);
    const bool EqualBQ = Triangle.PointB.equal(Line.End, Precision_);
    
    const bool EqualCP = Triangle.PointC.equal(Line.Start, Precision_);
    const bool EqualCQ = Triangle.PointC.equal(Line.End, Precision_);
    
    // Check if there is any contact
    if ( ( ( EqualAP && EqualBQ ) || ( EqualAQ && EqualBP ) ) ||
         ( ( EqualBP && EqualCQ ) || ( EqualBQ && EqualCP ) ) ||
         ( ( EqualCP && EqualAQ ) || ( EqualCQ && EqualAP ) ) )
    {
        return true;
    }
    
    return false;
}

void MeshBooleanOperator::STriangle::getLineEdgesIntersection(
    const dim::line3df &Line, std::list<dim::vector3df> &Points) const
{
    // Temporary varables
    dim::vector3df Intersection;
    
    // Check each edge with the line
    if (SLine::getLineLineIntersection(Line, dim::line3df(Triangle.PointA, Triangle.PointB), Intersection))
        Points.push_back(Intersection);
    if (SLine::getLineLineIntersection(Line, dim::line3df(Triangle.PointB, Triangle.PointC), Intersection))
        Points.push_back(Intersection);
    if (SLine::getLineLineIntersection(Line, dim::line3df(Triangle.PointC, Triangle.PointA), Intersection))
        Points.push_back(Intersection);
}

bool MeshBooleanOperator::STriangle::checkPointCornersContact(
    const dim::triangle3df &OpTriangle, const dim::vector3df &Point)
{
    // Check if the point is equal to any of the three triangle corners
    if ( OpTriangle.PointA.equal(Point, Precision_) ||
         OpTriangle.PointB.equal(Point, Precision_) ||
         OpTriangle.PointC.equal(Point, Precision_) )
    {
        return true;
    }
    
    return false;
}

bool MeshBooleanOperator::STriangle::checkPointInside(const dim::triangle3df &OpTriangle, const dim::vector3df &Point)
{
    // Make a simple point inside triangle test and check the correctness using round error
    if ( OpTriangle.isPointInside(Point) && !checkPointCornersContact(OpTriangle, Point) &&
         dim::line3df(OpTriangle.PointA, OpTriangle.PointB).getPointDistance(Point) > Precision_ &&
         dim::line3df(OpTriangle.PointB, OpTriangle.PointC).getPointDistance(Point) > Precision_ &&
         dim::line3df(OpTriangle.PointC, OpTriangle.PointA).getPointDistance(Point) > Precision_ )
    {
        return true;
    }
    
    return false;
}

bool MeshBooleanOperator::STriangle::checkPointInsideInv(const dim::triangle3df &OpTriangle, const dim::vector3df &Point)
{
    // Make a simple point inside triangle test and check the distance between the point and the three edges
    if ( OpTriangle.isPointInside(Point) ||
         dim::line3df(OpTriangle.PointA, OpTriangle.PointB).getPointDistance(Point) < Precision_ ||
         dim::line3df(OpTriangle.PointB, OpTriangle.PointC).getPointDistance(Point) < Precision_ ||
         dim::line3df(OpTriangle.PointC, OpTriangle.PointA).getPointDistance(Point) < Precision_ )
    {
        return true;
    }
    
    return false;
}


/*
 * SFace strucure
 */

MeshBooleanOperator::SFace::SFace(u32 DefSurface)
    : Surface(DefSurface)
{
}
MeshBooleanOperator::SFace::~SFace()
{
    for (std::list<SVertex*>::iterator it = Vertices.begin(); it != Vertices.end(); ++it)
        MemoryManager::deleteMemory(*it);
}

void MeshBooleanOperator::SFace::addCutLine(
    const SVertex &A, const SVertex &B, const SVertex &C,
    const dim::line3df &Line, const dim::plane3df OppositPlane)
{
    SLine NewCut;
    {
        NewCut.A.interpolate(Line.Start, A, B, C);
        NewCut.B.interpolate(Line.End, A, B, C);
        
        NewCut.Plane    = OppositPlane;
        NewCut.Face     = this;
    }
    CutLines.push_back(NewCut);
}

void MeshBooleanOperator::SFace::optimizeCutLines()
{
    if (CutLines.empty())
        return;
    
    // Temporary variables
    bool PlaneEqual;
    
    // Sort the cut line list
    CutLines.sort(cmpModelCutLinePlane);
    
    // Loop for each cut line
    for (std::list<SLine>::iterator it = CutLines.begin(), itLast = CutLines.begin(); it != CutLines.end();)
    {
        PlaneEqual  = (it != itLast && itLast->checkRedundance(*it));
        
        if (PlaneEqual)
        {
            // Check if the line's center is outside the last line
            if (!dim::line3df(it->A.Position, it->B.Position).getCenter().isBetweenPoints(itLast->A.Position, itLast->B.Position))
            {
                // Point equality tests
                if (itLast->B.Position.equal(it->A.Position, Precision_))
                    itLast->B = it->B;
                else if (itLast->B.Position.equal(it->B.Position, Precision_))
                    itLast->B = it->A;
                else if (itLast->A.Position.equal(it->A.Position, Precision_))
                    itLast->A = it->B;
                else if (itLast->A.Position.equal(it->B.Position, Precision_))
                    itLast->A = it->A;
                else
                {
                    // Temporary constants
                    const f32 DistAA = math::getDistanceSq(itLast->A.Position, it->A.Position);
                    const f32 DistAB = math::getDistanceSq(itLast->A.Position, it->B.Position);
                    const f32 DistBA = math::getDistanceSq(itLast->B.Position, it->A.Position);
                    const f32 DistBB = math::getDistanceSq(itLast->B.Position, it->B.Position);
                    
                    // Point distance tests
                    if (DistAA > DistAB && DistAA > DistBA && DistAA > DistBB)
                        itLast->B = it->A;
                    else if (DistAB > DistAA && DistAB > DistBA && DistAB > DistBB)
                        itLast->B = it->B;
                    else if (DistBA > DistAA && DistBA > DistAB && DistBA > DistBB)
                        itLast->A = it->A;
                    else
                        itLast->A = it->B;
                }
            }
            
            // Erase the redundant line
            it = CutLines.erase(it);
        }
        else
            itLast = it++;
    }
}

void MeshBooleanOperator::SFace::createCutVertices()
{
    // Loop for each cut line
    for (std::list<SLine>::const_iterator it = CutLines.begin(); it != CutLines.end(); ++it)
    {
        CutVertices.push_back(it->A);
        CutVertices.push_back(it->B);
    }
    
    // Optimize the cut vertices
    optimizeCutVertices();
}

void MeshBooleanOperator::SFace::optimizeCutVertices()
{
    // Sort the vertex list
    CutVertices.sort(cmpModelCutVertexPosition);
    
    // Loop for each cut vertex
    for (std::list<SVertex>::iterator it = CutVertices.begin(), itLast = CutVertices.begin(); it != CutVertices.end();)
    {
        if (it != itLast && it->Position.equal(itLast->Position, Precision_))
            it = CutVertices.erase(it);
        else
            itLast = it++;
    }
}

void MeshBooleanOperator::SFace::generateDeltaConnections()
{
    // Check if there are any cut lines
    if (CutLines.empty())
    {
        createDefaultFace();
        return;
    }
    
    // Temporary variables
    std::list<SVertex*>::iterator ita, itb, itc;
    
    // Loop for each vertex (1st pass)
    for (ita = Vertices.begin(); ita != Vertices.end(); ++ita)
    {
        // Loop for each vertex (2nd pass)
        for (itb = Vertices.begin(); itb != Vertices.end(); ++itb)
        {
            if (itb == ita || (*itb)->Position.equal((*ita)->Position, Precision_))
                continue;
            
            // Loop for each vertex (3rd pass)
            for (itc = Vertices.begin(); itc != Vertices.end(); ++itc)
            {
                if (itc == ita || itc == itb ||
                    (*itc)->Position.equal((*ita)->Position, Precision_) || (*itc)->Position.equal((*itb)->Position, Precision_))
                {
                    continue;
                }
                
                CurTriangle.Vertices[0] = *ita;
                CurTriangle.Vertices[1] = *itb;
                CurTriangle.Vertices[2] = *itc;
                
                CurTriangle.Triangle.PointA = (*ita)->Position;
                CurTriangle.Triangle.PointB = (*itb)->Position;
                CurTriangle.Triangle.PointC = (*itc)->Position;
                
                // Check if the triangle represents no area
                if (CurTriangle.Triangle.getArea() <= Precision_)
                    continue;
                
                // Make all tests for checking the delta connection
                if (checkDeltaConnection())
                    FinalTriangles.push_back(CurTriangle);
            } // /3rd pass
        } // /2nd pass
    } // /1st pass
}

bool MeshBooleanOperator::SFace::checkDeltaConnection() const
{
    // Compare face normal with triangle normal
    if (!CurTriangle.checkNormalEquality(Plane.Normal))
        return false;
    
    // Check triangle-triangle collision
    if (checkTriangleCollision())
        return false;
    
    // Check if the triangle's center is on the right side of cut lines' plane
    if (checkCutPlanesSide())
        return false;
    
    // Check if the triangle is inside the previous face (for concave faces)
    if (!checkInsideConcaveFace())
        return false;
    
    return true;
}

bool MeshBooleanOperator::SFace::checkTriangleCollision() const
{
    // Loop for each triangle
    for (std::vector<STriangle>::const_iterator it = FinalTriangles.begin(); it != FinalTriangles.end(); ++it)
    {
        if (it->checkCollision(CurTriangle))
            return true;
    }
    
    // Loop for each cut line
    for (std::list<SLine>::const_iterator it = CutLines.begin(); it != CutLines.end(); ++it)
    {
        if (it->checkCollision(CurTriangle))
            return true;
    }
    
    return false;
}

bool MeshBooleanOperator::SFace::checkCutPlanesSide() const
{
    // Temporary variables
    const dim::vector3df TriangleCenter(CurTriangle.Triangle.getCenter());
    const SLine* Line = getClosestCutLine(TriangleCenter);
    
    if (Line)
    {
        // Check the plane's side
        const f32 Distance = Line->Plane.getPointDistance(TriangleCenter);
        
        if ( ( CutFrontSide_ && Distance >= Precision_ ) ||
             ( !CutFrontSide_ && Distance <= -Precision_ ) )
        {
            return true;
        }
    }
    
    return false;
}

const MeshBooleanOperator::SLine* MeshBooleanOperator::SFace::getClosestCutLine(const dim::vector3df &Point) const
{
    // Temporary variables
    const SLine* ClosestLine = 0;
    
    f32 TmpDist;
    f32 Distance = 999999.f;
    dim::line3df LineAB;
    
    // Find the nearest line to the triangle's edges
    for (std::list<SLine>::const_iterator it = CutLines.begin(); it != CutLines.end(); ++it)
    {
        if (CurTriangle.checkLineEdgesContact(dim::line3df(it->A.Position, it->B.Position)))
            return &*it;
    }
    
    // Find the nearest line to the triangle's center (1st pass)
    for (std::list<SLine>::const_iterator it = CutLines.begin(); it != CutLines.end(); ++it)
    {
        LineAB.Start    = it->A.Position;
        LineAB.End      = it->B.Position;
        
        TmpDist = LineAB.getPointDistance(Point);
        
        if (TmpDist < Distance)
        {
            Distance    = TmpDist;
            ClosestLine = &*it;
        }
    }
    
    // Find other lines which have the same distance
    // If the triangle's center is on the right side of any of these lines' plane cancel the line search
    for (std::list<SLine>::const_iterator it = CutLines.begin(); it != CutLines.end(); ++it)
    {
        if (&*it == ClosestLine)
            continue;
        
        LineAB.Start    = it->A.Position;
        LineAB.End      = it->B.Position;
        
        TmpDist = LineAB.getPointDistance(Point);
        
        if (TmpDist <= Distance + Precision_)
        {
            TmpDist = it->Plane.getPointDistance(Point);
            
            if ( ( !CutFrontSide_ && TmpDist >= Precision_ ) ||
                 ( CutFrontSide_ && TmpDist <= -Precision_ ) )
            {
                return 0;
            }
        }
    }
    
    return ClosestLine;
}

bool MeshBooleanOperator::SFace::checkInsideConcaveFace() const
{
    // Temporary variables
    std::list<dim::vector3df>::const_iterator ita, itb, itc;
    dim::triangle3df TmpTriangle;
    
    // Get the list of all cut points between the edges
    // of the current triangle and the original triangles
    std::list<dim::vector3df> Points;
    fillCutPointList(Points);
    
    // Loop for each point (1st pass)
    for (ita = Points.begin(); ita != Points.end(); ++ita)
    {
        // Loop for each point (2nd pass)
        for (itb = Points.begin(); itb != Points.end(); ++itb)
        {
            if (itb == ita || ita->equal(*itb, Precision_))
                continue;
            
            // Loop for each point (3rd pass)
            for (itc = Points.begin(); itc != Points.end(); ++itc)
            {
                if (itc == ita || itc == itb || itc->equal(*ita, Precision_) || itc->equal(*itb, Precision_))
                    continue;
                
                TmpTriangle.PointA = *ita;
                TmpTriangle.PointB = *itb;
                TmpTriangle.PointC = *itc;
                
                // Check if the triangle represents no area
                if (TmpTriangle.getArea() <= Precision_)
                    continue;
                
                // Check if the temp-triangle's center is inside the face
                if (!isPointInside(TmpTriangle.getCenter()))
                    return false;
            } // /3rd pass
        } // /2nd pass
    } // /1st pass
    
    return true;
}

void MeshBooleanOperator::SFace::fillCutPointList(std::list<dim::vector3df> &Points) const
{
    // Temporary variables
    dim::vector3df Intersection;
    
    const dim::line3df EdgeAB(CurTriangle.Triangle.PointA, CurTriangle.Triangle.PointB);
    const dim::line3df EdgeBC(CurTriangle.Triangle.PointB, CurTriangle.Triangle.PointC);
    const dim::line3df EdgeCA(CurTriangle.Triangle.PointC, CurTriangle.Triangle.PointA);
    
    // Add each triangle's vertices' point
    Points.push_back(CurTriangle.Triangle.PointA);
    Points.push_back(CurTriangle.Triangle.PointB);
    Points.push_back(CurTriangle.Triangle.PointC);
    
    // Loop for each original triangle
    for (std::vector<STriangle>::const_iterator it = Triangles.begin(); it != Triangles.end(); ++it)
    {
        // Add each triangle's vertex's point which is inside the current triangle
        for (s32 i = 0; i < 3; ++i)
        {
            if (CurTriangle.Triangle.isPointInside(it->Triangle[i]))
                Points.push_back(it->Triangle[i]);
        }
        
        // Make line-line intersection tests with each triangle's edge of both
        it->getLineEdgesIntersection(EdgeAB, Points);
        it->getLineEdgesIntersection(EdgeBC, Points);
        it->getLineEdgesIntersection(EdgeCA, Points);
    }
    
    Points.unique();
}

bool MeshBooleanOperator::SFace::isPointInside(const dim::vector3df &Point) const
{
    // Loop for each original triangle
    for (std::vector<STriangle>::const_iterator it = Triangles.begin(); it != Triangles.end(); ++it)
    {
        if (STriangle::checkPointInsideInv(it->Triangle, Point))
            return true;
    }
    
    return false;
}

void MeshBooleanOperator::SFace::createDefaultFace()
{
    // Loop for each original triangle
    for (std::vector<STriangle>::const_iterator it = Triangles.begin(); it != Triangles.end(); ++it)
    {
        CurTriangle.Vertices[0] = getVertex(it->Surface, it->Indices[0]);
        CurTriangle.Vertices[1] = getVertex(it->Surface, it->Indices[1]);
        CurTriangle.Vertices[2] = getVertex(it->Surface, it->Indices[2]);
        
        if (CurTriangle.Vertices[0] && CurTriangle.Vertices[1] && CurTriangle.Vertices[2])
            FinalTriangles.push_back(CurTriangle);
    }
}

MeshBooleanOperator::SVertex* MeshBooleanOperator::SFace::getVertex(u32 VertexSurface, u32 VertexIndex)
{
    for (std::list<SVertex*>::iterator it = Vertices.begin(); it != Vertices.end(); ++it)
    {
        if ((*it)->Surface == VertexSurface && (*it)->Index == VertexIndex)
            return *it;
    }
    
    return 0;
}


/*
 * SLine strucure
 */

MeshBooleanOperator::SLine::SLine()
    : Face(0)
{
}
MeshBooleanOperator::SLine::~SLine()
{
}

bool MeshBooleanOperator::SLine::checkRedundance(const SLine &other) const
{
    dim::vector3df dir1(B.Position - A.Position);
    dim::vector3df dir2(other.B.Position - other.A.Position);
    
    dir1.normalize();
    dir2.normalize();
    
    return (
        Plane.equal(other.Plane, Precision_) && ( dir1.equal(dir2, Precision_) || dir1.equal(-dir2, Precision_) )
    );
}

bool MeshBooleanOperator::SLine::checkCollision(const STriangle &Triangle) const
{
    // Temporary constants
    const dim::line3df Line(A.Position, B.Position);
    const dim::line3df EdgeA(Triangle.Triangle.PointA, Triangle.Triangle.PointB);
    const dim::line3df EdgeB(Triangle.Triangle.PointB, Triangle.Triangle.PointC);
    const dim::line3df EdgeC(Triangle.Triangle.PointC, Triangle.Triangle.PointA);
    
    // Make line-line intersection tests with the triangle's edges
    if ( checkLineLineIntersection(Line, EdgeA) ||
         checkLineLineIntersection(Line, EdgeB) ||
         checkLineLineIntersection(Line, EdgeC) )
    {
        return true;
    }
    
    // Check if the line's center is inside the triangle
    if (STriangle::checkPointInside(Triangle.Triangle, Line.getCenter()))
        return true;
    
    return false;
}

bool MeshBooleanOperator::SLine::checkLineLineIntersection(const dim::line3df &A, const dim::line3df &B)
{
    // Check corner contact
    if (checkLineLineContact(A, B, CONTACT_CORNERS))
        return false;
    
    // Make line line intersection test
    dim::vector3df p1, p2;
    
    const f32 Distance = math::CollisionLibrary::getLineLineDistanceSq(A, B, p1, p2);
    
    if ( Distance <= Precision_ &&
         math::getDistance(p1, A.Start) > Precision_ && math::getDistance(p2, A.Start) > Precision_ &&
         math::getDistance(p1, A.End) > Precision_ && math::getDistance(p2, A.End) > Precision_ &&
         math::getDistance(p1, B.Start) > Precision_ && math::getDistance(p2, B.Start) > Precision_ &&
         math::getDistance(p1, B.End) > Precision_ && math::getDistance(p2, B.End) > Precision_ )
    {
        return true;
    }
    
    // No intersection or contact
    return false;
}

bool MeshBooleanOperator::SLine::getLineLineIntersection(const dim::line3df &A, const dim::line3df &B, dim::vector3df &Point)
{
    // Make line line intersection test
    dim::vector3df p1, p2;
    
    const f32 Distance = math::CollisionLibrary::getLineLineDistanceSq(A, B, p1, p2);
    
    if ( Distance <= Precision_ &&
         math::getDistance(p1, A.Start) >= 0.0f && math::getDistance(p2, A.Start) >= 0.0f &&
         math::getDistance(p1, A.End) >= 0.0f && math::getDistance(p2, A.End) >= 0.0f &&
         math::getDistance(p1, B.Start) >= 0.0f && math::getDistance(p2, B.Start) >= 0.0f &&
         math::getDistance(p1, B.End) >= 0.0f && math::getDistance(p2, B.End) >= 0.0f )
    {
        Point = p1;
        return true;
    }
    
    // No intersection or contact
    return false;
}

bool MeshBooleanOperator::SLine::checkLineLineContact(const dim::line3df &A, const dim::line3df &B, s32 Flags)
{
    // Temporary constants
    const bool EqualA1A2 = A.Start.equal(B.Start, Precision_);
    const bool EqualB1B2 = A.End.equal(B.End, Precision_);
    const bool EqualA1B2 = A.Start.equal(B.End, Precision_);
    const bool EqualB1A2 = A.End.equal(B.Start, Precision_);
    
    // Compare corners
    if (Flags & CONTACT_CORNERS)
    {
        if (EqualA1A2 || EqualB1B2 || EqualA1B2 || EqualB1A2)
            return true;
    }
    
    // Compare edges
    if (Flags & CONTACT_EDGES)
    {
        if ( ( EqualA1A2 && EqualB1B2 ) || ( EqualA1B2 && EqualB1A2 ))
            return true;
    }
    
    // Make intersection test
    if (Flags & CONTACT_CUT)
    {
        if (checkLineLineIntersection(A, B))
            return true;
    }
    
    return false;
}


/*
 * SModel structure
 */

MeshBooleanOperator::SModel::SModel(scene::Mesh* DefMesh) :
    Mesh(DefMesh)
{
    if (Mesh)
    {
        Matrix = NormalMatrix = Mesh->getTransformMatrix(true);
        NormalMatrix.setPosition(0.0f);
    }
}
MeshBooleanOperator::SModel::~SModel()
{
    for (std::vector<SFace*>::iterator it = Faces.begin(); it != Faces.end(); ++it)
        MemoryManager::deleteMemory(*it);
}

void MeshBooleanOperator::SModel::createVertices(SModel* OppositMod)
{
    // Temporary variables
    std::vector<SFace*>::iterator itFace;
    std::vector<STriangle>::iterator it;
    video::MeshBuffer* CurSurface = 0;
    s32 i;
    
    // Loop for each face/ trianlge
    for (itFace = Faces.begin(); itFace != Faces.end(); ++itFace)
    {
        for (it = (*itFace)->Triangles.begin(); it != (*itFace)->Triangles.end(); ++it)
        {
            for (i = 0; i < 3; ++i)
            {
                CurSurface = Mesh->getMeshBuffer(it->Surface);
                
                if (isPointInside(OppositMod, Matrix * CurSurface->getVertexCoord(it->Indices[i])))
                    (*itFace)->OrigVertices.push_back(SVertex(this, it->Surface, it->Indices[i]));
            }
            
            (*itFace)->OrigVertices.unique();
        }
    }
}

void MeshBooleanOperator::SModel::createFaces()
{
    // Temporary variabels
    STriangle TmpTriangle;
    std::list<STriangle> TmpTriangleList;
    
    SFace* CurFace = 0;
    video::MeshBuffer* CurSurface = 0;
    dim::plane3df LastPlane;
    bool PlaneEqual;
    
    // Loop for each surface/ triangle
    for (u32 s = 0, i; s < Mesh->getMeshBufferCount(); ++s)
    {
        CurSurface = Mesh->getMeshBuffer(s);
        TmpTriangle.Surface = s;
        
        for (i = 0; i < CurSurface->getTriangleCount(); ++i)
        {
            CurSurface->getTriangleIndices(i, TmpTriangle.Indices);
            
            TmpTriangle.Triangle    = Matrix * CurSurface->getTriangleCoords(i);
            TmpTriangle.Plane       = dim::plane3df(TmpTriangle.Triangle);
            
            TmpTriangleList.push_back(TmpTriangle);
        }
    }
    
    // Sort the triangle list
    TmpTriangleList.sort(cmpModelTrianglePlane);
    
    // Create the faces & and its triangles
    if (TmpTriangleList.size())
    {
        CurFace     = new SFace(TmpTriangleList.begin()->Surface);
        LastPlane   = TmpTriangleList.begin()->Plane;
        Faces.push_back(CurFace);
    }
    
    for (std::list<STriangle>::iterator it = TmpTriangleList.begin(); it != TmpTriangleList.end(); ++it)
    {
        PlaneEqual = (it == TmpTriangleList.begin() || LastPlane.equal(it->Plane, Precision_));
        
        if (!PlaneEqual)
        {
            CurFace     = new SFace(it->Surface);
            LastPlane   = it->Plane;
            Faces.push_back(CurFace);
        }
        
        it->Face = CurFace;
        CurFace->Plane = LastPlane;
        CurFace->Triangles.push_back(*it);
    }
}

void MeshBooleanOperator::SModel::computeCutLines(SModel* OppositMod)
{
    // Temporary variables
    std::vector<SFace*>::iterator itFace;
    std::vector<STriangle>::iterator it;
    
    // Loop for each face/ triangle
    for (itFace = Faces.begin(); itFace != Faces.end(); ++itFace)
    {
        for (it = (*itFace)->Triangles.begin(); it != (*itFace)->Triangles.end(); ++it)
            it->computeCutLines(this, OppositMod);
        
        (*itFace)->optimizeCutLines();
        (*itFace)->createCutVertices();
    }
    
}

bool MeshBooleanOperator::SModel::isPointInside(SModel* OppositMod, dim::vector3df Point)
{
    if (!Mesh || !Mesh->getTriangleCount())
        return false;
    
    // Temporary variables
    u32 Indices[3];
    dim::triangle3df UsedTriangle, LastTriangle;
    dim::plane3df TempPlane;
    video::MeshBuffer* CurSurface = 0;
    
    f32 Distance = 999999.f, TempDist;
    
    Point = OppositMod->Matrix.getInverse() * Point;
    
    std::list<SModel::STrianglePointDistance> DistanceList;
    SModel::STrianglePointDistance DistanceData;
    
    // Loop for each surface
    for (u32 s = 0, i; s < OppositMod->Mesh->getMeshBufferCount(); ++s)
    {
        CurSurface = OppositMod->Mesh->getMeshBuffer(s);
        
        // Loop for each triangle
        for (i = 0; i < CurSurface->getTriangleCount(); ++i)
        {
            CurSurface->getTriangleIndices(i, Indices);
            
            // Get the current triangle
            DistanceData.Triangle = dim::triangle3df(
                CurSurface->getVertexCoord(Indices[0]),
                CurSurface->getVertexCoord(Indices[1]),
                CurSurface->getVertexCoord(Indices[2])
            );
            
            // Compute the distance between the point and the triangle
            DistanceData.Distance = TempDist = math::getDistanceSq(
                Point, math::CollisionLibrary::getClosestPoint(DistanceData.Triangle, Point)
            );
            DistanceList.push_back(DistanceData);
            
            // Save the shortest distance
            if (TempDist < Distance)
                Distance = TempDist;
        }
    }
    
    // Sort the distance list
    DistanceList.sort(cmpTrianglePointDistance);
    
    // Loop each distance data entry and search for a valid point-plane distance
    for (std::list<SModel::STrianglePointDistance>::iterator it = DistanceList.begin(); it != DistanceList.end(); ++it)
    {
        // If no valid point-plane distance could found break and check for the last triangle
        if (it->Distance > Distance)
            break;
        
        UsedTriangle = it->Triangle;
        
        // Check if the point is on the right plane's side
        TempPlane   = dim::plane3df(UsedTriangle);
        TempDist    = TempPlane.getPointDistance(Point);
        
        if ( ( CutFrontSide_ && TempDist <= Precision_ ) ||
             ( !CutFrontSide_ && TempDist >= -Precision_ ) )
        {
            return true;
        }
    }
    
    // Check if the point is on the right plane's side
    TempPlane   = dim::plane3df(UsedTriangle);
    TempDist    = TempPlane.getPointDistance(Point);
    
    if ( ( CutFrontSide_ && TempDist <= Precision_ ) ||
         ( !CutFrontSide_ && TempDist >= -Precision_ ) )
    {
        return true;
    }
    
    return false;
}

void MeshBooleanOperator::SModel::generateDeltaConnections()
{
    std::list<SVertex>::iterator it;
    
    for (std::vector<SFace*>::iterator itFace = Faces.begin(); itFace != Faces.end(); ++itFace)
    {
        for (it = (*itFace)->CutVertices.begin(); it != (*itFace)->CutVertices.end(); ++it)
            (*itFace)->Vertices.push_back(new SVertex(*it));
        for (it = (*itFace)->OrigVertices.begin(); it != (*itFace)->OrigVertices.end(); ++it)
            (*itFace)->Vertices.push_back(new SVertex(*it));
        
        (*itFace)->generateDeltaConnections();
    }
}

void MeshBooleanOperator::SModel::addVertex(video::MeshBuffer* Surface, SVertex* Vertex)
{
    Vertex->Index = Surface->addVertex(
        Vertex->Position - Matrix.getPosition(),
        (!CutFrontSide_ ? -Vertex->Normal : Vertex->Normal),
        Vertex->TexCoord[0],
        Vertex->Color,
        Vertex->Fog
    );
    
    for (s32 i = 0; i < MAX_COUNT_OF_TEXTURES; ++i)
        Surface->setVertexTexCoord(Vertex->Index, Vertex->TexCoord[i], i);
}

void MeshBooleanOperator::SModel::build()
{
    // Clear each surface
    for (u32 s = 0; s < Mesh->getMeshBufferCount(); ++s)
        Mesh->getMeshBuffer(s)->clearIndices();
    
    video::MeshBuffer* Surface = 0;
    
    // Loop for each face
    for (std::vector<SFace*>::iterator itFace = Faces.begin(); itFace != Faces.end(); ++itFace)
    {
        Surface = Mesh->getMeshBuffer((*itFace)->Surface);
        
        // Create each final vertex
        for (std::list<SVertex*>::iterator it = (*itFace)->Vertices.begin(); it != (*itFace)->Vertices.end(); ++it)
            addVertex(Surface, *it);
        
        // Create each triangle
        for (std::vector<STriangle>::iterator it = (*itFace)->FinalTriangles.begin(); it != (*itFace)->FinalTriangles.end(); ++it)
        {
            if (CutFrontSide_)
            {
                Surface->addTriangle(
                    it->Vertices[0]->Index, it->Vertices[1]->Index, it->Vertices[2]->Index
                );
            }
            else
            {
                Surface->addTriangle(
                    it->Vertices[2]->Index, it->Vertices[1]->Index, it->Vertices[0]->Index
                );
            }
        }
    }
    
    // Final modeling
    Mesh->updateMeshBuffer();
    
    Mesh->setRotation(0.0f);
    Mesh->setScale(1.0f);
    
    clear();
}

void MeshBooleanOperator::SModel::clear()
{
    for (std::vector<SFace*>::iterator it = Faces.begin(); it != Faces.end(); ++it)
        MemoryManager::deleteMemory(*it);
}


} // /namespace tool

} // /namespace sp


#endif



// ================================================================================
