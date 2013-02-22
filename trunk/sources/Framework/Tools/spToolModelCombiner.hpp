/*
 * Model combiner header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_MESH_BOOLEAN_OPERATOR_H__
#define __SP_MESH_BOOLEAN_OPERATOR_H__


#include "Base/spStandard.hpp"

#ifdef SP_COMPILE_WITH_MESHBOOLEANOPERATOR


#include "Base/spInputOutputString.hpp"
#include "Base/spDimension.hpp"
#include "SceneGraph/spSceneGraph.hpp"

#include <list>
#include <vector>


namespace sp
{
namespace tool
{


enum EModelCombinations
{
    COMBINATION_UNION,
    COMBINATION_DIFFERENCE,
    COMBINATION_DIFFERENCEINV,
    COMBINATION_INTERSECTION,
};

/**
MeshBooleanOperator is used to combine two models or to cut a model using a mask model.
The following methods for combining models can be used:

Model combination methods notes:
\code
    +------+
    |      |
+---+--+ B |
|   |  |   |
| A +--+---+
|      |
+------+

COMBINATION_UNION:      COMBINATION_DIFFERENCE:     COMBINATION_DIFFERENCEINV:  COMBINATION_INTERSECTION:

    +------+                                            +------+
    |      |                                            |      |
+---+    B |            +---+    B                      +--+ B |                    +--+ B
|          |            |   |                              |   |                    |  |
| A    +---+            | A +--+                         A +---+                  A +--+
|      |                |      |
+------+                +------+
\endcode
*/
class SP_EXPORT MeshBooleanOperator
{
    
    public:
        
        MeshBooleanOperator();
        ~MeshBooleanOperator();
        
        /**
        Combines two models. The result is the two modified models.
        \param MeshA: First mesh which is to be combined with the second one.
        \param MeshB: Second mesh which is to be combined with the first one.
        \param Method: Method which specifies how the models are to be combined.
        */
        void combineModels(
            scene::Mesh* MeshA, scene::Mesh* MeshB, const EModelCombinations Method = COMBINATION_UNION
        );
        
        /* Static functions */
        
        static void setPrecision(const f32 Precision);
        static f32 getPrecision();
        
    private:
        
        /* === Declerations === */
        
        struct STriangle;
        struct SFace;
        struct SLine;
        struct SModel;
        struct SVertex;
        
        /* === Enumerations === */
        
        enum ELineContactFlags
        {
            CONTACT_CORNERS = 0x0001,
            CONTACT_EDGES   = 0x0002,
            CONTACT_CUT     = 0x0004, // Intersection
            CONTACT_FULL    = (CONTACT_CORNERS | CONTACT_EDGES | CONTACT_CUT)
        };
        
        /* === Structures === */
        
        struct SVertex
        {
            SVertex();
            SVertex(SModel* Mod, u32 VertexSurface, u32 VertexIndex);
            SVertex(const SVertex &other);
            ~SVertex();
            
            /* Operations */
            bool operator == (const SVertex &other) const;
            bool operator != (const SVertex &other) const;
            
            SVertex& operator = (const SVertex &other);
            
            SVertex operator + (const SVertex &other) const;
            SVertex operator - (const SVertex &other) const;
            SVertex operator * (const SVertex &other) const;
            SVertex operator / (const SVertex &other) const;
            
            SVertex operator + (const f32 Size) const;
            SVertex operator - (const f32 Size) const;
            SVertex operator * (const f32 Size) const;
            SVertex operator / (const f32 Size) const;
            
            /* Functions */
            SVertex& set(SModel* Mod, u32 VertexSurface, u32 VertexIndex);
            
            SVertex& interpolate(
                const dim::vector3df &Pos, const SVertex &A, const SVertex &B, const SVertex &C
            );
            
            /* Members */
            u32 Surface, Index;
            
            dim::vector3df Position;
            dim::vector3df Normal;
            dim::vector3df TexCoord[MAX_COUNT_OF_TEXTURES];
            video::color Color;
            f32 Fog;
        };
        
        struct STriangle
        {
            STriangle();
            ~STriangle();
            
            /* Functions */
            void computeCutLines(SModel* Mod, SModel* OppositMod);
            
            bool checkNormalEquality(const dim::vector3df &Normal) const;
            
            bool checkCollision(const STriangle &OpTriangle) const;
            bool checkCollisionCorners(const STriangle &OpTriangle) const;
            bool checkCollisionEdges(const STriangle &OpTriangle) const;
            bool checkCollisionOverlap(const STriangle &OpTriangle) const;
            
            bool checkLineEdgesContact(const dim::line3df &Line) const;
            
            void getLineEdgesIntersection(const dim::line3df &Line, std::list<dim::vector3df> &Points) const;
            
            static bool checkPointCornersContact(const dim::triangle3df &OpTriangle, const dim::vector3df &Point);
            static bool checkPointInside(const dim::triangle3df &OpTriangle, const dim::vector3df &Point);
            static bool checkPointInsideInv(const dim::triangle3df &OpTriangle, const dim::vector3df &Point);
            
            /* Members */
            u32 Surface;
            u32 Indices[3];                 // Vertex indices
            SVertex* Vertices[3];           // Vertex points
            dim::triangle3df Triangle;      // Triangle coordinates
            SFace* Face;                    // Associated face
            dim::plane3df Plane;
        };
        
        struct SFace
        {
            SFace(u32 DefSurface = 0);
            ~SFace();
            
            /* Functions */
            void addCutLine(
                const SVertex &A, const SVertex &B, const SVertex &C,
                const dim::line3df &Line, const dim::plane3df OppositPlane
            );
            void optimizeCutLines();
            
            void createCutVertices();
            void optimizeCutVertices();
            
            void generateDeltaConnections();
            bool checkDeltaConnection() const;
            bool checkTriangleCollision() const;
            
            bool checkCutPlanesSide() const;
            const SLine* getClosestCutLine(const dim::vector3df &Point) const;
            
            bool checkInsideConcaveFace() const;
            void fillCutPointList(std::list<dim::vector3df> &Points) const;
            bool isPointInside(const dim::vector3df &Point) const;
            
            void createDefaultFace();
            SVertex* getVertex(u32 VertexSurface, u32 VertexIndex);
            
            /* Members */
            u32 Surface;
            dim::plane3df Plane;
            std::vector<STriangle> Triangles, FinalTriangles;
            std::list<SLine> CutLines;
            std::list<SVertex> CutVertices, OrigVertices;       
            std::list<SVertex*> Vertices;                   // Vertices = CutVertices + OrigVertices
            STriangle CurTriangle;                          // Current triangle to be tested
        };
        
        struct SLine
        {
            SLine();
            ~SLine();
            
            /* Functions */
            bool checkRedundance(const SLine &other) const;
            
            bool checkCollision(const STriangle &Triangle) const;
            
            static bool checkLineLineIntersection(const dim::line3df &A, const dim::line3df &B);
            static bool getLineLineIntersection(const dim::line3df &A, const dim::line3df &B, dim::vector3df &Point);
            
            static bool checkLineLineContact(const dim::line3df &A, const dim::line3df &B, s32 Flags = CONTACT_FULL);
            
            /* Members */
            SVertex A, B;
            dim::plane3df Plane;            // Plane of the opposit triangle
            SFace* Face;                    // Associated face
        };
        
        struct SModel
        {
            SModel(scene::Mesh* DefMesh);
            ~SModel();
            
            /* Functions */
            bool isPointInside(SModel* OppositMod, dim::vector3df Point);
            
            void createVertices(SModel* OppositMod);
            void createFaces();
            void computeCutLines(SModel* OppositMod);
            void generateDeltaConnections();
            
            void addVertex(video::MeshBuffer* Surface, SVertex* Vertex);
            void build();
            void clear();
            
            /* Structures */
            struct STrianglePointDistance
            {
                dim::triangle3df Triangle;
                f32 Distance;
            };
            
            /* Members */
            scene::Mesh* Mesh;
            dim::matrix4f Matrix, NormalMatrix;
            
            std::vector<SFace*> Faces;       // Face list (not final)
        };
        
        /* === Friends === */
        
        friend bool cmpModelTrianglePlane(const MeshBooleanOperator::STriangle &obj1, const MeshBooleanOperator::STriangle &obj2);
        friend bool cmpModelCutLinePlane(const MeshBooleanOperator::SLine &obj1, const MeshBooleanOperator::SLine &obj2);
        friend bool cmpModelCutVertexPosition(const MeshBooleanOperator::SVertex &obj1, const MeshBooleanOperator::SVertex &obj2);
        
        friend bool cmpTrianglePointDistance(
            const MeshBooleanOperator::SModel::STrianglePointDistance &obj1,
            const MeshBooleanOperator::SModel::STrianglePointDistance &obj2
        );
        
        /* === Functions === */
        
        void cutModel(SModel* ModA, SModel* ModB, bool CutFrontSide);
        
        /* === Members === */
        
        EModelCombinations Method_;
        
        SModel * Model_, * OppositModel_;
        
        static bool CutFrontSide_;
        static f32 Precision_;
        
};


} // /namespace tool

} // /namespace sp


#endif

#endif



// ================================================================================
