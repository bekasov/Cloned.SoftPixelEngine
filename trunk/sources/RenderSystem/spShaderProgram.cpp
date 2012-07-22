/*
 * Shader program file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "RenderSystem/spShaderProgram.hpp"


namespace sp
{
namespace video
{


Shader::Shader(
    ShaderTable* Table, const EShaderTypes Type, const EShaderVersions Version) :
    Type_                   (Type                                                           ),
    Version_                (Version                                                        ),
    Table_                  (Table                                                          ),
    HighLevel_              (Type_ != SHADER_VERTEX_PROGRAM && Type_ != SHADER_PIXEL_PROGRAM),
    OwnShaderTable_         (Table_ == 0                                                    ),
    CompiledSuccessfully_   (false                                                          )
{
}
Shader::~Shader()
{
}

bool Shader::compile(const std::vector<io::stringc> &ShaderBuffer, const io::stringc &EntryPoint)
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


/*
 * ======= Protected: =======
 */

void Shader::updateTable()
{
    if (Table_)
    {
        Table_->HighLevel_ = HighLevel_;
        
        switch (Type_)
        {
            case SHADER_VERTEX_PROGRAM:
            case SHADER_VERTEX:
                Table_->VertexShader_ = this; break;
            case SHADER_PIXEL_PROGRAM:
            case SHADER_PIXEL:
                Table_->PixelShader_ = this; break;
            case SHADER_GEOMETRY:
                Table_->GeometryShader_ = this; break;
            case SHADER_HULL:
                Table_->HullShader_ = this; break;
            case SHADER_DOMAIN:
                Table_->DomainShader_ = this; break;
            case SHADER_COMPUTE:
                Table_->ComputeShader_ = this; break;
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


} // /namespace scene

} // /namespace sp



// ================================================================================
