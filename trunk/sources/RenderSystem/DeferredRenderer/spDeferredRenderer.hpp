/*
 * Deferred renderer header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_DEFERRED_RENDERER_H__
#define __SP_DEFERRED_RENDERER_H__


#include "Base/spStandard.hpp"

#if defined(SP_COMPILE_WITH_DEFERREDRENDERER)


#include "RenderSystem/DeferredRenderer/spGBuffer.hpp"
#include "Base/spVertexFormatUniversal.hpp"


namespace sp
{

namespace scene
{
    class SceneGraph;
    class Camera;
}

namespace video
{


//! Deferred rendering flags.
enum EDeferredRenderFlags
{
    DEFERREDFLAG_NORMAL_MAPPING     = 0x0001,                               //!< Enables normal mapping.
    DEFERREDFLAG_PARALLAX_MAPPING   = 0x0002 | DEFERREDFLAG_NORMAL_MAPPING, //!< Enables parallax-occlusion mapping. This includes normal mapping.
};


class ShaderClass;

/**
Integrated deferred-renderer which supports normal- and parallax-occlision mapping.
\note The integrated deferred-renderer requires that the engine was compiled with the Cg-Toolkit.
\since Version 3.2
*/
class SP_EXPORT DeferredRenderer
{
    
    public:
        
        DeferredRenderer();
        virtual ~DeferredRenderer();
        
        /* === Functions === */
        
        /**
        Generates the deferred rendering shaders.
        \param Flags: Specifies the shader generation flags.
        \return True on success otherwise false.
        \see EDeferredRenderFlags
        */
        virtual bool generateShaders(s32 Flags = 0);
        
        /**
        Renders the whole given scene with deferred shading onto the
        screen or into the render target if specified.
        \param Graph: Specifies the scene graph which is to be rendered.
        \param ActiveCamera: Specifies the active camera for which the scene is to be rendered.
        This can also be 0 to render the scene for all cameras with their individual viewports.
        \param RenderTarget: Specifies a render target texture where the whole scene will
        be rendered in. When this parameter is 0 the result will be rendered directly onto the screen.
        This texture must be a valid render target otherwise nothing will be rendered.
        \param UseDefaultGBufferShader: Specifies whether the default g-buffer shader is to be used or not.
        If false each rendered object must have a valid shader class which renders into the g-buffer textures.
        \note First of all a valid g-buffer- and deferred shader must be set.
        \see Texture::setRenderTarget
        \see setGBufferShader
        \see setDeferredShader
        */
        virtual void renderScene(
            scene::SceneGraph* Graph, scene::Camera* ActiveCamera = 0,
            video::Texture* RenderTarget = 0, bool UseDefaultGBufferShader = true
        );
        
        /* === Inline functions === */
        
        //! Returns the g-buffer shader class. This shader is used to render the scene into the g-buffer.
        inline const ShaderClass* getGBufferShader() const
        {
            return GBufferShader_;
        }
        //! Returns the deferred shader class. This shader is used to render the g-buffer into the pixel buffer with deferred lighting.
        inline const ShaderClass* getDeferredShader() const
        {
            return DeferredShader_;
        }
        
        //! Returns the vertex format which must be used for the objects which should be rendered with this deferred renderer.
        inline const VertexFormatUniversal* getVertexFormat() const
        {
            return &VertexFormat_;
        }
        
    protected:
        
        /* === Functions === */
        
        //! \warning Does not check for null pointers!
        virtual void renderSceneIntoGBuffer(
            scene::SceneGraph* Graph, scene::Camera* ActiveCamera, bool UseDefaultGBufferShader
        );
        virtual void renderDeferredShading(video::Texture* RenderTarget);
        
        void deleteShaders();
        void createVertexFormats();
        
        /* === Members === */
        
        GBuffer GBuffer_;
        
        ShaderClass* GBufferShader_;
        ShaderClass* DeferredShader_;
        
        VertexFormatUniversal VertexFormat_;        //!< Object vertex format.
        VertexFormatUniversal ImageVertexFormat_;   //!< 2D image vertex format.
        
        s32 Flags_;
        
};


} // /namespace video

} // /namespace sp


#endif

#endif



// ================================================================================
