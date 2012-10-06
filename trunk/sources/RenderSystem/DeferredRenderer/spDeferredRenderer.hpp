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
#include "RenderSystem/DeferredRenderer/spShadowMapper.hpp"
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
    DEFERREDFLAG_USE_TEXTURE_MATRIX = 0x0001,
    DEFERREDFLAG_HAS_SPECULAR_MAP   = 0x0002,
    DEFERREDFLAG_NORMAL_MAPPING     = 0x0004, //!< Enables normal mapping.
    DEFERREDFLAG_PARALLAX_MAPPING   = 0x0008, //!< Enables parallax-occlusion mapping. This requires normal mapping (DEFERREDFLAG_NORMAL_MAPPING).
    DEFERREDFLAG_SHADOW_MAPPING     = 0x0010, //!< Enables shadow mapping.
    DEFERREDFLAG_TESSELLATION       = 0x0020, //!< Enables height-field tessellation. This can not be used together with parallax-mapping (DEFERREDFLAG_PARALLAX_MAPPING).
    DEFERREDFLAG_BLOOM              = 0x0040, //!< Enables bloom effect.
    
    DEFERREDFLAG_DEBUG_GBUFFER      = 0x8000, //!< Renders g-buffer for debugging.
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
        Generates the deferred rendering shaders and builds the g-buffer.
        \param Flags: Specifies the shader generation flags.
        \param ShadowTexSize: Specifies the texture size for shadow maps. By default 256.
        \param MaxPointLightCount: Specifies the maximal count of point lights used for shadow maps. By default 8.
        \param MaxSpotLightCount: Specifies the maximal count of spot lights used for shadow maps. By default 8.
        \return True on success otherwise false.
        \note The last three parameters have no effect if shadow mapping is disabled (DEFERREDFLAG_SHADOW_MAPPING).
        \see EDeferredRenderFlags
        */
        virtual bool generateResources(
            s32 Flags, s32 ShadowTexSize, u32 MaxPointLightCount, u32 MaxSpotLightCount
        );
        
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
            Texture* RenderTarget = 0, bool UseDefaultGBufferShader = true
        );
        
        /**
        Changes the gaussian multiplier if a bloom filter has already been generated.
        \param GaussianMultiplier: Specifies the gaussian multiplier for the bloom filter. By default 0.6.
        \note This has no effect until "generateResources" was called with the "DEFERREDFLAG_BLOOM" bit-field.
        \see generateResources
        \see EDeferredRenderFlags
        */
        virtual void changeBloomFactor(f32 GaussianMultiplier);
        
        /* === Inline functions === */
        
        /**
        Generates the deferred rendering shaders and builds the g-buffer.
        This is just an overloaded version of the function with less parameters.
        The default settings are:
        \code
        generateResources(Flags, 256, 8, 8);
        \endcode
        */
        inline bool generateResources(s32 Flags = 0)
        {
            return generateResources(Flags, 256, 8, 8);
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
        //! Returns the bloom filter shader for the horizontal-render-pass. This shader is used to render a bloom filter as an optional third render pass.
        inline ShaderClass* getBloomShaderHRP() const
        {
            return BloomShaderHRP_;
        }
        //! Returns the bloom filter shader for the vertical-render-pass. This shader is used to render a bloom filter as an optional third render pass.
        inline ShaderClass* getBloomShaderVRP() const
        {
            return BloomShaderVRP_;
        }
        
        //! Returns the vertex format which must be used for the objects which should be rendered with this deferred renderer.
        inline const VertexFormatUniversal* getVertexFormat() const
        {
            return &VertexFormat_;
        }
        
    protected:
        
        /* === Macros === */
        
        static const s32 MAX_LIGHTS     = 35;
        static const s32 MAX_EX_LIGHTS  = 15;
        
        /* === Structures === */
        
        #if defined(_MSC_VER)
        #   pragma pack(push, packing)
        #   pragma pack(1)
        #   define SP_PACK_STRUCT
        #elif defined(__GNUC__)
        #   define SP_PACK_STRUCT __attribute__((packed))
        #else
        #   define SP_PACK_STRUCT
        #endif
        
        struct SLight
        {
            SLight() :
                Radius      (1000.0f),
                Color       (1.0f   ),
                Type        (0      ),
                ShadowIndex (-1     )
            {
            }
            ~SLight()
            {
            }
            
            /* Members */
            dim::vector3df Position;
            f32 Radius;
            dim::vector3df Color;
            s32 Type;
            s32 ShadowIndex;
        }
        SP_PACK_STRUCT;
        
        struct SLightEx
        {
            SLightEx() :
                Direction           (0.0f, 0.0f, 1.0f   ),
                SpotTheta           (0.0f               ),
                SpotPhiMinusTheta   (0.0f               )
            {
            }
            ~SLightEx()
            {
            }
            
            /* Members */
            dim::matrix4f Projection;
            dim::vector3df Direction;
            f32 SpotTheta;
            f32 SpotPhiMinusTheta;
        }
        SP_PACK_STRUCT;
        
        #ifdef _MSC_VER
        #   pragma pack(pop, packing)
        #endif
        
        #undef SP_PACK_STRUCT
        
        struct SBloomFilter
        {
            SBloomFilter();
            ~SBloomFilter();
            
            /* Functions */
            void computeWeights(f32 GaussianMultiplier = 0.6f);
            void computeOffsets(const dim::size2di &Resolution);
            
            /* Macros */
            static const s32 FILTER_SIZE = 9;
            
            /* Members */
            f32 BlurOffsets[FILTER_SIZE*2], BlurWeights[FILTER_SIZE];
        };
        
        /* === Functions === */
        
        //! \warning Does not check for null pointers!
        virtual void updateLightSources(scene::SceneGraph* Graph, scene::Camera* ActiveCamera);
        //! \warning Does not check for null pointers!
        virtual void renderSceneIntoGBuffer(
            scene::SceneGraph* Graph, scene::Camera* ActiveCamera, bool UseDefaultGBufferShader
        );
        virtual void renderDeferredShading(Texture* RenderTarget);
        virtual void renderBloomFilter(Texture* RenderTarget);
        
        bool buildShader(
            const io::stringc &Name, ShaderClass* &ShdClass, VertexFormat* VertFmt,
            const std::vector<io::stringc> &ShdBuffer, const c8** CompilerOptions = 0,
            const io::stringc &VertexMain = "VertexMain", const io::stringc &PixelMain = "PixelMain",
            bool HasTessellation = false
        );
        
        void deleteShaders();
        void createVertexFormats();
        
        void drawFullscreenImage(Texture* Tex);
        void drawFullscreenImageStreched(Texture* Tex);
        
        /* === Members === */
        
        GBuffer GBuffer_;
        ShadowMapper ShadowMapper_;
        
        ShaderClass* GBufferShader_;
        ShaderClass* DeferredShader_;
        ShaderClass* BloomShaderHRP_;               //!< Bloom shader class for the horizontal render pass.
        ShaderClass* BloomShaderVRP_;               //!< Bloom shader class for the vertical render pass.
        ShaderClass* ShadowShader_;
        
        VertexFormatUniversal VertexFormat_;        //!< Object vertex format.
        VertexFormatUniversal ImageVertexFormat_;   //!< 2D image vertex format.
        
        s32 Flags_;
        
        std::vector<SLight> Lights_;
        std::vector<SLightEx> LightsEx_;
        
        SBloomFilter BloomFilter_;
        
};


} // /namespace video

} // /namespace sp


#endif

#endif



// ================================================================================
