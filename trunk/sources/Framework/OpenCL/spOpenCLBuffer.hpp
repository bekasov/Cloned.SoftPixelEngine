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


class Texture;
class MeshBuffer;

//! OpenCL buffer states.
enum EOpenCLBufferStates
{
    OCLBUFFER_READ,         //!< Only read access.
    OCLBUFFER_WRITE,        //!< Only write access.
    OCLBUFFER_READ_WRITE,   //!< Read and write access.
};


/**
\since Version 3.2
\ingroup group_gpgpu
*/
class SP_EXPORT OpenCLBuffer
{
    
    public:
        
        OpenCLBuffer(const EOpenCLBufferStates State, u32 BufferSize);
        OpenCLBuffer(const EOpenCLBufferStates State, video::Texture* TexBuffer);
        OpenCLBuffer(const EOpenCLBufferStates State, video::MeshBuffer* MeshBuffer);
        ~OpenCLBuffer();
        
        /* === Functions === */
        
        void writeBuffer(const void* Buffer, u32 Size, u32 Offset = 0);
        void readBuffer(void* Buffer, u32 Size, u32 Offset = 0);
        
        void lock();
        void unlock();
        
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
        
        /**
        Returns true if this is a custom buffer. Otherwise it refers to a hardware mesh- or texture buffer.
        In this case you have to care about locking and unlocking the buffer before running a shader which is using it.
        */
        inline bool isCustomBuffer() const
        {
            return CustomBuffer_;
        }
        
    private:
        
        friend class OpenCLProgram;
        
        /* === Functions === */
        
        cl_mem_flags getMemFlags() const;
        
        void createBuffer(cl_mem_flags Flags, void* Buffer);
        
        /* === Members === */
        
        cl_mem clBuffer_;
        
        EOpenCLBufferStates State_;
        u32 BufferSize_;
        
        bool CustomBuffer_; //!< If false this buffer refers to a mesh or texture buffer.
        
};


} // /namespace video

} // /namespace sp


#endif

#endif



// ================================================================================
