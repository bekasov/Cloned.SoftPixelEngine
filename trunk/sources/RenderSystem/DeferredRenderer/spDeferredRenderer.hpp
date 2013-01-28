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
    /**
    Makes use of the texture transformation matrix. This will be used for the
    diffuse-, specular-, normal- and height map.
    */
    DEFERREDFLAG_USE_TEXTURE_MATRIX         = 0x0001,
    /**
    Enables individual specular map usage. If this option is enabled every model
    must have an additional texture (layer 1) with specular information.
    */
    DEFERREDFLAG_HAS_SPECULAR_MAP           = 0x0002,
    /**
    Enables individual light map usage. If this option is enabled every model must have
    an addition texture (layer 1 if there is no specular map, otherwise 2) with illumination data.
    Those lightmaps can be generated with the "SoftPixel Sandbox" or rather the lightmap-generator from this engine.
    */
    DEFERREDFLAG_HAS_LIGHT_MAP              = 0x0004,
    //! Allows the lighting to over-blend. By default disabled.
    DEFERREDFLAG_ALLOW_OVERBLENDING         = 0x0008,
    
    /**
    Enables normal-mapping. If this option is enabled every model must have an additional
    texture (layer 1 if there is no specular map and no light map, otherwise 2 or 3) with normal vector information.
    */
    DEFERREDFLAG_NORMAL_MAPPING             = 0x0010,
    /**
    Enables parallax-occlusion mapping. If this option is enabled every model must have an additional
    texture (layer 2 if there is no specular map and no light map, otherwise 3 or 4) with height map information.
    This can be a gray-scaled texture. If the DEFERREDFLAG_NORMALMAP_XYZ_H option is enabled,
    no height map is needed. In that case the height map information is get from the normal map's alpha channel.
    This requires normal-mapping (DEFERREDFLAG_NORMAL_MAPPING).
    */
    DEFERREDFLAG_PARALLAX_MAPPING           = 0x0020,
    /**
    Enables the normal map to also contain the height map data in the alpha channel. This this option is enabled
    no height map texture is used. This requires parallax-mapping (DEFERREDFLAG_PARALLAX_MAPPING).
    */
    DEFERREDFLAG_NORMALMAP_XYZ_H            = 0x0040,
    /**
    Enables shadow mapping. For this technique "variance shadow mapping" (VSM)
    is used for performan ce reasons.
    */
    DEFERREDFLAG_SHADOW_MAPPING             = 0x0080, //!< Enables shadow mapping.
    //! Enables the bloom effect. All glossy surfaces glow intensely.
    DEFERREDFLAG_BLOOM                      = 0x0100,
    
    #if 0
    /**
    Enables height-field tessellation. This can not be used together
    with parallax-mapping (DEFERREDFLAG_PARALLAX_MAPPING).
    */
    DEFERREDFLAG_TESSELLATION               = 0x0100,
    #endif
    
    /**
    This option can be used for debugging purposes. It renders the final image as four viewports
    containing the color buffer output, normal buffer output, depth buffer output and the final image.
    */
    DEFERREDFLAG_DEBUG_GBUFFER              = 0x1000,
    /**
    This option can be used for debugging purposes. It renders the
    world-position for each pixel instead of its distance to the view camera.
    This requires gbuffer debugging (DEFERREDFLAG_DEBUG_GBUFFER).
    */
    DEFERREDFLAG_DEBUG_GBUFFER_WORLDPOS     = 0x2000,
    /**
    This option can be used for debugging purposes. It renders the
    texture coordinates as colors for each pixel instead of the diffuse colors.
    This requires gbuffer debugging (DEFERREDFLAG_DEBUG_GBUFFER).
    */
    DEFERREDFLAG_DEBUG_GBUFFER_TEXCOORDS    = 0x4000,
};


class ShaderClass;


/**
Texture layer model for deferred-renderer. Use this structure to determine how the texture layers are constructed.
If a texture layer has the value of TEXTURE_IGNORE this layer is not used. By default every layer has the value of TEXTURE_IGNORE.
*/
struct STextureLayerModel
{
    STextureLayerModel() :
        DiffuseMap  (TEXTURE_IGNORE),
        SpecularMap (TEXTURE_IGNORE),
        LightMap    (TEXTURE_IGNORE),
        NormalMap   (TEXTURE_IGNORE),
        HeightMap   (TEXTURE_IGNORE)
    {
    }
    ~STextureLayerModel()
    {
    }
    
    /* Functions */
    void clear()
    {
        DiffuseMap  = TEXTURE_IGNORE;
        SpecularMap = TEXTURE_IGNORE;
        LightMap    = TEXTURE_IGNORE;
        NormalMap   = TEXTURE_IGNORE;
        HeightMap   = TEXTURE_IGNORE;
    }
    
    /* Members */
    u8 DiffuseMap;  //!< Diffuse map layer. Commonly 0.
    u8 SpecularMap; //!< Diffuse map layer. Commonly 1 or TEXTURE_IGNORE.
    u8 LightMap;    //!< Diffuse map layer. Commonly 1, 2 or TEXTURE_IGNORE.
    u8 NormalMap;   //!< Diffuse map layer. Commonly 1, 2, 3 or TEXTURE_IGNORE.
    /**
    Diffuse map layer. Commonly 1, 2, 3, 4 or TEXTURE_IGNORE.
    If the height-map information is stored in the alpha-channel
    of the normal-maps, this layer has the same value as the "NormalMap" layer.
    */
    u8 HeightMap;
};


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
        
        /**
        Returns the texture layer model. Use this to determine how the texture layers are constructed.
        These texture layers are used for the g-buffer.
        \see STextureLayerModel
        */
        inline const STextureLayerModel& getTextureLayerModel() const
        {
            return LayerModel_;
        }
        
        /**
        Sets the ambient color for the deferred shading. This is a 3-component vector whose values
        are used to be in the range [0.0 .. 1.0]. The default value is (0.1, 0.1, 0.1).
        */
        inline void setAmbientColor(const dim::vector3df &ColorVec)
        {
            AmbientColor_ = ColorVec;
        }
        //! Returns the ambient color for the deferred shading. By default (0.1, 0.1, 0.1).
        inline dim::vector3df getAmbientColor() const
        {
            return AmbientColor_;
        }
        
    protected:
        
        /* === Macros === */
        
        static const s32 MAX_LIGHTS     = 35;
        static const s32 MAX_EX_LIGHTS  = 15;
        
        /* === Enumerations === */
        
        enum EBuildShaderFlags
        {
            BUILD_TESSELLATION  = 0x0001,
            
            BUILD_CG            = 0x0002,
            BUILD_GLSL          = 0x0004,
            BUILD_HLSL3         = 0x0008,
            BUILD_HLSL5         = 0x0010,
            
            BUILD_VERTEX        = 0x0020,
            BUILD_PIXEL         = 0x0040,
            BUILD_GEOMETRY      = 0x0080,
            BUILD_HULL          = 0x0100,
            BUILD_DOMAIN        = 0x0200,
        };
        
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
        
        struct SP_EXPORT SLight
        {
            SLight();
            ~SLight();
            
            /* Members */
            dim::vector3df Position;
            f32 Radius;
            dim::vector3df Color;
            s32 Type;
            s32 ShadowIndex;
            s32 UsedForLightmaps;
        }
        SP_PACK_STRUCT;
        
        struct SP_EXPORT SLightEx
        {
            SLightEx();
            ~SLightEx();
            
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
        
        struct SP_EXPORT SBloomFilter
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
        
        EShaderVersions getShaderVersionFromFlags(s32 Flags) const;
        
        bool buildShader(
            const io::stringc &Name,
            
            ShaderClass* &ShdClass,
            VertexFormat* VertFmt,
            
            const std::list<io::stringc>* ShdBufferVert,
            const std::list<io::stringc>* ShdBufferFrag,
            
            const io::stringc &VertexMain = "VertexMain",
            const io::stringc &PixelMain = "PixelMain",
            
            s32 Flags = BUILD_CG
        );
        
        void deleteShaders();
        void createVertexFormats();
        
        void drawFullscreenImage(Texture* Tex);
        void drawFullscreenImageStreched(Texture* Tex);
        
        void setupCompilerOptions(
            std::list<io::stringc> &GBufferCompilerOp, std::list<io::stringc> &DeferredCompilerOp
        );
        
        void setupGBufferSampler(video::Shader* PixelShader);
        void setupDeferredSampler(video::Shader* PixelShader);
        
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
        STextureLayerModel LayerModel_;
        
        std::vector<SLight> Lights_;
        std::vector<SLightEx> LightsEx_;
        
        SBloomFilter BloomFilter_;
        
        dim::vector3df AmbientColor_;
        
};


} // /namespace video

} // /namespace sp


#endif

#endif



// ================================================================================
