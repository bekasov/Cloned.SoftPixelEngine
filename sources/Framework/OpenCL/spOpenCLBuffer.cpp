/*
 * OpenCL buffer file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "Framework/OpenCL/spOpenCLBuffer.hpp"

#if defined(SP_COMPILE_WITH_OPENCL)


#include "Framework/OpenCL/spOpenCLDevice.hpp"
#include "RenderSystem/spTextureBase.hpp"
#include "RenderSystem/spRenderSystem.hpp"
#include "Base/spInputOutputLog.hpp"
#include "Base/spMeshBuffer.hpp"

#include <GL/gl.h>


namespace sp
{

extern video::RenderSystem* __spVideoDriver;

namespace video
{


OpenCLBuffer::OpenCLBuffer(const EOpenCLBufferStates State, u32 BufferSize) :
    clBuffer_       (0          ),
    State_          (State      ),
    BufferSize_     (BufferSize ),
    CustomBuffer_   (true       )
{
    /* Create OpenCL buffer */
    createBuffer(getMemFlags(), 0);
}
OpenCLBuffer::OpenCLBuffer(const EOpenCLBufferStates State, video::Texture* TexBuffer) :
    clBuffer_       (0      ),
    State_          (State  ),
    BufferSize_     (0      ),
    CustomBuffer_   (false  )
{
    if (!TexBuffer)
        throw "Invalid texture object for OpenCL buffer";
}
OpenCLBuffer::OpenCLBuffer(const EOpenCLBufferStates State, video::MeshBuffer* MeshBuffer) :
    clBuffer_       (0      ),
    State_          (State  ),
    BufferSize_     (0      ),
    CustomBuffer_   (false  )
{
    if (!MeshBuffer)
        throw "Invalid mesh buffer object for OpenCL buffer";
    
    cl_int Error = 0;
    
    switch (__spVideoDriver->getRendererType())
    {
        #ifdef SP_COMPILE_WITH_OPENGL
        case video::RENDERER_OPENGL:
            clBuffer_ = clCreateFromGLBuffer(
                OpenCLDevice::clContext_, getMemFlags(), *(u32*)MeshBuffer->getVertexBufferID(), &Error
            );
            break;
        #endif
        
        #ifdef SP_COMPILE_WITH_DIRECT3D11
        case video::RENDERER_DIRECT3D11:
            //clCreateFromD3D11BufferKHR();
            break;
        #endif
        
        default:
            throw "Render system can not be used with OpenCL";
    }
    
    OpenCLDevice::checkForError(Error, "Could not create OpenCL buffer");
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

void OpenCLBuffer::lock()
{
    if (clBuffer_ && !CustomBuffer_)
    {
        glFinish();
        clEnqueueAcquireGLObjects(OpenCLDevice::clQueue_, 1, &clBuffer_, 0, 0, 0);
    }
}

void OpenCLBuffer::unlock()
{
    if (clBuffer_ && !CustomBuffer_)
    {
        clEnqueueReleaseGLObjects(OpenCLDevice::clQueue_, 1, &clBuffer_, 0, 0, 0);
        clFinish(OpenCLDevice::clQueue_);
    }
}


/*
 * ======= Private: =======
 */

cl_mem_flags OpenCLBuffer::getMemFlags() const
{
    switch (State_)
    {
        case OCLBUFFER_READ:
            return CL_MEM_READ_ONLY;
        case OCLBUFFER_WRITE:
            return CL_MEM_WRITE_ONLY;
        case OCLBUFFER_READ_WRITE:
            return CL_MEM_READ_WRITE;
        default:
            throw "Invalid state for OpenCL buffer";
    }
    return 0;
}

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
