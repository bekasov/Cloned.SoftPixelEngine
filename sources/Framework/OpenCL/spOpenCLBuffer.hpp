/*
 * OpenCL buffer header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_OPENCL_BUFFER_H__
#define __SP_OPENCL_BUFFER_H__


#include "Base/spStandard.hpp"

#if defined(SP_COMPILE_WITH_OPENCL)


#include "Framework/OpenCL/spOpenCLCoreHeader.hpp"


namespace sp
{
namespace video
{


enum EOpenCLBufferStates
{
    OCLBUFFER_READ,
    OCLBUFFER_WRITE,
};


class SP_EXPORT OpenCLBuffer
{
    
    public:
        
        OpenCLBuffer(const EOpenCLBufferStates State, u32 BufferSize);
        ~OpenCLBuffer();
        
        /* === Functions === */
        
        void writeBuffer(const void* Buffer, u32 Size, u32 Offset = 0);
        void readBuffer(void* Buffer, u32 Size, u32 Offset = 0);
        
        /* === Inline functions === */
        
        //! Returns the size in bytes of this OpenCL buffer.
        inline u32 getBufferSize() const
        {
            return BufferSize_;
        }
        inline EOpenCLBufferStates getState() const
        {
            return State_;
        }
        
    private:
        
        friend class OpenCLProgram;
        
        /* === Functions === */
        
        void createBuffer(cl_mem_flags Flags, void* Buffer);
        
        /* === Members === */
        
        cl_mem clBuffer_;
        
        EOpenCLBufferStates State_;
        u32 BufferSize_;
        
};


} // /namespace video

} // /namespace sp


#endif

#endif



// ================================================================================
