/*
 * Compute shader header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_COMPUTE_SHADER_H__
#define __SP_COMPUTE_SHADER_H__


#include "Base/spStandard.hpp"


namespace sp
{
namespace video
{


/**
ComputeShaderIO is used to have an access to input- and output buffers between
your program and the compute shader on the GPU.
*/
class SP_EXPORT ComputeShaderIO
{
    
    public:
        
        ComputeShaderIO()
        {
        }
        virtual ~ComputeShaderIO()
        {
        }
        
        /* Functions */
        
        /**
        Adds an input buffer to set user data to a compute shader.
        \param BufferSize: Size (in bytes) of one element of the buffer.
        \param Count: Count of elements.
        \param InitData: Initial data for the buffer. If this parameter is none zero
        it needs to point to a buffer which has the size of [BufferSize * Count] bytes.
        \return Zero based buffer index.
        */
        virtual u32 addInputBuffer(u32 BufferSize, u32 Count, void* InitData = 0)
        {
            return 0;
        }
        
        /**
        Adds an output buffer to get the result from a compute shader.
        \param BufferSize: Size (in bytes) of one element of the buffer.
        \param Count: Count of elements.
        \return Zero based buffer index.
        */
        virtual u32 addOutputBuffer(u32 BufferSize, u32 Count)
        {
            return 0;
        }
        
        /**
        Sets the data of a buffer.
        \param Index: Zero based buffer index. Add an input buffer and use the returned index.
        \param InputBuffer: Input buffer data which is to be set. The buffer needs to point to a buffer data
        with the same size of the created input buffer.
        */
        virtual void setBuffer(const u32 Index, const void* InputBuffer)
        {
        }
        
        /**
        Gets the data of a buffer.
        \param Index: Zero based buffer index. Add an ouput buffer and use the returned index. Input buffers can also be used.
        \param OutputBuffer: Pointer to the buffer data where the output data is to be copied. The buffer needs to point to
        a buffer data with the same size of the created output buffer.
        */
        virtual bool getBuffer(const u32 Index, void* OutputBuffer)
        {
            return false;
        }
        
};


} // /namespace scene

} // /namespace sp


#endif



// ================================================================================
