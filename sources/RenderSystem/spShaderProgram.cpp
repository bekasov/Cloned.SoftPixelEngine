/*
 * Shader program file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "RenderSystem/spShaderProgram.hpp"
#include "RenderSystem/spRenderSystem.hpp"
#include "RenderSystem/spConstantBuffer.hpp"
#include "Base/spMemoryManagement.hpp"

#include <boost/foreach.hpp>


namespace sp
{

extern video::RenderSystem* GlbRenderSys;

namespace video
{


SShaderConstant Shader::EmptyConstant_;

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

const SShaderConstant* Shader::getConstantRef(const io::stringc &Name) const
{
    return 0;
}
const SShaderConstant& Shader::getConstant(const io::stringc &Name) const
{
    return Shader::EmptyConstant_;
}

ConstantBuffer* Shader::getConstantBuffer(const io::stringc &Name) const
{
    foreach (ConstantBuffer* ConstBuffer, ConstantBufferList_)
    {
        if (ConstBuffer->getName() == Name)
            return ConstBuffer;
    }
    return 0;
}


/* === Index-based constant functions === */

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


/* === String-based constant functions === */

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


/* === Structure-based constnat functions === */

bool Shader::setConstant(const SShaderConstant &Constant, const f32 Value)
{
    return setConstant(Constant.Name, Value);
}
bool Shader::setConstant(const SShaderConstant &Constant, const f32* Buffer, s32 Count)
{
    return setConstant(Constant.Name, Buffer, Count);
}

bool Shader::setConstant(const SShaderConstant &Constant, const s32 Value)
{
    return setConstant(Constant.Name, Value);
}
bool Shader::setConstant(const SShaderConstant &Constant, const s32* Buffer, s32 Count)
{
    return setConstant(Constant.Name, Buffer, Count);
}

bool Shader::setConstant(const SShaderConstant &Constant, const dim::vector3df &Vector)
{
    return setConstant(Constant.Name, Vector);
}
bool Shader::setConstant(const SShaderConstant &Constant, const dim::vector4df &Vector)
{
    return setConstant(Constant.Name, Vector);
}
bool Shader::setConstant(const SShaderConstant &Constant, const video::color &Color)
{
    return setConstant(Constant.Name, Color);
}
bool Shader::setConstant(const SShaderConstant &Constant, const dim::matrix4f &Matrix)
{
    return setConstant(Constant.Name, Matrix);
}


/* === Other constant functions === */

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


/* === Static functions === */

io::stringc Shader::getOption(const io::stringc &Op)
{
    return "#define " + Op + "\n";
}
void Shader::addOption(std::list<io::stringc> &ShaderCompilerOp, const io::stringc &Op)
{
    ShaderCompilerOp.push_back(getOption(Op));
}

void Shader::addShaderCore(std::list<io::stringc> &ShaderCode, bool UseCg)
{
    if (!GlbRenderSys)
    {
        io::Log::error("Can not add shader core without active render system");
        return;
    }
    
    if (UseCg)
    {
        ShaderCode.push_back(
            #include "Resources/spShaderCoreStr.cg"
        );
    }
    else
    {
        switch (GlbRenderSys->getRendererType())
        {
            case RENDERER_OPENGL:
            {
                ShaderCode.push_back(
                    #include "Resources/spShaderCoreStr.glsl"
                );
            }
            break;
            
            case RENDERER_DIRECT3D9:
            {
                ShaderCode.push_back(
                    #include "Resources/spShaderCoreStr.hlsl3"
                );
            }
            break;
            
            case RENDERER_DIRECT3D11:
            {
                ShaderCode.push_back(
                    #include "Resources/spShaderCoreStr.hlsl5"
                );
            }
            break;
            
            default:
                break;
        }
    }
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
