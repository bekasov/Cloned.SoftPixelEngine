/*
 * Shader program file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "RenderSystem/spShaderProgram.hpp"

#include <boost/foreach.hpp>


namespace sp
{
namespace video
{


Shader::Shader(
    ShaderClass* ShdClass, const EShaderTypes Type, const EShaderVersions Version) :
    Type_                   (Type                                                           ),
    Version_                (Version                                                        ),
    ShdClass_               (ShdClass                                                       ),
    HighLevel_              (Type_ != SHADER_VERTEX_PROGRAM && Type_ != SHADER_PIXEL_PROGRAM),
    OwnShaderClass_         (ShdClass_ == 0                                                 ),
    CompiledSuccessfully_   (false                                                          )
{
}
Shader::~Shader()
{
    if (OwnShaderClass_ && ShdClass_)
        delete ShdClass_;
}

bool Shader::compile(
    const std::list<io::stringc> &ShaderBuffer, const io::stringc &EntryPoint, const c8** CompilerOptions)
{
    return false; // do nothing
}

bool Shader::setConstant(s32 Number, const EConstantTypes Type, const f32 Value)
{
    return false; // do nothing
}
bool Shader::setConstant(s32 Number, const EConstantTypes Type, const f32* Buffer, s32 Count)
{
    return false; // do nothing
}

bool Shader::setConstant(s32 Number, const EConstantTypes Type, const dim::vector3df &Position)
{
    return false; // do nothing
}
bool Shader::setConstant(s32 Number, const EConstantTypes Type, const video::color &Color)
{
    return false; // do nothing
}
bool Shader::setConstant(s32 Number, const EConstantTypes Type, const dim::matrix4f &Matrix)
{
    return false; // do nothing
}

bool Shader::setConstant(const io::stringc &Name, const f32 Value)
{
    return false; // do nothing
}
bool Shader::setConstant(const io::stringc &Name, const f32* Buffer, s32 Count)
{
    return false; // do nothing
}

bool Shader::setConstant(const io::stringc &Name, const s32 Value)
{
    return false; // do nothing
}
bool Shader::setConstant(const io::stringc &Name, const s32* Buffer, s32 Count)
{
    return false; // do nothing
}

bool Shader::setConstant(const io::stringc &Name, const dim::vector3df &Position)
{
    return false; // do nothing
}
bool Shader::setConstant(const io::stringc &Name, const dim::vector4df &Position)
{
    return false; // do nothing
}
bool Shader::setConstant(const io::stringc &Name, const video::color &Color)
{
    return false; // do nothing
}
bool Shader::setConstant(const io::stringc &Name, const dim::matrix4f &Matrix)
{
    return false; // do nothing
}

//bool Shader::setConstant(const io::stringc &Name, const Texture* Tex) { return false; }

bool Shader::setConstant(const f32* Buffer, s32 StartRegister, s32 ConstAmount)
{
    return false; // do nothing
}

bool Shader::setConstantBuffer(const io::stringc &Name, const void* Buffer)
{
    return false; // do nothing
}
bool Shader::setConstantBuffer(u32 Number, const void* Buffer)
{
    return false; // do nothing
}

void Shader::addOption(std::list<io::stringc> &ShaderCompilerOp, const io::stringc &Op)
{
    ShaderCompilerOp.push_back("#define " + Op + "\n");
}


/*
 * ======= Protected: =======
 */

void Shader::updateShaderClass()
{
    if (ShdClass_)
    {
        ShdClass_->HighLevel_ = HighLevel_;
        
        switch (Type_)
        {
            case SHADER_VERTEX_PROGRAM:
            case SHADER_VERTEX:
                ShdClass_->VertexShader_ = this; break;
            case SHADER_PIXEL_PROGRAM:
            case SHADER_PIXEL:
                ShdClass_->PixelShader_ = this; break;
            case SHADER_GEOMETRY:
                ShdClass_->GeometryShader_ = this; break;
            case SHADER_HULL:
                ShdClass_->HullShader_ = this; break;
            case SHADER_DOMAIN:
                ShdClass_->DomainShader_ = this; break;
            case SHADER_COMPUTE:
                ShdClass_->ComputeShader_ = this; break;
            default:
                break;
        }
    }
}

void Shader::printError(const io::stringc &Message)
{
    io::Log::message(Message, io::LOG_ERROR | io::LOG_TIME | io::LOG_NOTAB);
}
void Shader::printWarning(const io::stringc &Message)
{
    io::Log::message(Message, io::LOG_WARNING | io::LOG_TIME | io::LOG_NOTAB);
}

void Shader::createProgramString(const std::list<io::stringc> &ShaderBuffer, c8* &ProgramBuffer)
{
    /* Allocate program string buffer */
    u32 Len = 0;
    
    foreach (const io::stringc &Str, ShaderBuffer)
        Len += Str.size();
    
    ProgramBuffer = new c8[Len + 1];
    
    /* Fill string buffer */
    c8* Ptr = ProgramBuffer;
    
    foreach (const io::stringc &Str, ShaderBuffer)
    {
        if (Str.size())
        {
            memcpy(Ptr, Str.c_str(), Str.size());
            Ptr += Str.size();
        }
    }
    
    *Ptr = 0;
}


} // /namespace scene

} // /namespace sp



// ================================================================================
