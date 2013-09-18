/*
 * Advanced renderer header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_ADVANCED_RENDERER_H__
#define __SP_ADVANCED_RENDERER_H__


#include "Base/spStandard.hpp"

#if defined(SP_COMPILE_WITH_ADVANCEDRENDERER)


#include "RenderSystem/AdvancedRenderer/spAdvancedRendererFlags.hpp"
#include "RenderSystem/AdvancedRenderer/spShadowMapper.hpp"
#include "RenderSystem/AdvancedRenderer/spLightGrid.hpp"
#include "RenderSystem/AdvancedRenderer/spVPLGenerator.hpp"
#include "RenderSystem/PostProcessing/spBloomEffect.hpp"
#include "Base/spVertexFormatUniversal.hpp"
#include "Base/spDimensionUniversalBuffer.hpp"


namespace sp
{

namespace scene
{
    class SceneGraph;
    class Camera;
}

namespace video
{


class ShaderClass;
class ShaderResource;


//!!!
#if ( defined(SP_DEBUGMODE) || 1 ) && 0
#   define _DEB_LOAD_SHADERS_FROM_FILES_
#   ifdef _DEB_LOAD_SHADERS_FROM_FILES_
#       define _DEB_SHADER_PATH_ io::stringc("../../sources/RenderSystem/AdvancedRenderer/")
#   endif
#endif


//!  Advanced renderer types.
enum EAdvancedRenderers
{
    ADVANCEDRENDERER_DEFERRED,  //!< Deferred renderer. \see DeferredRenderer
    ADVANCEDRENDERER_FORWARD,   //!< Forward renderer. \see ForwardRenderer
    ADVANCEDRENDERER_CUSTOM,    //!< Custom advanced renderer. Use this for your own advanced renderer.
};


/**
Advanced renderer which supports normal mapping and further visual effects.
\since Version 3.3
*/
class SP_EXPORT AdvancedRenderer
{
    
    public:
        
        virtual ~AdvancedRenderer();
        
        /* === Functions === */
        
        //! Returns a description or name of this advanced renderer.
        virtual io::stringc getDescription() const = 0;
        
        /**
        Generates the deferred rendering shaders and builds the g-buffer.
        \param[in] Flags Specifies the shader generation flags.
        \param[in] ShadowTexSize Specifies the texture size for shadow maps. By default 256.
        \param[in] MaxPointLightCount Specifies the maximal count of point lights used for shadow maps. By default 8.
        \param[in] MaxSpotLightCount Specifies the maximal count of spot lights used for shadow maps. By default 8.
        \param[in] MultiSampling Specifies the count of multi-samples. By default 0.
        \return True on success otherwise false.
        \note The last three parameters have no effect if shadow mapping is disabled (RENDERERFLAG_SHADOW_MAPPING).
        \see EAdvancedRenderFlags
        */
        virtual bool generateResources(
            u32 Flags, s32 ShadowTexSize, u32 MaxPointLightCount, u32 MaxSpotLightCount, s32 MultiSampling = 0
        );
        
        /**
        Releases (or rather deletes) all previously generated resources.
        This will be called automatically before new resources will be generated or the deferred renderer will be deleted.
        \see generateResources
        \since Version 3.3
        */
        virtual void releaseResources();
        
        /**
        Renders the whole given scene with deferred shading onto the
        screen or into the render target if specified.
        \param[in] Graph Specifies the scene graph which is to be rendered.
        \param[in] ActiveCamera Specifies the active camera for which the scene is to be rendered.
        This can also be 0 to render the scene for all cameras with their individual viewports.
        \see Texture::setRenderTarget
        \see setGBufferShader
        \see setDeferredShader
        */
        virtual void renderScene(scene::SceneGraph* Graph, scene::Camera* ActiveCamera = 0) = 0;
        
        //! Sets the global-illumination (GI) reflectivity. By default 0.1f.
        virtual void setGIReflectivity(f32 Reflectivity);
        
        /**
        Sets the ambient color for the deferred shading. This is a 3-component vector whose values
        are used to be in the range [0.0 .. 1.0]. The default value is (0.1, 0.1, 0.1).
        */
        virtual void setAmbientColor(const dim::vector3df &ColorVec);
        
        /**
        Adjusts all buffers and internal configurations for the new resolution. Call this when you changed the screen resolution.
        \see setResolution
        \since Version 3.3
        */
        void adjustResolution();
        
        /**
        Sets the new resolution (or rather resizes the current resolution).
        \see adjustResolution
        \since Version 3.3
        */
        virtual void setResolution(const dim::size2di &Resolution);
        
        /* === Inline functions === */
        
        //! Returns the type of this advanced renderer.
        inline EAdvancedRenderers getType() const
        {
            return Type_;
        }
        
        //! Returns the resolution set after creating the GBuffer textures.
        inline const dim::size2di& getResolution() const
        {
            return Resolution_;
        }
        
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
        
        //! Returns a constant pointer to the shadow mapper. This will never return a null pointer.
        inline const ShadowMapper* getShadowMapper() const
        {
            return &ShadowMapper_;
        }
        //! Returns a pointer to the shadow mapper. This will never return a null pointer.
        inline ShadowMapper* getShadowMapper()
        {
            return &ShadowMapper_;
        }
        
        //! Returns a constant pointer to the bloom effect. This will never return a null pointer.
        inline const BloomEffect* getBloomEffect() const
        {
            return &BloomEffect_;
        }
        //! Returns a pointer to the bloom effect. This will never return a null pointer.
        inline BloomEffect* getBloomEffect()
        {
            return &BloomEffect_;
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
        
        //! Returns the ambient color for the deferred shading. By default (0.1, 0.1, 0.1).
        inline const dim::vector3df& getAmbientColor() const
        {
            return ShadingDesc_.AmbientColor;
        }

        /**
        Enables or disables virtual-point-light (VPL) debugging. By default enabled.
        This requires that the deferred-renderer resources have been generated with the
        debug VPL flag (RENDERERFLAG_DEBUG_VIRTUALPOINTLIGHTS).
        \see EAdvancedRenderFlags
        */
        inline void setDebugVPL(bool Enable)
        {
            DebugVPL_.Enabled = Enable;
        }
        /**
        Returns ture if virtual-point-light (VPL) debugging is enabled.
        \see setDebugVPL
        */
        inline bool getDebugVPL() const
        {
            return DebugVPL_.Enabled;
        }
        
        //! Returns the global-illumination (GI) reflectivity. By default 0.1.
        inline f32 getGIReflectivity() const
        {
            return ShadingDesc_.GIReflectivity;
        }
        
    protected:
        
        /* === Macros === */
        
        static const u32 VPL_COUNT;
        
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
        
        struct SP_EXPORT SLightCB
        {
            /* Members */
            dim::vector3df Position;
            f32 InvRadius;
            dim::vector3df Color;
            f32 Pad0;
            s32 Type;
            s32 ShadowIndex;
            s32 UsedForLightmaps;
            s32 ExID;
        }
        SP_PACK_STRUCT;
        
        struct SP_EXPORT SLightExCB
        {
            /* Members */
            dim::matrix4f ViewProjection;
            dim::matrix4f InvViewProjection;
            dim::vector3df Direction;
            f32 Pad0;
            f32 SpotTheta;
            f32 SpotPhiMinusTheta;
            f32 Pad1[2];
        }
        SP_PACK_STRUCT;
        
        struct SP_EXPORT SShadingDescCB
        {
            SShadingDescCB();
            ~SShadingDescCB();

            /* Members */
            dim::vector3df AmbientColor;
            f32 GIReflectivity;
            s32 LightCount;
        }
        SP_PACK_STRUCT;

        #ifdef _MSC_VER
        #   pragma pack(pop, packing)
        #endif
        
        #undef SP_PACK_STRUCT
        
        struct SP_EXPORT SLightDesc
        {
            SShaderConstant LightCountConstant;
        };
        
        struct SP_EXPORT SDebugVPL
        {
            SDebugVPL();
            ~SDebugVPL();
            
            /* Functions */
            void load();
            void unload();
            
            /* Members */
            ShaderClass* ShdClass;
            VertexFormatUniversal* VtxFormat;
            MeshBuffer Model;
            MaterialStates Material;
            bool Enabled;
        };
        
        struct SP_EXPORT SRendererConfig
        {
            SRendererConfig();
            ~SRendererConfig();
            
            /* Functions */
            void setupFlags(s32 NewFlags);
            
            /* Members */
            s32 Flags;
            s32 ShadowTexSize;
            u32 MaxNumPointLights;
            u32 MaxNumSpotLights;
            s32 MultiSampling;
        };
        
        /* === Functions === */
        
        AdvancedRenderer(const EAdvancedRenderers Type);
        
        void initResourceConfig(
            u32 Flags, s32 ShadowTexSize, u32 MaxPointLightCount, u32 MaxSpotLightCount, s32 MultiSampling
        );
        
        //! \warning Does not check for null pointers!
        void updateLightSources(
            scene::SceneGraph* Graph, scene::Camera* ActiveCamera,
            Texture* DepthTexture, s32 &i, s32 &iEx
        );
        
        void renderDebugVPLs(scene::Camera* ActiveCamera);
        
        bool buildShader(
            const io::stringc &Name,
            
            ShaderClass* &ShdClass,
            const VertexFormat* VertFmt,
            
            const std::list<io::stringc>* ShdBufferVertex,
            const std::list<io::stringc>* ShdBufferPixel,
            
            const io::stringc &VertexMain = "VertexMain",
            const io::stringc &PixelMain = "PixelMain",
            
            s32 Flags = SHADERBUILD_CG
        );
        
        void deleteShader(ShaderClass* &ShdClass);
        
        virtual bool loadAllShaders();
        virtual void deleteAllShaders();
        
        virtual void createVertexFormats();
        virtual bool setupFinalResources();
        
        void setupGeometryCompilerOptions   (std::list<io::stringc> &CompilerOp, bool UseGuard = false);
        void setupShadingCompilerOptions    (std::list<io::stringc> &CompilerOp, bool UseGuard = false);
        
        void setupGeometrySampler(Shader* ShaderObj, s32 &SamplerIndex);
        
        void setupLightShaderConstants(Shader* FragShd);
        
        void setupVPLOffsets(
            Shader* ShaderObj, const io::stringc &BufferName, u32 OffsetCount,
            s32 Rings = 5, s32 Rotations = 5, f32 Bias = 1.5f, f32 JitterBias = 0.05f
        );
        
        void printInfo();
        void pushBackInfo(io::stringc &FlagsStr, u32 Flag, const io::stringc &Desc);
        
        bool getActiveCamera(scene::SceneGraph* Graph, scene::Camera* &ActiveCamera) const;
        
        /* === Members === */
        
        ERenderSystems RenderSys_;
        dim::size2di Resolution_;
        
        SRendererConfig Config_;
        
        ShadowMapper ShadowMapper_;
        BloomEffect BloomEffect_;
        LightGrid LightGrid_;
        VPLGenerator VPLGenerator_;
        
        ShaderClass* ShadowShader_;                 //!< Shadow map rendering shader class.
        
        ConstantBuffer* ConstBufferLights_;         //!< Light list constant buffer.
        ConstantBuffer* ConstBufferLightsEx_;       //!< Extended light list constant buffer.
        
        STextureLayerModel LayerModel_;
        
        SLightDesc LightDesc_;
        SShadingDescCB ShadingDesc_;
        
        dim::UniversalBuffer Lights_;
        dim::UniversalBuffer LightsEx_;
        
        std::vector<dim::vector4df> PointLightsPositionAndRadius_;
        
        SDebugVPL DebugVPL_;                        //!< Debug virtual-point-light data.
        
    private:
        
        /* === Functions === */
        
        bool loadShadowShader();
        bool loadDebugVPLShader();
        
        void setupShadowCompilerOptions(std::list<io::stringc> &CompilerOp, bool UseGuard = false);
        
        void setupDebugVPLSampler(Shader* ShaderObj);
        
        /* === Members === */
        
        EAdvancedRenderers Type_;
        
        VertexFormatUniversal VertexFormat_;        //!< Object vertex format.
        
};


} // /namespace video

} // /namespace sp


#endif

#endif



// ================================================================================
