/*
 * Shader class file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "RenderSystem/spShaderClass.hpp"
#include "RenderSystem/spRenderSystem.hpp"


namespace sp
{

extern video::RenderSystem* __spVideoDriver;

namespace video
{


ShaderClass::ShaderClass() :
    ObjectCallback_         (0      ),
    SurfaceCallback_        (0      ),
    VertexShader_           (0      ),
    PixelShader_            (0      ),
    GeometryShader_         (0      ),
    HullShader_             (0      ),
    DomainShader_           (0      ),
    ComputeShader_          (0      ),
    HighLevel_              (true   ),
    CompiledSuccessfully_   (false  )
{
}
ShaderClass::~ShaderClass()
{
}

EShaderVersions ShaderClass::getShaderVersion(s32 Flags)
{
    if (Flags & SHADERBUILD_GLSL)
        return GLSL_VERSION_1_20;
    
    if (Flags & SHADERBUILD_CG)
        return CG_VERSION_2_0;
    
    if (Flags & SHADERBUILD_HLSL3)
    {
        if (Flags & SHADERBUILD_VERTEX) return HLSL_VERTEX_3_0;
        if (Flags & SHADERBUILD_PIXEL ) return HLSL_PIXEL_3_0;
    }
    
    if (Flags & SHADERBUILD_HLSL5)
    {
        if (Flags & SHADERBUILD_VERTEX    ) return HLSL_VERTEX_5_0;
        if (Flags & SHADERBUILD_PIXEL     ) return HLSL_PIXEL_5_0;
        if (Flags & SHADERBUILD_GEOMETRY  ) return HLSL_GEOMETRY_5_0;
        if (Flags & SHADERBUILD_HULL      ) return HLSL_HULL_5_0;
        if (Flags & SHADERBUILD_DOMAIN    ) return HLSL_DOMAIN_5_0;
    }
    
    return DUMMYSHADER_VERSION;
}

bool ShaderClass::build(
    const io::stringc &Name,
    ShaderClass* &ShdClass,
    VertexFormat* VertFmt,
    
    const std::list<io::stringc>* ShdBufferVertex,
    const std::list<io::stringc>* ShdBufferPixel,
    
    const io::stringc &VertexMain,
    const io::stringc &PixelMain,
    
    s32 Flags)
{
    if (!ShdBufferVertex)
        return false;
    
    /* Create shader class */
    if (Flags & SHADERBUILD_CG)
    {
        #ifdef SP_COMPILE_WITH_CG
        ShdClass = __spVideoDriver->createCgShaderClass(VertFmt);
        #else
        io::Log::error("Engine was not compiled with Cg Toolkit");
        return false;
        #endif
    }
    else
        ShdClass = __spVideoDriver->createShaderClass(VertFmt);
    
    if (!ShdClass)
        return false;
    
    /* Create vertex- and pixel shaders */
    if (Flags & SHADERBUILD_CG)
    {
        __spVideoDriver->createCgShader(
            ShdClass, SHADER_VERTEX, getShaderVersion(Flags | SHADERBUILD_VERTEX), *ShdBufferVertex, VertexMain
        );
        
        if (ShdBufferPixel)
        {
            __spVideoDriver->createCgShader(
                ShdClass, SHADER_PIXEL, getShaderVersion(Flags | SHADERBUILD_PIXEL), *ShdBufferPixel, PixelMain
            );
        }
    }
    else
    {
        __spVideoDriver->createShader(
            ShdClass, SHADER_VERTEX, getShaderVersion(Flags | SHADERBUILD_VERTEX), *ShdBufferVertex, VertexMain
        );
        
        if (ShdBufferPixel)
        {
            __spVideoDriver->createShader(
                ShdClass, SHADER_PIXEL, getShaderVersion(Flags | SHADERBUILD_PIXEL), *ShdBufferPixel, PixelMain
            );
        }
    }
    
    /* Compile and link shaders */
    if (!ShdClass->link())
    {
        io::Log::error("Compiling " + Name + " shader failed");
        return false;
    }
    
    return true;
}

void ShaderClass::printError(const io::stringc &Message)
{
    io::Log::message(Message, io::LOG_ERROR | io::LOG_TIME | io::LOG_NOTAB);
}
void ShaderClass::printWarning(const io::stringc &Message)
{
    io::Log::message(Message, io::LOG_WARNING | io::LOG_TIME | io::LOG_NOTAB);
}


} // /namespace scene

} // /namespace sp



// ================================================================================
