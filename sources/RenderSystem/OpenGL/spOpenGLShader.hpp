/*
 * OpenGL shader header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_OPENGL_SHADER_H__
#define __SP_OPENGL_SHADER_H__


#include "Base/spStandard.hpp"

#if defined(SP_COMPILE_WITH_OPENGL) || defined(SP_COMPILE_WITH_OPENGLES2)


#include "RenderSystem/OpenGL/spOpenGLShaderClass.hpp"


namespace sp
{
namespace video
{


class SP_EXPORT OpenGLShader : public Shader
{
    
    public:
        
        OpenGLShader(ShaderClass* Table, const EShaderTypes Type, const EShaderVersions Version);
        ~OpenGLShader();
        
        /* Shader compilation */
        
        bool compile(const std::vector<io::stringc> &ShaderBuffer, const io::stringc &EntryPoint = "");
        
        /* Set the constants (by number) */
        
        bool setConstant(s32 Number, const EConstantTypes Type, const f32 Value);
        bool setConstant(s32 Number, const EConstantTypes Type, const f32* Buffer, s32 Count);
        
        bool setConstant(s32 Number, const EConstantTypes Type, const dim::vector3df &Position);
        bool setConstant(s32 Number, const EConstantTypes Type, const video::color &Color);
        bool setConstant(s32 Number, const EConstantTypes Type, const dim::matrix4f &Matrix);
        
        /* Set the constants (by name) */
        
        bool setConstant(const io::stringc &Name, const f32 Value);
        bool setConstant(const io::stringc &Name, const f32* Buffer, s32 Count);
        
        bool setConstant(const io::stringc &Name, const s32 Value);
        bool setConstant(const io::stringc &Name, const s32* Buffer, s32 Count);
        
        bool setConstant(const io::stringc &Name, const dim::vector3df &Position);
        bool setConstant(const io::stringc &Name, const dim::vector4df &Position);
        bool setConstant(const io::stringc &Name, const video::color &Color);
        bool setConstant(const io::stringc &Name, const dim::matrix4f &Matrix);
        
        /* Set the constants for assembly shaders */
        
        bool setConstant(const f32* Buffer, s32 StartRegister, s32 ConstAmount);
        
    private:
        
        friend class OpenGLShaderClass;
        
        /* Functions */
        
        bool compileGLSL(const std::vector<io::stringc> &ShaderBuffer);
        bool checkCompilingErrors();
        
        #ifdef SP_COMPILE_WITH_OPENGL
        bool compileProgram(const std::vector<io::stringc> &ShaderBuffer);
        bool checkCompilingErrors(const io::stringc &ShaderName);
        #endif
        
        void addShaderConstant(const c8* Name, const GLenum Type, u32 Count);
        
        /* Members */
        
        GLuint ProgramObject_;
        GLuint ShaderObject_;
        GLuint AsmShaderProgram_;
        GLenum AsmShaderType_;
        
        s32 CurShaderProgram_;
        
};


} // /namespace scene

} // /namespace sp


#endif

#endif



// ================================================================================
