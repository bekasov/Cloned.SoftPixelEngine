/*
 * OpenCL device file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */
#include "GL/glx.h"
#include "Framework/OpenCL/spOpenCLDevice.hpp"

#if defined(SP_COMPILE_WITH_OPENCL)


#include "Base/spMemoryManagement.hpp"
#include "Base/spInputOutputLog.hpp"
#include "Base/spInputOutputFileSystem.hpp"

namespace sp
{
namespace video
{


cl_platform_id      OpenCLDevice::clPlatform_   = 0;
cl_context          OpenCLDevice::clContext_    = 0;
cl_command_queue    OpenCLDevice::clQueue_      = 0;
cl_device_id        OpenCLDevice::clDevice_     = 0;

OpenCLDevice::OpenCLDevice()
{
    cl_int Error = 0;
    
    /* Get OpenCL platform */
    Error = clGetPlatformIDs(1, &OpenCLDevice::clPlatform_, 0);
    OpenCLDevice::checkForError(Error, "Could not get OpenCL platform ID");
    
    /* Get OpenCL device */
    Error = clGetDeviceIDs(OpenCLDevice::clPlatform_, CL_DEVICE_TYPE_GPU/*CL_DEVICE_TYPE_DEFAULT*/, 1, &OpenCLDevice::clDevice_, 0);
    OpenCLDevice::checkForError(Error, "Could not get OpenCL device IDs");
    
    /* Create OpenCL context */
    #if defined(SP_PLATFORM_MACOSX)
    CGLContextObj GLContext = CGLGetCurrentContext();
    CGLShareGroupObj GhareGroup = CGLGetShareGroup(GLContext);
    
    cl_context_properties Properties[] = {
        CL_CONTEXT_PROPERTY_USE_CGL_SHAREGROUP_APPLE,
        (cl_context_properties)ShareGroup,
        0
    };
    #else
    cl_context_properties Properties[] = {
        #if defined(SP_PLATFORM_WINDOWS)
        CL_GL_CONTEXT_KHR,      (cl_context_properties)wglGetCurrentContext(),
        CL_WGL_HDC_KHR,         (cl_context_properties)wglGetCurrentDC(),
        #elif defined(SP_PLATFORM_LINUX)
        CL_GL_CONTEXT_KHR,      (cl_context_properties)glXGetCurrentContext(),
        CL_GLX_DISPLAY_KHR,     (cl_context_properties)glXGetCurrentDisplay(),
        #endif
        CL_CONTEXT_PLATFORM,    (cl_context_properties)OpenCLDevice::clPlatform_,
        0
    };
    #endif
    
    OpenCLDevice::clContext_ = clCreateContext(Properties, 1, &OpenCLDevice::clDevice_, 0, 0, &Error);
    OpenCLDevice::checkForError(Error, "Could not create OpenCL context");
    
    /* Create OpenCL command-queue */
    OpenCLDevice::clQueue_ = clCreateCommandQueue(OpenCLDevice::clContext_, OpenCLDevice::clDevice_, 0, &Error);
    OpenCLDevice::checkForError(Error, "Could not create OpenCL command-queue");
    
    /* Load OpenCL KHR extensions */
    loadExtensions();
    
    /* Print OpenCL library information */
    io::Log::message(getVersion(), 0);
    io::Log::message(getDescription() + ": " + getVendor(), 0);
    io::Log::message(
        OpenCLDevice::getDeviceInfo(CL_DEVICE_NAME) +
        " (Driver Version " + OpenCLDevice::getDeviceInfo(CL_DRIVER_VERSION) + ")"
    );
    io::Log::message("", 0);
}
OpenCLDevice::~OpenCLDevice()
{
    /* Delete all OpenCL programs and buffers */
    MemoryManager::deleteList(ProgramList_);
    MemoryManager::deleteList(BufferList_);
    
    /* Release OpenCL objects */
    if (OpenCLDevice::clQueue_)
        clReleaseCommandQueue(OpenCLDevice::clQueue_);
    if (OpenCLDevice::clContext_)
        clReleaseContext(OpenCLDevice::clContext_);
}

io::stringc OpenCLDevice::getVersion() const
{
    return OpenCLDevice::getPlatformInfo(CL_PLATFORM_VERSION);
}
io::stringc OpenCLDevice::getDescription() const
{
    return OpenCLDevice::getPlatformInfo(CL_PLATFORM_NAME);
}
io::stringc OpenCLDevice::getVendor() const
{
    return OpenCLDevice::getPlatformInfo(CL_PLATFORM_VENDOR);
}
io::stringc OpenCLDevice::getExtensionString() const
{
    return OpenCLDevice::getPlatformInfo(CL_PLATFORM_EXTENSIONS);
}

OpenCLProgram* OpenCLDevice::createProgram(const io::stringc &SourceString, const io::stringc &CompilationOptions)
{
    OpenCLProgram* NewProgram = new OpenCLProgram(SourceString, CompilationOptions);
    ProgramList_.push_back(NewProgram);
    return NewProgram;
}

OpenCLProgram* OpenCLDevice::loadProgram(const io::stringc &Filename, const io::stringc &CompilationOptions)
{
    io::Log::message("Load OpenCL program: \"" + Filename + "\"");
    io::Log::upperTab();
    
    OpenCLProgram* NewProgram = createProgram(
        io::FileSystem().readFileString(Filename), CompilationOptions
    );
    
    io::Log::lowerTab();
    
    return NewProgram;
}

void OpenCLDevice::deleteProgram(OpenCLProgram* Program)
{
    MemoryManager::removeElement(ProgramList_, Program, true);
}

OpenCLBuffer* OpenCLDevice::createBuffer(const EOpenCLBufferStates State, u32 BufferSize)
{
    try
    {
        return addBufferToList(new OpenCLBuffer(State, BufferSize));
    }
    catch (const c8* &Err)
    {
        io::Log::error(Err);
    }
    return 0;
}

OpenCLBuffer* OpenCLDevice::createBuffer(const EOpenCLBufferStates State, video::Texture* TexBuffer)
{
    try
    {
        return addBufferToList(new OpenCLBuffer(State, TexBuffer));
    }
    catch (const c8* &Err)
    {
        io::Log::error(Err);
    }
    return 0;
}

OpenCLBuffer* OpenCLDevice::createBuffer(const EOpenCLBufferStates State, video::MeshBuffer* MeshBuffer)
{
    try
    {
        return addBufferToList(new OpenCLBuffer(State, MeshBuffer));
    }
    catch (const c8* &Err)
    {
        io::Log::error(Err);
    }
    return 0;
}

void OpenCLDevice::deleteBuffer(OpenCLBuffer* Buffer)
{
    MemoryManager::removeElement(BufferList_, Buffer, true);
}


/*
 * ======= Private: =======
 */

OpenCLBuffer* OpenCLDevice::addBufferToList(OpenCLBuffer* NewBuffer)
{
    BufferList_.push_back(NewBuffer);
    return NewBuffer;
}

io::stringc OpenCLDevice::getPlatformInfo(cl_platform_info Info)
{
    static const size_t BufferSize = 1024;
    c8 Buffer[BufferSize];
    
    clGetPlatformInfo(clPlatform_, Info, BufferSize, Buffer, 0);
    
    return io::stringc(Buffer);
}

io::stringc OpenCLDevice::getDeviceInfo(cl_device_info Info)
{
    static const size_t BufferSize = 1024;
    c8 Buffer[BufferSize];
    
    clGetDeviceInfo(clDevice_, Info, BufferSize, Buffer, 0);
    
    return io::stringc(Buffer);
}

bool OpenCLDevice::loadExtensions()
{
    #if 0 //!!! How can I load OpenCL extensions to make OpenCL interoperable with D3D11??
    
    //clGetExtensionFunctionAddressForPlatform();
    //clGetExtensionFunctionAddress("clCreateFromD3D11BufferNV");
    
    #endif
    
    return false;
}

io::stringc OpenCLDevice::getErrorString(cl_int Error)
{
    switch (Error)
    {
        case CL_DEVICE_NOT_FOUND:                   return "Device Not Found";
        case CL_DEVICE_NOT_AVAILABLE:               return "Device Not Available";
        case CL_COMPILER_NOT_AVAILABLE:             return "Compiler Not Available";
        case CL_MEM_OBJECT_ALLOCATION_FAILURE:      return "Memory Object Allocation Failure";
        case CL_OUT_OF_RESOURCES:                   return "Out Of Resoruces";
        case CL_OUT_OF_HOST_MEMORY:                 return "Out Of Host Memory";
        case CL_PROFILING_INFO_NOT_AVAILABLE:       return "Profiling Information Not Available";
        case CL_MEM_COPY_OVERLAP:                   return "Memory Copy Overlap";
        case CL_IMAGE_FORMAT_MISMATCH:              return "Image Format Mismatch";
        case CL_IMAGE_FORMAT_NOT_SUPPORTED:         return "Image Format Not Supported";
        case CL_BUILD_PROGRAM_FAILURE:              return "Build Program Failure";
        case CL_MAP_FAILURE:                        return "Map Failure";
        
        case CL_INVALID_VALUE:                      return "Invalid Value";
        case CL_INVALID_DEVICE_TYPE:                return "Invalid Device Type";
        case CL_INVALID_PLATFORM:                   return "Invalid Platform";
        case CL_INVALID_DEVICE:                     return "Invalid Device";
        case CL_INVALID_CONTEXT:                    return "Invalid Context";
        case CL_INVALID_QUEUE_PROPERTIES:           return "Invalid Queue Properties";
        case CL_INVALID_COMMAND_QUEUE:              return "Invalid Command Queue";
        case CL_INVALID_HOST_PTR:                   return "Invalid Host Pointer";
        case CL_INVALID_MEM_OBJECT:                 return "Invalid Memory Object";
        case CL_INVALID_IMAGE_FORMAT_DESCRIPTOR:    return "Invalid Image Format Description";
        case CL_INVALID_IMAGE_SIZE:                 return "Invalid Image Size";
        case CL_INVALID_SAMPLER:                    return "Invalid Sampler";
        case CL_INVALID_BINARY:                     return "Invalid Binary";
        case CL_INVALID_BUILD_OPTIONS:              return "Invalid Build Options";
        case CL_INVALID_PROGRAM:                    return "Invalid Program";
        case CL_INVALID_PROGRAM_EXECUTABLE:         return "Invalid Program Executable";
        case CL_INVALID_KERNEL_NAME:                return "Invalid Kernel Name";
        case CL_INVALID_KERNEL_DEFINITION:          return "Invalid Kernel Definition";
        case CL_INVALID_KERNEL:                     return "Invalid Kernel";
        case CL_INVALID_ARG_INDEX:                  return "Invalid Argument Index";
        case CL_INVALID_ARG_VALUE:                  return "Invalid Argument Value";
        case CL_INVALID_ARG_SIZE:                   return "Invalid Argument Size";
        case CL_INVALID_KERNEL_ARGS:                return "Invalid Kernel Arguments";
        case CL_INVALID_WORK_DIMENSION:             return "Invalid Work Dimension";
        case CL_INVALID_WORK_GROUP_SIZE:            return "Invalid Work Group Size";
        case CL_INVALID_WORK_ITEM_SIZE:             return "Invalid Work Item Size";
        case CL_INVALID_GLOBAL_OFFSET:              return "Invalid Global Offset";
        case CL_INVALID_EVENT_WAIT_LIST:            return "Invalid Event Wait List";
        case CL_INVALID_EVENT:                      return "Invalid Event";
        case CL_INVALID_OPERATION:                  return "Invalid Operation";
        case CL_INVALID_GL_OBJECT:                  return "Invalid OpenGL Object";
        case CL_INVALID_BUFFER_SIZE:                return "Invalid Buffer Size";
        case CL_INVALID_MIP_LEVEL:                  return "Invalid MIP Level";
        case CL_INVALID_GLOBAL_WORK_SIZE:           return "Invalid Global Work Size";
        
        default:
            break;
    }
    
    return "Unknown Error";
}

void OpenCLDevice::checkForError(cl_int Error, const io::stringc &Message)
{
    if (Error != CL_SUCCESS)
        throw (Message + " (" + getErrorString(Error) + ")").str();
}


} // /namespace video

} // /namespace sp


#endif



// ================================================================================
