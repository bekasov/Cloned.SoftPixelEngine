/*
 * Mesh saver header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_MESHSAVER_H__
#define __SP_MESHSAVER_H__


#include "Base/spStandard.hpp"
#include "Base/spInputOutput.hpp"
#include "Base/spBaseFileFormatHandler.hpp"
#include "SceneGraph/Animation/spAnimation.hpp"
#include "Base/spDimension.hpp"
#include "SceneGraph/spSceneMesh.hpp"
#include "RenderSystem/spTextureBase.hpp"


namespace sp
{
namespace scene
{


/*
 * ======= MeshSaver class =======
 */

class SP_EXPORT MeshSaver : public io::BaseFileFormatHandler
{
    
    public:
        
        virtual ~MeshSaver()
        {
        }
        
        virtual bool saveMesh(Mesh* Model, const io::stringc &Filename) = 0;
        
    protected:
        
        /* Protected functions */
        
        MeshSaver() :
            io::BaseFileFormatHandler   (   ),
            Mesh_                       (0  ),
            Surface_                    (0  ),
            Anim_                       (0  )
        {
        }
        
        inline bool openSaveFile(Mesh* Model, const io::stringc &Filename)
        {
            Filename_   = Filename;
            Mesh_       = Model;
            
            return ( File_ = FileSys_.openFile(Filename, io::FILE_WRITE) ) != 0;
        }
        
        /* Members */
        
        Mesh* Mesh_;
        video::MeshBuffer* Surface_;
        Animation* Anim_;
        
};


} // /namespace scene

} // /namespace sp


#endif





// ================================================================================
