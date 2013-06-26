/*
 * Cg shader context file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "Framework/Cg/spCgShaderContext.hpp"

#if defined(SP_COMPILE_WITH_CG)


#include "Platform/spSoftPixelDeviceOS.hpp"
#include "RenderSystem/Direct3D9/spDirect3D9RenderSystem.hpp"
#include "RenderSystem/Direct3D11/spDirect3D11RenderSystem.hpp"

#ifdef SP_COMPILE_WITH_DIRECT3D11
#   include <Cg/cgD3D11.h>
#endif
#ifdef SP_COMPILE_WITH_DIRECT3D9
#   include <Cg/cgD3D9.h>
#endif
#ifdef SP_COMPILE_WITH_OPENGL
#   include <Cg/cgGL.h>
#endif


namespace sp
{

extern video::RenderSystem* __spVideoDriver;

namespace video
{


CGcontext CgShaderContext::cgContext_;
ERenderSystems CgShaderContext::RendererType_ = RENDERER_DUMMY;

CgShaderContext::CgShaderContext()
{
    /* Print Cg library information */
    io::Log::message(getVersion(), 0);
    io::Log::message("Copyright (c) <2001-2011> - NVIDIA Corporation", 0);
    io::Log::message("", 0);
    
    /* General settings */
    CgShaderContext::RendererType_ = __spVideoDriver->getRendererType();
    
    /* Create Cg context */
    cgContext_ = cgCreateContext();
    checkForError("context creation");
    
    /* Configure Cg context */
    switch (CgShaderContext::RendererType_)
    {
        #ifdef SP_COMPILE_WITH_OPENGL
        case RENDERER_OPENGL:
            cgGLSetDebugMode(CG_FALSE); break;
        #endif
        #ifdef SP_COMPILE_WITH_DIRECT3D9
        case RENDERER_DIRECT3D9:
            cgD3D9SetDevice(static_cast<Direct3D9RenderSystem*>(__spVideoDriver)->getDirect3DDevice()); break;
        #endif
        #ifdef SP_COMPILE_WITH_DIRECT3D11
        case RENDERER_DIRECT3D11:
            #ifdef SP_DEBUGMODE
            io::Log::debug("CgShaderContext::CgShaderContext", "Incomplete Cg support for D3D11");
            #endif
            //cgD3D11SetDevice(cgContext_, static_cast<Direct3D11RenderSystem*>(__spVideoDriver)->getDirect3DDevice()); break;
        #endif
        default:
            io::Log::error("Renderer is not supported for Cg");
            cgDestroyContext(cgContext_);
            return;
    }
    
    cgSetParameterSettingMode(cgContext_, CG_DEFERRED_PARAMETER_SETTING);
}
CgShaderContext::~CgShaderContext()
{
    /* Release Cg context */
    #if defined(SP_COMPILE_WITH_DIRECT3D9) || defined(SP_COMPILE_WITH_DIRECT3D11)
    switch (CgShaderContext::RendererType_)
    {
        #ifdef SP_COMPILE_WITH_DIRECT3D9
        case RENDERER_DIRECT3D9:
            cgD3D9SetDevice(0); break;
        #endif
        #ifdef SP_COMPILE_WITH_DIRECT3D11
        case RENDERER_DIRECT3D11:
            cgD3D11SetDevice(cgContext_, 0); break;
        #endif
        default:
            break;
    }
    #endif
    
    cgDestroyContext(cgContext_);
}

io::stringc CgShaderContext::getVersion() const
{
    return "Cg Shader Compiler 3.1"; // todo -> get real version
}

bool CgShaderContext::checkForError(const io::stringc &Situation)
{
    CGerror Error;
    const c8* ErrorStr = cgGetLastErrorString(&Error);
    
    if (Error == CG_NO_ERROR)
        return false;
    
    if (Situation.size())
        io::Log::error("Cg " + Situation + " error: " + io::stringc(ErrorStr));
    else
        io::Log::error("Cg error: " + io::stringc(ErrorStr));
    
    if (Error == CG_COMPILER_ERROR)
        io::Log::message(cgGetLastListing(cgContext_), io::LOG_ERROR | io::LOG_TIME | io::LOG_NOTAB);
    
    return true;
}


} // /namespace video

} // /namespace sp


#endif



// ================================================================================
