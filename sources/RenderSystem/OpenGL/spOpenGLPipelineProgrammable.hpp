/*
 * OpenGL programmable function renderer header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_OPENGL_PROGRAMMABLEFUNCTIONPIPELINE_H__
#define __SP_OPENGL_PROGRAMMABLEFUNCTIONPIPELINE_H__


#include "Base/spStandard.hpp"

#if defined(SP_COMPILE_WITH_OPENGL) || defined(SP_COMPILE_WITH_OPENGLES2)

#include "RenderSystem/OpenGL/spOpenGLPipelineBase.hpp"
#include "RenderSystem/OpenGL/spOpenGLShader.hpp"


namespace sp
{
namespace video
{


//! OpenGL fixed function render system. This is the parent class of all OpenGL and OpenGL|ES renderers.
class SP_EXPORT GLProgrammableFunctionPipeline : virtual public GLBasePipeline
{
    
    public:
        
        virtual ~GLProgrammableFunctionPipeline();
        
        /* === Render system information === */
        
        io::stringc getShaderVersion() const;
        
        /* === Shader programs === */
        
        ShaderClass* createShaderClass(const VertexFormat* VertexInputLayout = 0);
        
        Shader* createShader(
            ShaderClass* ShaderClassObj, const EShaderTypes Type, const EShaderVersions Version,
            const std::list<io::stringc> &ShaderBuffer, const io::stringc &EntryPoint = ""
        );
        
        void unbindShaders();
        
        #ifdef SP_COMPILE_WITH_OPENGL
        
        ShaderResource* createShaderResource();
        
        bool dispatch(ShaderClass* ShdClass, const dim::vector3d<u32> &GroupSize);
        
        #endif
        
        /* === Render targets === */
        
        bool setRenderTarget(Texture* Target);
        
        /* === Special renderer functions === */
        
        /**
        Enables or disables the balance for render-targets. This is a render specific function and should only
        be used when rendering text into a texture with OpenGL.
        \param Enable: Specifies if the balance is to be enabled or disabled. If enabled each drawing operation
        will be rendered vertically fliped. Thus you have to Adjust your further operations using the final render target texture.
        */
        static void setBalanceRT(bool Enable);
        static bool getBalanceRT();
        
    protected:
        
        /* Functions */
        
        GLProgrammableFunctionPipeline();
        
        /* Members */
        
        static bool EnableBalanceRT_;
        
};


} // /namespace video

} // /namespace sp


#endif

#endif



// ================================================================================
