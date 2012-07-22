/*
 * Mesh saver OBJ header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_MESHSAVER_OBJ_H__
#define __SP_MESHSAVER_OBJ_H__


#include "Base/spStandard.hpp"

#ifdef SP_COMPILE_WITH_MESHSAVER_OBJ


#include "FileFormats/Mesh/spMeshSaver.hpp"


namespace sp
{
namespace scene
{


class SP_EXPORT MeshSaverOBJ : public MeshSaver
{
    
    public:
        
        MeshSaverOBJ();
        ~MeshSaverOBJ();
        
        bool saveMesh(Mesh* Model, const io::stringc &Filename);
        
    private:
        
        /* Functions */
        
        void saveModelData();
        
        /* Templates */
        
        template <typename T> void writeValueList(const c8* TypeName, const T* Values, const s32 Count)
        {
            File_->writeBuffer(TypeName, sizeof(c8), strlen(TypeName));
            
            for (s32 i = 0; i < Count; ++i)
            {
                File_->writeValue<c8>(' ');
                File_->writeString(Values[i]);
            }
            
            File_->writeStringN("");
        }
        
        template <typename T> void writeValueListFace(const T* Values)
        {
            File_->writeValue<c8>('f');
            
            for (s32 i = 0; i < 3; ++i)
            {
                File_->writeValue<c8>(' ');
                File_->writeString(Values[i]);
                
                File_->writeValue<c8>('/');
                File_->writeString(Values[i]);
                
                File_->writeValue<c8>('/');
                File_->writeString(Values[i]);
            }
            
            File_->writeStringN("");
        }
        
};


} // /namespace scene

} // /namespace sp


#endif

#endif



// ================================================================================
