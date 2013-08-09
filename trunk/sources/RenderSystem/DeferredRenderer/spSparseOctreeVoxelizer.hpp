/*
 * Sparse octree voxelizer header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_SPARSE_OCTREE_VOXELIZER_H__
#define __SP_SPARSE_OCTREE_VOXELIZER_H__


#include "Base/spStandard.hpp"
#include "Base/spDimensionAABB.hpp"
#include "SceneGraph/spSceneCamera.hpp"


namespace sp
{

namespace scene
{
    class SceneGraph;
}

namespace video
{


class Texture;
class ShaderClass;

/**
Class for generating a sparse voxel octree for a specified scene.
This is a modern technique and required Direct3D 11 or OpenGL 4.3.
\since Version 3.3
*/
class SP_EXPORT SparseOctreeVoxelizer
{
    
    public:
        
        SparseOctreeVoxelizer();
        ~SparseOctreeVoxelizer();
        
        /* === Functions === */
        
        bool createShaders();
        void deleteShaders();
        
        /**
        Generates the sparse voxel octree and stores it into a 3D texture.
        \param[in,out] VolumceTexture Pointer to the volume (or rather 3D) texture where the octree is to be stored.
        \param[in] Graph Pointer to the scene graph which is to be voxelized.
        \param[in] BoundVolume Specifies the scene graph's AABB bounding volume.
        \return True if the octree could be generated successful. Otherwise one of the following problems occured:
        1. The arguments are invalid (null pointers are passed).
        2. The specified texture is not a 3D texture (not from type video::TEXTURE_3D).
        3. The required shader programs weren't loaded successfully.
        */
        bool generateSparseOctree(Texture* VolumeTexture, scene::SceneGraph* Graph, const dim::aabbox3df &BoundVolume);
        
    private:
        
        /* === Functions === */
        
        bool loadShader();
        
        /* === Members === */
        
        ShaderClass* ShdClass_;     //!< Voxelization pipeline shader class.
        
        scene::Camera ViewCam_;
        
};


} // /namespace video

} // /namespace sp


#endif



// ================================================================================
