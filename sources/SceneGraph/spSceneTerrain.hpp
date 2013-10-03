/*
 * Terrain scene node header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_SCENE_TERRAIN_H__
#define __SP_SCENE_TERRAIN_H__


#include "Base/spStandard.hpp"
#include "Base/spTreeNodeQuad.hpp"
#include "Base/spMeshBuffer.hpp"
#include "SceneGraph/spSceneMesh.hpp"
#include "RenderSystem/spTextureBase.hpp"

#include <vector>
#include <list>


namespace sp
{
namespace video
{
    class Texture;
}
namespace scene
{


/**
Terrain class assumes the work of mesh LOD (level-of-detail) for the model-patches. Used are quad-trees.
*/
class SP_EXPORT Terrain : public MaterialNode
{
    
    public:
        
        #if 0
        
        Terrain(const video::SHeightMapTexture &HeightMap, const dim::size2di &Resolution, s32 GeoMIPLevels);
        virtual ~Terrain();
        
        /* Rendering */
        
        virtual void render();
        
        void changeHeightMap(
            const video::SHeightMapTexture &HeightMap, const dim::size2di &Resolution, s32 GeoMIPLevels
        );
        
        /* Inline functions */
        
        inline const dim::size2di& getResolution() const
        {
            return Resolution_;
        }
        
        inline video::MeshBuffer* getTextureReferenceMesh()
        {
            return MeshTexReference_;
        }
        
        #else
        
        Terrain();
        virtual ~Terrain();
        
        /* === Functions === */
        
        virtual void render();
        
        bool generate(u32 GridSize, u8 GeoMIPLevels);
        
        /* === Inline functions === */
        
        inline void setHeightMap(video::Texture* HeightMap)
        {
            HeightMap_ = HeightMap;
        }
        inline video::Texture* getHeightMap() const
        {
            return HeightMap_;
        }
        
        inline u32 getGridSize() const
        {
            return GridSize_;
        }
        inline u8 getGeoMIPLevels() const
        {
            return GeoMIPLevels_;
        }
        
        #endif
        
    protected:
        
        #if 0
        
        friend void clbTerrainTreeNodeDestructor(TreeNode* Node);
        
        /* === Structures === */
        
        struct SP_EXPORT STreeNodeData
        {
            STreeNodeData();
            ~STreeNodeData();
            
            /* Functions */
            void recreateBottom (bool Lower, const dim::size2di &Resolution);
            void recreateTop    (bool Lower, const dim::size2di &Resolution);
            void recreateLeft   (bool Lower, const dim::size2di &Resolution);
            void recreateRight  (bool Lower, const dim::size2di &Resolution);
            
            /* Members */
            s32 MIPLevel;
            dim::point2df Center;
            dim::size2di Resolution;
            dim::aabbox3df BoundBox;
            std::vector<f32> EdgeVerticesHeight;
            video::MeshBuffer Mesh;
            bool Selected;
            dim::rect2d<bool> EdgesLower;
        };
        
        /* === Functions === */
        
        void setupTerrain();
        
        void createQuadTree(QuadTreeNode* Node, s32 &MIPLevel, dim::point2di CurPos);
        void createTreeNodeMesh(QuadTreeNode* Node, STreeNodeData* NodeData, const dim::point2di &CurPos);
        void selectTreeNodeMesh(QuadTreeNode* Node);
        void renderTreeNodeMesh(QuadTreeNode* Node);
        bool deformTreeNodeMesh(QuadTreeNode* Node, const dim::point2df &Pos);
        
        s32 getNodeLevel(const QuadTreeNode* Node, const dim::point2df &Pos);
        
        /* === Inline functions === */
        
        inline s32 getVertexIndex(s32 x, s32 y) const
        {
            return y * (MeshResolution_.Width + 1) + x;
        }
        
        inline bool checkFurstumCulling(
            const scene::ViewFrustum &Frustum, const dim::matrix4f &Transformation, const dim::aabbox3df &BoundBox) const
        {
            return Frustum.isBoundBoxInside(BoundBox, Transformation);
        }
        
        /* === Members === */
        
        video::SHeightMapTexture HeightMap_;
        
        dim::size2di Resolution_, MeshResolution_;
        s32 GeoMIPLevels_;
        
        QuadTreeNode* RootTreeNode_;
        video::MeshBuffer* MeshTexReference_;
        
        s32 RenderModeListSize_;
        std::vector<QuadTreeNode*> RenderNodeList_;
        
        dim::matrix4f GlobalTerrainTransformation_;
        dim::vector3df GlobalCamPosition_;
        
        #else
        
        typedef dim::vector2d<u32> VertexPos;
        
        /* === Enumerations === */
        
        enum EChunkTypes
        {
            CHUNK_BASE = 0,
            
            CHUNK_LEFT,
            CHUNK_RIGHT,
            CHUNK_TOP,
            CHUNK_BOTTOM,
            
            CHUNK_LEFT_TOP,
            CHUNK_LEFT_BOTTOM,
            CHUNK_RIGHT_TOP,
            CHUNK_RIGHT_BOTTOM,
            
            CHUNK_NUM
        };
        
        /* === Structures === */
        
        struct SGridChunk
        {
            SGridChunk() :
                StartOffset(0),
                NumVertices(0)
            {
            }
            ~SGridChunk()
            {
            }
            
            /* Members */
            u32 StartOffset;
            u32 NumVertices;
        };
        
        /* === Functions === */
        
        void createChunkBase();
        void createChunkEdge(const EChunkTypes Type);
        void createChunkCorner(const EChunkTypes Type);
        
        void createVertex   (u32 x, u32 y);
        void createQuad     (u32 x, u32 y);
        void createEdge     (u32 x, u32 y, const EChunkTypes Type);
        void createCorner   (u32 x, u32 y, const EChunkTypes Type);
        
        bool isPosCorner            (const VertexPos &Pos, const EChunkTypes CornerType ) const;
        bool isPosEdge              (const VertexPos &Pos, const EChunkTypes EdgeType   ) const;
        EChunkTypes getChunkEdgeType(const VertexPos &Pos, const EChunkTypes CornerType ) const;
        
        void drawChunk(const dim::vector3df &GlobalCamPos, dim::matrix4f Transform, u8 GeoMIPLevel);
        
        /* === Members === */
        
        video::Texture* HeightMap_;
        
        video::MeshBuffer MeshBuffer_;
        
        u32 GridSize_;
        u8 GeoMIPLevels_;
        f32 InvGridSize_;
        
        SGridChunk GridChunks_[CHUNK_NUM];
        
        #endif
        
};


} // /namespace scene

} // /namespace sp


#endif



// ================================================================================
