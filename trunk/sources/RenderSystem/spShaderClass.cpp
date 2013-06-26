/*
 * Shader class file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "RenderSystem/spShaderClass.hpp"
#include "RenderSystem/spRenderSystem.hpp"
#include "RenderSystem/spConstantBuffer.hpp"
#include "Base/spMemoryManagement.hpp"


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

void ShaderClass::addShaderResource(ShaderResource* Resource)
{
    if (Resource)
        ShaderResources_.push_back(Resource);
}
void ShaderClass::removeShaderResource(ShaderResource* Resource)
{
    MemoryManager::removeElement(ShaderResources_, Resource);
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

bool ShaderClass::loadShaderResourceFile(
    io::FileSystem &FileSys, const io::stringc &Filename, std::list<io::stringc> &ShaderBuffer)
{
    io::File* ShaderFile = FileSys.readResourceFile(Filename);
    
    if (!ShaderFile)
        return false;
    
    io::stringc Line, SubFilename;
    
    while (!ShaderFile->isEOF())
    {
        const io::stringc Line(ShaderFile->readString());
        
        if (hasStringIncludeDirective(Line, SubFilename))
        {
            if (!loadShaderResourceFile(FileSys, Filename.getPathPart() + SubFilename, ShaderBuffer))
            {   
                FileSys.closeFile(ShaderFile);
                return false;
            }
        }
        else
            ShaderBuffer.push_back(Line + "\n");
    }
    
    FileSys.closeFile(ShaderFile);
    
    return true;
}

bool ShaderClass::hasStringIncludeDirective(const io::stringc &Line, io::stringc &Filename)
{
    /* Temporary search states */
    u32 PrevIndex = 0;
    
    static const c8* IncludeDirectiveStr = "include";
    
    bool HasDirectiveStarted = false;
    bool HasDirectiveEnded = false;
    bool HasFilenameStarted = false;
    
    for (u32 i = 0, c = Line.size(); i < c; ++i)
    {
        /* Get current character from line */
        const c8 Chr = Line[i];
        
        if (HasFilenameStarted)
        {
            /* Find quotation mark as end character for the filename */
            if (Chr == '\"')
            {
                /* The include directive has been found and the filename will be returned */
                Filename = Line.section(PrevIndex, i);
                return true;
            }
        }
        else if (HasDirectiveEnded)
        {
            /* Find quotation mark as start charcter for the filename */
            if (Chr == '\"')
            {
                HasFilenameStarted = true;
                PrevIndex = i + 1;
            }
        }
        else if (HasDirectiveStarted)
        {
            /* Get current index of directive string */
            const u32 j = i - 1 - PrevIndex;
            
            /* Check if the include directive has ended */
            if (Chr == ' ' || Chr == '\t' || Chr == '\"')
            {
                /* Check if include directive has been completed */
                if (j == 7)
                {
                    HasDirectiveEnded = true;
                    
                    if (Chr == '\"')
                    {
                        HasFilenameStarted = true;
                        PrevIndex = i + 1;
                    }
                }
                else
                    return false;
            }
            /* Check if the current string part forms the string "include".
               Otherwise the include directive is not part of the line */
            else if (j > 6 || Chr != IncludeDirectiveStr[j])
                return false;
        }
        else
        {
            /* Find the first character which starts the directive and ignore white spaces */
            if (Chr == ' ' || Chr == '\t')
                continue;
            else if (Chr == '#')
            {
                PrevIndex = i;
                HasDirectiveStarted = true;
            }
            else
                return false;
        }
    }
    
    return false;
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
