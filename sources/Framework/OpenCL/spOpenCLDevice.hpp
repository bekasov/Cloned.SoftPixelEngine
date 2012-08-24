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


/**
OpenCL device class for GPGPU (General Purpose Computing on Graphics Processing Units).
\since Version 3.2
\ingroup group_gpgpu
*/
class SP_EXPORT OpenCLDevice
{
    
    public:
        
        OpenCLDevice();
        ~OpenCLDevice();
        
        /* === Functions === */
        
        //! Returns the version of the present OpenCL hardware implementation.
        io::stringc getVersion() const;
        io::stringc getDescription() const;
        io::stringc getVendor() const;
        io::stringc getExtensionString() const;
        
        /**
        Creates a new OpenCL program.
        \param SourceString: Contains the whole OpenCL program source code.
        \param CompilationOptions: Contains the compilation options. Similar to a C++ compiler option.
        Here are a few options:
        \code
        "-D name" -> Predefine 'name' as a macro, with definition 1.
        "-cl-single-precision-constant" -> Treat double precision floating-point constant as single precision constant.
        "-cl-opt-disable" -> This option disables all optimizations. The default is optimizations are enabled.
        \endcode
        For a detailed list take a look at the <a href="http://www.khronos.org/registry/cl/sdk/1.0/docs/man/xhtml/clBuildProgram.html#notes">OpenCL API Docu</a>.
        \return Pointer to the new OpenCLProgram object.
        */
        OpenCLProgram* createProgram(const io::stringc &SourceString, const io::stringc &CompilationOptions = "");
        
        /**
        Loads a new OpenCL program from file.
        \see createProgram
        */
        OpenCLProgram* loadProgram(const io::stringc &Filename, const io::stringc &CompilationOptions = "");
        
        //! Deletes the specified OpenCL program.
        void deleteProgram(OpenCLProgram* Program);
        
        /**
        Creates a new OpenCL memory buffer.
        \param State: Specifies the buffer state or rather if read and/or write access is enabled.
        \param BufferSize: Specifies the buffer size (in bytes).
        \return Pointer to the new OpenCLBuffer object.
        */
        OpenCLBuffer* createBuffer(const EOpenCLBufferStates State, u32 BufferSize);
        OpenCLBuffer* createBuffer(const EOpenCLBufferStates State, video::Texture* TexBuffer);
        OpenCLBuffer* createBuffer(const EOpenCLBufferStates State, video::MeshBuffer* MeshBuffer);
        
        void deleteBuffer(OpenCLBuffer* Buffer);
        
    private:
        
        friend class OpenCLBuffer;
        friend class OpenCLProgram;
        
        /* === Functions === */
        
        OpenCLBuffer* addBufferToList(OpenCLBuffer* NewBuffer);
        
        bool loadExtensions();
        
        static io::stringc getPlatformInfo(cl_platform_info Info);
        
        static io::stringc getErrorString(cl_int Error);
        static void checkForError(cl_int Error, const io::stringc &Message);
        
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
