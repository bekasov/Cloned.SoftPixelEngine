/*
 * OpenGL shader table header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_OPENGL_SHADERTABLE_H__
#define __SP_OPENGL_SHADERTABLE_H__


#include "Base/spStandard.hpp"

#if defined(SP_COMPILE_WITH_OPENGL) || defined(SP_COMPILE_WITH_OPENGLES2)


#include "RenderSystem/spShaderProgram.hpp"
#include "RenderSystem/OpenGL/spOpenGLCoreHeader.hpp"
#include "RenderSystem/OpenGLES/spOpenGLESFunctionsARB.hpp"


namespace sp
{
namespace video
{


class OpenGLShader;

class SP_EXPORT OpenGLShaderTable : public ShaderTable
{
    
    public:
        
        OpenGLShaderTable(VertexFormat* VertexInputLayout = 0);
        ~OpenGLShaderTable();
        
        void bind(const scene::MaterialNode* Object = 0);
        void unbind();
        
        bool link();
        
    private:
        
        friend class OpenGLShader;
        friend class GLProgrammableFunctionPipeline;
        
        /* Functions */
        
        void deleteShaderObject(Shader* ShaderObject);
        bool checkLinkingErrors();
        bool setupUniforms();
        
        void setupVertexFormat(VertexFormat* VertexInputLayout);
        
        void addShaderConstant(const c8* Name, const GLenum Type, u32 Count);
        
        /* Members */
        
        GLhandleARB ProgramObject_;
        
        VertexFormat* VertexInputLayout_;
        
        static GLhandleARB LastProgramObject_;
        
};


} // /namespace scene

} // /namespace sp


#endif

#endif



// ================================================================================
