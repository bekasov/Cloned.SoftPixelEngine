/*
 * OpenCL buffer file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "Framework/OpenCL/spOpenCLBuffer.hpp"

#if defined(SP_COMPILE_WITH_OPENCL)


#include "Framework/OpenCL/spOpenCLDevice.hpp"
#include "Base/spInputOutputLog.hpp"


namespace sp
{
namespace video
{


OpenCLBuffer::OpenCLBuffer(const EOpenCLBufferStates State, u32 BufferSize) :
    clBuffer_   (0          ),
    State_      (State      ),
    BufferSize_ (BufferSize )
{
    /* Get OpenCL buffer flags */
    cl_mem_flags Flags = 0;
    
    switch (State_)
    {
        case OCLBUFFER_READ:
            Flags = CL_MEM_READ_ONLY;
            break;
        case OCLBUFFER_WRITE:
            Flags = CL_MEM_WRITE_ONLY;
            break;
    }
    
    /* Create OpenCL buffer */
    createBuffer(Flags, 0);
}
OpenCLBuffer::~OpenCLBuffer()
{
    /* Release OpenCL buffer */
    if (clBuffer_)
        clReleaseMemObject(clBuffer_);
}

void OpenCLBuffer::writeBuffer(const void* Buffer, u32 Size, u32 Offset)
{
    if (Buffer && Size > 0 && OpenCLDevice::clQueue_ && clBuffer_)
    {
        clEnqueueWriteBuffer(
            OpenCLDevice::clQueue_, clBuffer_, CL_TRUE, Offset, Size, Buffer, 0, 0, 0
        );
    }
}
void OpenCLBuffer::readBuffer(void* Buffer, u32 Size, u32 Offset)
{
    if (Buffer && Size > 0 && OpenCLDevice::clQueue_ && clBuffer_)
    {
        clEnqueueReadBuffer(
            OpenCLDevice::clQueue_, clBuffer_, CL_TRUE, Offset, Size, Buffer, 0, 0, 0
        );
    }
}


/*
 * ======= Private: =======
 */

void OpenCLBuffer::createBuffer(cl_mem_flags Flags, void* Buffer)
{
    if (OpenCLDevice::clContext_)
    {
        /* Create OpenCL buffer */
        cl_int Error = 0;
        clBuffer_ = clCreateBuffer(OpenCLDevice::clContext_, Flags, BufferSize_, Buffer, &Error);
        OpenCLDevice::checkForError(Error, "Could not create OpenCL buffer");
    }
    else
        io::Log::error("Cannot create OpenCL buffer without valid context");
}


} // /namespace video

} // /namespace sp


#endif



// ================================================================================
