/*
 * Sparse octree voxelizer file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "RenderSystem/DeferredRenderer/spSparseOctreeVoxelizer.hpp"
#include "RenderSystem/spRenderSystem.hpp"
#include "SceneGraph/spSceneGraph.hpp"


namespace sp
{

extern video::RenderSystem* GlbRenderSys;

namespace video
{


SparseOctreeVoxelizer::SparseOctreeVoxelizer() :
    ShdClass_(0)
{
}
SparseOctreeVoxelizer::~SparseOctreeVoxelizer()
{
}

bool SparseOctreeVoxelizer::generateSparseOctree(Texture* VolumeTexture, scene::SceneGraph* Graph, const dim::aabbox3df &BoundVolume)
{
    /* Validate argument list */
    if (!VolumeTexture || !Graph)
    {
        #ifdef SP_DEBUGMODE
        io::Log::debug("SparseOctreeVoxelizer::generateSparseOctree", "Null pointers passed");
        #endif
        return false;
    }
    if (VolumeTexture->getDimension() != TEXTURE_3D)
    {
        io::Log::error("Cannot generate sparse voxel octree when texture is not a 3D texture");
        return false;
    }
    if (BoundVolume.getVolume() <= math::ROUNDING_ERROR)
    {
        io::Log::error("Cannot generate sparse voxel octree when bounding volume is nearly zero");
        return false;
    }
    
    /* Render scene with diffuse map only and setup render states */
    const s32 PrevTexLayerMask = GlbRenderSys->getTexLayerVisibleMask();
    GlbRenderSys->setTexLayerVisibleMask(TEXLAYERFLAG_DIFFUSE);
    
    GlbRenderSys->setColorMask(false, false, false, false);
    GlbRenderSys->setDepthMask(false);

    /* Setup shader class for voxelization pipeline */
    GlbRenderSys->setGlobalShaderClass(ShdClass_);
    
    /* Render scene */
    
    //todo ...
    
    /* Restore previous render states */
    GlbRenderSys->setColorMask(true, true, true, true);
    GlbRenderSys->setDepthMask(true);

    GlbRenderSys->setGlobalShaderClass(0);
    GlbRenderSys->setTexLayerVisibleMask(PrevTexLayerMask);
    
    return true;
}


/*
 * ======= Private: =======
 */

bool SparseOctreeVoxelizer::loadShader()
{
    
    //todo ...
    
    return false;
}


} // /namespace video

} // /namespace sp



// ================================================================================