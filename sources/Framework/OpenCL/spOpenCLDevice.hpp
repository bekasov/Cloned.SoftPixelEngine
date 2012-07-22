/*
 * OpenCL device header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_OPENCL_DEVICE_H__
#define __SP_OPENCL_DEVICE_H__


#include "Base/spStandard.hpp"

#if defined(SP_COMPILE_WITH_OPENCL)


#include "Base/spInputOutputString.hpp"
#include "Framework/OpenCL/spOpenCLCoreHeader.hpp"
#include "Framework/OpenCL/spOpenCLBuffer.hpp"
#include "Framework/OpenCL/spOpenCLProgram.hpp"

#include <list>


namespace sp
{
namespace video
{


class SP_EXPORT OpenCLDevice
{
    
    public:
        
        OpenCLDevice();
        ~OpenCLDevice();
        
        /* === Functions === */
        
        io::stringc getVersion() const;
        io::stringc getDescription() const;
        io::stringc getVendor() const;
        io::stringc getExtensionString() const;
        
        OpenCLProgram* createProgram(const io::stringc &SourceString, const io::stringc &CompilationOptions = "");
        OpenCLProgram* loadProgram(const io::stringc &Filename, const io::stringc &CompilationOptions = "");
        void deleteProgram(OpenCLProgram* Program);
        
        OpenCLBuffer* createBuffer(const EOpenCLBufferStates State, u32 BufferSize);
        void deleteBuffer(OpenCLBuffer* Buffer);
        
    private:
        
        friend class OpenCLBuffer;
        friend class OpenCLProgram;
        
        /* === Functions === */
        
        static io::stringc getPlatformInfo(cl_platform_info Info);
        
        static io::stringc getErrorString(cl_int Error);
        static bool checkForError(cl_int Error, const io::stringc &Message);
        
        /* === Members === */
        
        static cl_platform_id clPlatform_;
        static cl_context clContext_;
        static cl_command_queue clQueue_;
        static cl_device_id clDevice_;
        
        std::list<OpenCLProgram*> ProgramList_;
        std::list<OpenCLBuffer*> BufferList_;
        
};


} // /namespace video

} // /namespace sp


#endif

#endif



// ================================================================================
