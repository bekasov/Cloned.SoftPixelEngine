/*
 * Cg shader program header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_CG_SHADERPROGRAM_H__
#define __SP_CG_SHADERPROGRAM_H__


#include "Base/spStandard.hpp"

#if defined(SP_COMPILE_WITH_CG)


#include "Framework/Cg/spCgShaderClass.hpp"
#include "RenderSystem/spShaderProgram.hpp"

#include <map>
#include <Cg/cg.h>


namespace sp
{
namespace video
{


//! Class for the "NVIDIA Cg Shader Program".
class SP_EXPORT CgShaderProgram : public Shader
{
    
    public:
        
        virtual ~CgShaderProgram();
        
        /* Functions */
        
        bool compile(
            const std::list<io::stringc> &ShaderBuffer, const io::stringc &EntryPoint = "", const c8** CompilerOptions = 0
        );
        
        /* Set the constants (by name) */
        
        virtual bool setConstant(const io::stringc &Name, const f32 Value);
        virtual bool setConstant(const io::stringc &Name, const f32* Buffer, s32 Count);
        
        virtual bool setConstant(const io::stringc &Name, const s32 Value);
        virtual bool setConstant(const io::stringc &Name, const s32* Buffer, s32 Count);
        
        virtual bool setConstant(const io::stringc &Name, const dim::vector3df &Position);
        virtual bool setConstant(const io::stringc &Name, const dim::vector4df &Position);
        virtual bool setConstant(const io::stringc &Name, const video::color &Color);
        virtual bool setConstant(const io::stringc &Name, const dim::matrix4f &Matrix);
        
    protected:
        
        friend class CgShaderClass;
        
        /* === Functions === */
        
        CgShaderProgram(ShaderClass* Table, const EShaderTypes Type, const EShaderVersions Version);
        
        bool createProgram(
            const io::stringc &SourceCodeString, const io::stringc &EntryPoint, const c8** CompilerOptions = 0
        );
        
        virtual bool compileCg(
            const io::stringc &SourceCodeString, const io::stringc &EntryPoint, const c8** CompilerOptions = 0
        ) = 0;
        
        virtual void bind() = 0;
        virtual void unbind() = 0;
        
        bool getParam(const io::stringc &Name);
        
        bool setupShaderConstants();
        
        /* === Members === */
        
        CGprofile cgProfile_;
        CGprogram cgProgram_;
        
        std::map<std::string, CGparameter> ParameterMap_;
        
        static CGparameter ActiveParam_;
        
};


} // /namespace video

} // /namespace sp


#endif

#endif



// ================================================================================
