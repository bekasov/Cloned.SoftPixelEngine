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
        
        Terrain();
        virtual ~Terrain();
        
        /* === Functions === */
        
        virtual void render();
        
        bool generate(u32 GridSize, u8 GeoMIPLevels);
        
        /* === Inline functions === */
        
        //! \todo The mesh buffer should not be modifyable!
        inline video::MeshBuffer* getMeshBuffer()
        {
            return &MeshBuffer_;
        }
        inline const video::MeshBuffer* getMeshBuffer() const
        {
            return &MeshBuffer_;
        }
        
        inline u32 getGridSize() const
        {
            return GridSize_;
        }
        inline u8 getGeoMIPLevels() const
        {
            return GeoMIPLevels_;
        }
        
    protected:
        
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
        
        enum ETranslateDirections
        {
            TRANSLATE_NONE,
            
            TRANSLATE_LEFT,
            TRANSLATE_RIGHT,
            TRANSLATE_TOP,
            TRANSLATE_BOTTOM,
            
            TRANSLATE_RIGHT_TOP,
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
        
        void addVertex(u32 x, u32 y);
        void resetVertexIndex();
        
        void createTriangle (u32 i0, u32 i1, u32 i2);
        void createQuad     (u32 x, u32 y);
        void createEdge     (u32 x, u32 y, const EChunkTypes Type);
        void createCorner   (u32 x, u32 y, const EChunkTypes Type);
        
        bool isPosCorner            (const VertexPos &Pos, const EChunkTypes CornerType ) const;
        bool isPosEdge              (const VertexPos &Pos, const EChunkTypes EdgeType   ) const;
        EChunkTypes getChunkEdgeType(const VertexPos &Pos, const EChunkTypes CornerType ) const;
        
        void drawChunk(
            const dim::vector3df &GlobalCamPos,
            dim::matrix4f GlobalTransform, dim::matrix4f LocalTransform,
            u8 GeoMIPLevel, const ETranslateDirections Translate = TRANSLATE_NONE
        );
        void drawChunkLeaf(
            const dim::matrix4f &GlobalTransform, const dim::matrix4f &LocalTransform, const EChunkTypes Type
        );
        
        dim::vector3df getCenter(dim::matrix4f Transform) const;
        
        u8 getSubDivision(const dim::vector3df &GlobalCamPos, const dim::matrix4f &Transform) const;
        
        u8 getNeighbourMIPLevel(
            const dim::vector3df &GlobalCamPos, dim::matrix4f Transform,
            const ETranslateDirections Translate = TRANSLATE_NONE
        ) const;
        
        EChunkTypes getChunkType(
            const dim::vector3df &GlobalCamPos, const dim::matrix4f &Transform, u8 GeoMIPLevel
        ) const;
        
        /* === Members === */
        
        u32 GridSize_;
        u8 GeoMIPLevels_;
        f32 InvGridSize_;
        
        SGridChunk GridChunks_[CHUNK_NUM];
        
        dim::vector3df Verts_[6];
        u32 VertIndex_;
        
    private:
        
        /* === Members === */
        
        video::MeshBuffer MeshBuffer_;
        
};


} // /namespace scene

} // /namespace sp


#endif



// ================================================================================
