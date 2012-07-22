/*
 * Cg shader program file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "Framework/Cg/spCgShaderProgram.hpp"

#if defined(SP_COMPILE_WITH_CG)


#include "Framework/Cg/spCgShaderContext.hpp"


namespace sp
{
namespace video
{


CGparameter CgShaderProgram::ActiveParam_ = 0;

CgShaderProgram::CgShaderProgram(ShaderTable* Table, const EShaderTypes Type, const EShaderVersions Version)
    : Shader(Table, Type, Version)
{
    updateTable();
}
CgShaderProgram::~CgShaderProgram()
{
    cgDestroyProgram(cgProgram_);
}

bool CgShaderProgram::compile(const std::vector<io::stringc> &ShaderBuffer, const io::stringc &EntryPoint)
{
    io::stringc SourceCodeString;
    
    for (std::vector<io::stringc>::const_iterator it = ShaderBuffer.begin(); it != ShaderBuffer.end(); ++it)
        SourceCodeString += *it + "\n";
    
    CompiledSuccessfully_ = compileCg(SourceCodeString, EntryPoint);
    
    setupShaderConstants();
    
    return CompiledSuccessfully_;
}

bool CgShaderProgram::setConstant(const io::stringc &Name, const f32 Value)
{
    if (getParam(Name))
    {
        cgSetParameter1f(ActiveParam_, Value);
        return true;
    }
    return false;
}
bool CgShaderProgram::setConstant(const io::stringc &Name, const f32* Buffer, s32 Count)
{
    if (getParam(Name))
    {
        cgSetParameterValuefc(ActiveParam_, Count, Buffer);
        return true;
    }
    return false;
}

bool CgShaderProgram::setConstant(const io::stringc &Name, const s32 Value)
{
    if (getParam(Name))
    {
        cgSetParameter1i(ActiveParam_, Value);
        return true;
    }
    return false;
}
bool CgShaderProgram::setConstant(const io::stringc &Name, const s32* Buffer, s32 Count)
{
    if (getParam(Name))
    {
        cgSetParameterValueic(ActiveParam_, Count, Buffer);
        return true;
    }
    return false;
}

bool CgShaderProgram::setConstant(const io::stringc &Name, const dim::vector3df &Position)
{
    if (getParam(Name))
    {
        cgSetParameter3fv(ActiveParam_, &Position.X);
        return true;
    }
    return false;
}
bool CgShaderProgram::setConstant(const io::stringc &Name, const dim::vector4df &Position)
{
    if (getParam(Name))
    {
        cgSetParameter4fv(ActiveParam_, &Position.X);
        return true;
    }
    return false;
}
bool CgShaderProgram::setConstant(const io::stringc &Name, const video::color &Color)
{
    if (getParam(Name))
    {
        f32 Buffer[4];
        Color.getFloatArray(Buffer);
        cgSetParameter4fv(ActiveParam_, Buffer);
        return true;
    }
    return false;
}
bool CgShaderProgram::setConstant(const io::stringc &Name, const dim::matrix4f &Matrix)
{
    if (getParam(Name))
    {
        cgSetMatrixParameterfc(ActiveParam_, Matrix.getArray());
        return true;
    }
    return false;
}


/*
 * ======= Protected: =======
 */

bool CgShaderProgram::createProgram(
    const io::stringc &SourceCodeString, const io::stringc &EntryPoint, const c8** ProfileOptions)
{
    cgProgram_ = cgCreateProgram(
        CgShaderContext::cgContext_, CG_SOURCE, SourceCodeString.c_str(), cgProfile_, EntryPoint.c_str(), ProfileOptions
    );
    return !CgShaderContext::checkForError("shader program creation");
}

bool CgShaderProgram::getParam(const io::stringc &Name)
{
    std::map<std::string, CGparameter>::iterator it = ParameterMap_.find(Name.str());
    
    if (it == ParameterMap_.end())
    {
        CgShaderProgram::ActiveParam_ = cgGetNamedParameter(cgProgram_, Name.c_str());
        
        if (CgShaderProgram::ActiveParam_)
        {
            ParameterMap_[Name.str()] = CgShaderProgram::ActiveParam_;
            return true;
        }
        
        io::Log::error("Could not found Cg parameter \"" + Name + "\"");
        return false;
    }
    
    CgShaderProgram::ActiveParam_ = it->second;
    
    return true;
}

bool CgShaderProgram::setupShaderConstants()
{
    /* Get first Cg parameter to iterate over all shader constants */
    CGparameter Param = cgGetFirstParameter(cgProgram_, CG_PROGRAM);
    
    SShaderConstant Constant;
    
    while (Param)
    {
        /* Setup constant name, array size */
        Constant.Name   = io::stringc(cgGetParameterName(Param));
        Constant.Count  = 1;
        
        /* Determine constant type */
        Constant.Type = CONSTANT_UNKNOWN;
        
        const u32 RowColSize = math::Max(cgGetParameterRows(Param), cgGetParameterColumns(Param));
        const CGtype Type = cgGetParameterType(Param);
        
        switch (cgGetParameterClass(Param))
        {
            case CG_PARAMETERCLASS_SCALAR:
            {
                switch (Type)
                {
                    case CG_BOOL:
                        Constant.Type = CONSTANT_BOOL; break;
                    case CG_INT:
                        Constant.Type = CONSTANT_INT; break;
                    case CG_FLOAT:
                        Constant.Type = CONSTANT_FLOAT; break;
                }
            }
            break;
            
            case CG_PARAMETERCLASS_VECTOR:
            {
                switch (Type)
                {
                    case CG_FLOAT2:
                        Constant.Type = CONSTANT_VECTOR2; break;
                    case CG_FLOAT3:
                        Constant.Type = CONSTANT_VECTOR3; break;
                    case CG_FLOAT4:
                        Constant.Type = CONSTANT_VECTOR4; break;
                }
            }
            break;
            
            case CG_PARAMETERCLASS_MATRIX:
            {
                switch (Type)
                {
                    case CG_FLOAT2x2:
                        Constant.Type = CONSTANT_MATRIX2; break;
                    case CG_FLOAT3x3:
                        Constant.Type = CONSTANT_MATRIX3; break;
                    case CG_FLOAT4x4:
                        Constant.Type = CONSTANT_MATRIX4; break;
                }
            }
            break;
            
            case CG_PARAMETERCLASS_ARRAY:
            {
                //todo ...
            }
            break;
            
            default:
                Param = cgGetNextParameter(Param);
                continue;
        }
        
        /* add constant to the list and get next Cg parameter */
        ConstantList_.push_back(Constant);
        
        #if 0 //!!!
        io::Log::message(
            " DEB: [ spCgShaderProgram.cpp ] New Shader Constant: \"" + io::stringc(Constant.Name) + "\", Count = " + io::stringc(Constant.Count) +
            ", Type = " + io::stringc(static_cast<s32>(Constant.Type))
        );
        #endif
        
        Param = cgGetNextParameter(Param);
    }
    
    return true;
}


} // /namespace video

} // /namespace sp


#endif



// ================================================================================
