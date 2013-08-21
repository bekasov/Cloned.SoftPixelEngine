/*
 * OpenGL hardware buffer header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_OPENGL_HARDWARE_BUFFER_H__
#define __SP_OPENGL_HARDWARE_BUFFER_H__


#include "Base/spStandard.hpp"

#if defined(SP_COMPILE_WITH_OPENGL)


#include "Base/spMaterialConfigTypes.hpp"
#include "RenderSystem/OpenGL/spOpenGLCoreHeader.hpp"


namespace sp
{
namespace video
{


class SP_EXPORT GLHardwareBuffer
{
    
    public:
        
        GLHardwareBuffer(const GLenum Target);
        virtual ~GLHardwareBuffer();
        
        /* === Functions === */
        
        void createBuffer();
        void deleteBuffer();

        void setupBuffer(const void* Buffer, u32 Size, const EHWBufferUsage Usage);
        void setupBufferSub(const void* Buffer, u32 Size, u32 Offset = 0);
        
        /* === Inline functions === */

        inline GLuint getBufferID() const
        {
            return HWBuffer_;
        }

        inline bool hasBuffer() const
        {
            return HWBuffer_ != 0;
        }

    private:
        
        /* === Members === */
        
        GLenum Target_;
        GLuint HWBuffer_;
        
};


} // /namespace scene

} // /namespace sp


#endif

#endif



// ================================================================================
