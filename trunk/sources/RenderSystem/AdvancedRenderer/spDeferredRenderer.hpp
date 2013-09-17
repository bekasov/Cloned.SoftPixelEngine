/*
 * Deferred renderer header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_DEFERRED_RENDERER_H__
#define __SP_DEFERRED_RENDERER_H__


#include "Base/spStandard.hpp"

#if defined(SP_COMPILE_WITH_ADVANCEDRENDERER)


#include "RenderSystem/AdvancedRenderer/spAdvancedRenderer.hpp"
#include "RenderSystem/AdvancedRenderer/spGBuffer.hpp"


namespace sp
{
namespace video
{


/**
Integrated deferred-renderer which supports normal- and parallax-occlision mapping.
\since Version 3.2
*/
class SP_EXPORT DeferredRenderer : public AdvancedRenderer
{
    
    public:
        
        DeferredRenderer();
        virtual ~DeferredRenderer();
        
        /* === Functions === */
        
        virtual io::stringc getDescription() const;
        
        virtual void releaseResources();
        
        virtual void renderScene(scene::SceneGraph* Graph, scene::Camera* ActiveCamera = 0);
        
        virtual void setGIReflectivity(f32 Reflectivity);
        virtual void setAmbientColor(const dim::vector3df &ColorVec);
        virtual void setResolution(const dim::size2di &Resolution);
        
        /* === Inline functions === */
        
        //! Returns a constant pointer to the g-buffer. This will never return a null pointer.
        inline const GBuffer* getGBuffer() const
        {
            return &GBuffer_;
        }
        //! Returns a pointer to the g-buffer. This will never return a null pointer.
        inline GBuffer* getGBuffer()
        {
            return &GBuffer_;
        }
        
        //! Returns the g-buffer shader class. This shader is used to render the scene into the g-buffer.
        inline ShaderClass* getGBufferShader() const
        {
            return GBufferShader_;
        }
        //! Returns the deferred shader class. This shader is used to render the g-buffer into the pixel buffer with deferred lighting.
        inline ShaderClass* getDeferredShader() const
        {
            return DeferredShader_;
        }
        
        //! Enables or disables the usage of the default g-buffer shader. By default true.
        inline void setDefaultGBufferShader(bool Enable)
        {
            UseDefaultGBufferShader_ = Enable;
        }
        //! Returns true if the usage of the default g-buffer shader is enabled.
        inline bool getDefaultGBufferShader() const
        {
            return UseDefaultGBufferShader_;
        }
        
    protected:
        
        /* === Functions === */
        
        virtual void updateLightSources(scene::SceneGraph* Graph, scene::Camera* ActiveCamera);
        //! \warning Does not check for null pointers!
        virtual void renderSceneIntoGBuffer(scene::SceneGraph* Graph, scene::Camera* ActiveCamera);
        virtual void renderDeferredShading(Texture* RenderTarget);
        virtual void renderLowResVPLShading();
        
        virtual bool loadAllShaders();
        virtual void deleteAllShaders();
        
        virtual void createVertexFormats();
        virtual bool setupFinalResources();
        
        bool loadGBufferShader();
        bool loadDeferredShader();
        bool loadLowResVPLShader();
        
        void setupDeferredSampler(Shader* ShaderObj, bool IsLowResVPL = false);
        
        /* === Members === */
        
        GBuffer GBuffer_;
        
        ShaderClass* GBufferShader_;                //!< G-Buffer rendering shader class.
        ShaderClass* DeferredShader_;               //!< Deferred lighting shader class.
        ShaderClass* LowResVPLShader_;              //!< Low-resolution VPL shader class.
        
        VertexFormatUniversal ImageVertexFormat_;   //!< 2D image vertex format.
        
        bool UseDefaultGBufferShader_;
        
};


} // /namespace video

} // /namespace sp


#endif

#endif



// ================================================================================
