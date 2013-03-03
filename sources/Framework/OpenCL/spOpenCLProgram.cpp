/*
 * OpenCL program file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "Framework/OpenCL/spOpenCLProgram.hpp"

#if defined(SP_COMPILE_WITH_OPENCL)


#include "Framework/OpenCL/spOpenCLDevice.hpp"
#include "Base/spInputOutputLog.hpp"

#include <boost/foreach.hpp>


namespace sp
{
namespace video
{


OpenCLProgram::OpenCLProgram(const io::stringc &SourceString, const io::stringc &CompilationOptions) :
    clProgram_      (0      ),
    BuildSuccessful_(false  )
{
    BuildSuccessful_ = buildProgram(SourceString, CompilationOptions);
}
OpenCLProgram::~OpenCLProgram()
{
    releaseProgram();
}

bool OpenCLProgram::compile(const io::stringc &SourceString, const io::stringc &CompilationOptions)
{
    /* Release previous OpenCL program and build a new one */
    releaseProgram();
    BuildSuccessful_ = buildProgram(SourceString, CompilationOptions);
    return valid();
}

bool OpenCLProgram::addKernel(const io::stringc &EntryPoint)
{
    if (!clProgram_ || !EntryPoint.size())
        return false;
    
    /* Create new OpenCL kernel */
    cl_int Error = 0;
    cl_kernel Kernel = clCreateKernel(clProgram_, EntryPoint.c_str(), &Error);
    
    try
    {
        OpenCLDevice::checkForError(Error, "Creating OpenCL kernel \"" + EntryPoint + "\" failed");
    }
    catch (const std::string &ErrorStr)
    {
        io::Log::error(ErrorStr);
        return false;
    }
    
    /* Add kernel to the map */
    clKernelList_[EntryPoint.str()] = Kernel;
    
    return true;
}

bool OpenCLProgram::run(
    const io::stringc &EntryPoint, s32 Dimensions, const size_t* GlobalWorkSizes, const size_t* LocalWorkSizes)
{
    /* Check parameter validity */
    if (!EntryPoint.size() || clKernelList_.empty())
    {
        io::Log::error("OpenCL kernel entry point must not be empty");
        return false;
    }
    if (Dimensions < 1 || Dimensions > 3)
    {
        io::Log::error("OpenCL programs can only be executed with a dimension of 1, 2 or 3");
        return false;
    }
    if (!GlobalWorkSizes || !LocalWorkSizes)
        return false;
    
    /* Find entry point */
    std::map<std::string, cl_kernel>::iterator it = clKernelList_.find(EntryPoint.str());
    
    if (it == clKernelList_.end())
    {
        io::Log::error("Could not find OpenCL kernel entry point \"" + EntryPoint + "\"");
        return false;
    }
    
    /* Ececute OpenCL kernel */
    cl_int Error = clEnqueueNDRangeKernel(
        OpenCLDevice::clQueue_, it->second, Dimensions, 0, GlobalWorkSizes, LocalWorkSizes, 0, 0, 0
    );
    
    try
    {
        OpenCLDevice::checkForError(Error, "Executing OpenCL program failed");
    }
    catch (const std::string &ErrorStr)
    {
        io::Log::error(ErrorStr);
        return false;
    }
    
    return true;
}

bool OpenCLProgram::setParameter(const io::stringc &EntryPoint, u32 Index, const void* Buffer, u32 Size)
{
    if (!Buffer || !Size || !EntryPoint.size() || clKernelList_.empty())
        return false;
    
    /* Find entry point */
    std::map<std::string, cl_kernel>::iterator it = clKernelList_.find(EntryPoint.str());
    
    if (it == clKernelList_.end())
    {
        io::Log::error("Could not find OpenCL kernel entry point \"" + EntryPoint + "\"");
        return false;
    }
    
    /* Set kernel parameter */
    cl_int Error = clSetKernelArg(it->second, Index, Size, Buffer);
    
    try
    {
        OpenCLDevice::checkForError(Error, "Setting OpenCL kernel parameter (" + io::stringc(Index) + ") failed");
    }
    catch (const std::string &ErrorStr)
    {
        io::Log::error(ErrorStr);
        return false;
    }
    
    return true;
}

bool OpenCLProgram::setParameter(const io::stringc &EntryPoint, u32 Index, OpenCLBuffer* Buffer)
{
    if (Buffer)
        return setParameter(EntryPoint, Index, &(Buffer->clBuffer_), sizeof(cl_mem));
    return false;
}


/*
 * ======= Private: =======
 */

void OpenCLProgram::releaseProgram()
{
    /* Release OpenCL kernels and program */
    for (std::map<std::string, cl_kernel>::iterator it = clKernelList_.begin(); it != clKernelList_.end(); ++it)
    {
        if (it->second)
            clReleaseKernel(it->second);
    }
    clKernelList_.clear();
    
    if (clProgram_)
        clReleaseProgram(clProgram_);
}

bool OpenCLProgram::buildProgram(const io::stringc &SourceString, const io::stringc &CompilationOptions)
{
    /* Create OpenCL program */
    cl_int Error = 0;
    const c8* SourcePtr = SourceString.c_str();
    
    clProgram_ = clCreateProgramWithSource(
        OpenCLDevice::clContext_, 1, &SourcePtr, 0, &Error
    );
    
    try
    {
        OpenCLDevice::checkForError(Error, "Creating OpenCL program failed");
        
        /* Build OpenCL program */
        Error = clBuildProgram(
            clProgram_, 1, &OpenCLDevice::clDevice_, CompilationOptions.c_str(), 0, 0
        );
        
        if (Error == CL_BUILD_PROGRAM_FAILURE)
        {
            /* Get compiler error log */
            size_t LogSize = 0;
            
            clGetProgramBuildInfo(
                clProgram_, OpenCLDevice::clDevice_, CL_PROGRAM_BUILD_LOG, 0, 0, &LogSize
            );
            
            io::stringc BuildLog(io::stringc::space(LogSize));
            
            clGetProgramBuildInfo(
                clProgram_, OpenCLDevice::clDevice_, CL_PROGRAM_BUILD_LOG, LogSize, &BuildLog[0], 0
            );
            
            io::Log::message(BuildLog, io::LOG_ERROR | io::LOG_TIME | io::LOG_NOTAB);
            
            return false;
        }
        
        OpenCLDevice::checkForError(Error, "Building OpenCL program failed");
    }
    catch (const std::string &ErrorStr)
    {
        io::Log::error(ErrorStr);
        return false;
    }
    
    return true;
}


} // /namespace video

} // /namespace sp


#endif



// ================================================================================
