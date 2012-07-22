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
#include "SceneGraph/spSceneMesh.hpp"
#include "RenderSystem/spTextureBase.hpp"

#include <vector>
#include <list>


namespace sp
{
namespace scene
{


/**
Terrain class assumes the work of mesh LOD (level-of-detail) for the model-patches. Used are quad-trees.
*/
class SP_EXPORT Terrain : public MaterialNode
{
    
    public:
        
        Terrain(const video::SHeightMapTexture &HeightMap, const dim::size2di &Resolution, s32 GeoMIPLevels);
        virtual ~Terrain();
        
        /* Rendering */
        
        virtual void render();
        
        void changeHeightMap(
            const video::SHeightMapTexture &HeightMap, const dim::size2di &Resolution, s32 GeoMIPLevels
        );
        
        /* Inline functions */
        
        inline dim::size2di getResolution() const
        {
            return Resolution_;
        }
        
        inline video::MeshBuffer* getTextureReferenceMesh()
        {
            return MeshTexReference_;
        }
        
    protected:
        
        friend void clbTerrainTreeNodeDestructor(TreeNode* Node);
        
        /* === Structures === */
        
        struct STreeNodeData
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
        
        void init();
        void clear();
        
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
            const math::ViewFrustum &Frustum, const dim::matrix4f &Transformation, const dim::aabbox3df &BoundBox) const
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
        
};


} // /namespace scene

} // /namespace sp


#endif



// ================================================================================
