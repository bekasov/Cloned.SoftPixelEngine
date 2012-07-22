/*
 * Mesh saver B3D header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_MESHSAVER_B3D_H__
#define __SP_MESHSAVER_B3D_H__


#include "Base/spStandard.hpp"

#ifdef SP_COMPILE_WITH_MESHSAVER_B3D


#include "FileFormats/Mesh/spMeshSaver.hpp"


namespace sp
{
namespace scene
{


class SP_EXPORT MeshSaverB3D : public MeshSaver
{
    
    public:
        
        MeshSaverB3D();
        ~MeshSaverB3D();
        
        bool saveMesh(Mesh* Model, const io::stringc &Filename);
        
    private:
        
        /* === Functions === */
        
        void beginChunk(io::stringc Tag);
        void endChunk();
        
        void saveModelData();
        void writeMesh();
        
        /* === Members === */
        
        std::vector<video::Texture*> TextureList_;
        
        s32 Stack_[100];
        s32 CurPos_;
        
};


} // /namespace scene

} // /namespace sp


#endif

#endif



// ================================================================================
