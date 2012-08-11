/*
 * Deferred renderer header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_DEFERRED_RENDERER_H__
#define __SP_DEFERRED_RENDERER_H__


#include "Base/spStandard.hpp"
#include "RenderSystem/DeferredRenderer/spGBuffer.hpp"


namespace sp
{

namespace scene
{
    class SceneGraph;
}

namespace video
{


class ShaderClass;

//! \todo This is unfinished
class DeferredRenderer
{
    
    public:
        
        DeferredRenderer();
        virtual ~DeferredRenderer();
        
        /* === Functions === */
        
        /**
        Renders the whole given scene with deferred shading onto the
        screen or into the render target if specified.
        \param Graph: Specifies the scene graph which is to be rendered.
        \param RenderTarget: Specifies a render target texture where the whole scene will
        be rendered in. When this parameter is 0 the result will be rendered directly onto the screen.
        This texture must be a valid render target otherwise nothing will be rendered.
        \note First of all a valid g-buffer- and deferred shader must be set.
        \see Texture::setRenderTarget
        \see setGBufferShader
        \see setDeferredShader
        */
        virtual void renderScene(scene::SceneGraph* Graph, video::Texture* RenderTarget = 0);
        
        /* === Inline functions === */
        
        /**
        Sets the g-buffer shader class.
        \see ShaderBuilder
        */
        inline void setGBufferShader(ShaderClass* ShdClass)
        {
            GBufferShader_ = ShdClass;
        }
        //! Returns the g-buffer shader class.
        inline ShaderClass* getGBufferShader() const
        {
            return GBufferShader_;
        }
        
        /**
        Sets the deferred shader class.
        \see ShaderBuilder
        */
        inline void setDeferredShader(ShaderClass* ShdClass)
        {
            DeferredShader_ = ShdClass;
        }
        //! Returns the deferred shader class.
        inline ShaderClass* getDeferredShader() const
        {
            return DeferredShader_;
        }
        
    protected:
        
        /* === Functions === */
        
        //! \warning Does not check for null pointers!
        virtual void renderSceneIntoGBuffer(scene::SceneGraph* Graph);
        virtual void renderDeferredShading(video::Texture* RenderTarget);
        
        /* === Members === */
        
        GBuffer GBuffer_;
        
        ShaderClass* GBufferShader_;
        ShaderClass* DeferredShader_;
        
};


} // /namespace video

} // /namespace sp


#endif



// ================================================================================
