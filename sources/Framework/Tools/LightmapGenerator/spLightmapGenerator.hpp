/*
 * Lightmap generator header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_LIGHTMAP_GENERATOR_H__
#define __SP_LIGHTMAP_GENERATOR_H__


#include "Base/spStandard.hpp"

#ifdef SP_COMPILE_WITH_LIGHTMAPGENERATOR


#include "Base/spInputOutputString.hpp"
#include "Base/spDimension.hpp"
#include "Base/spThreadManager.hpp"
#include "SceneGraph/spSceneGraph.hpp"
#include "SceneGraph/Collision/spCollisionConfigTypes.hpp"
#include "SceneGraph/Collision/spCollisionGraph.hpp"
#include "RenderSystem/spRenderSystem.hpp"
#include "Framework/Tools/LightmapGenerator/spLightmapBase.hpp"
#include "Framework/Tools/LightmapGenerator/spLightmapShaderDispatcher.hpp"

#include <list>
#include <vector>
#include <map>


namespace sp
{
namespace tool
{


//! The lightmap generator class is a utility actually only used in a world editor.
class SP_EXPORT LightmapGenerator
{
    
    public:
        
        LightmapGenerator();
        ~LightmapGenerator();
        
        /* === Functions === */
        
        /**
        Generates the lightmaps for each get-shadow-object.
        This is a very time-consuming procedure which has been created for a level editor.
        Simple-shadows are supported and since version 3.3 radiosity is supported as well (but only with GPU accelleration).
        \param[in] CastShadowObjects List of all 3D models which cast shadows.
        \param[in] GetShadowObjects List of all 3D models which get shadows. Only these objects build the resulting model.
        \param[in] LightSources List of all light sources which are to be used in the lightmap generation process.
        \param[in] Config Hold the common configurations for the process: ambient color, lightmap size etc.
        Since version 3.3 these settings are capsuled into a seperated structure.
        \param[in] ThreadCount Specifies the count of threads which are to be used when the lightmap texels
        will be computed for every light source. This has been added with version 3.2.
        This value must be greater than 1 to has any effect. By default 0.
        \param[in] Flags Specifies additional options for the generation process. For more information
        see the ELightmapGenerationsFlags enumeration.
        \return True if the lightmap generation has been completed successful. Otherwise it has been canceled.
        \note Since version 3.2 this function does no longer return a pointer to the final model which forms the whole
        lightmap-generated scene. From now you need to get this pointer by the "getFinalModel" function.
        More over multi-threading has been addded since version 3.2.
        \see getFinalModel
        \see ELightmapGenerationsFlags
        \see SLightmapGenConfig
        */
        bool generateLightmaps(
            const std::vector<SCastShadowObject> &CastShadowObjects,
            const std::vector<SGetShadowObject> &GetShadowObjects,
            const std::vector<SLightmapLight> &LightSources,
            const SLightmapGenConfig &Config,
            const u8 ThreadCount = 0,
            const s32 Flags = 0
        );
        
        /**
        Clears the internal objects and states.
        This will be called automatically when "generateLightmaps" is called.
        */
        void clearScene();
        
        /**
        Updates the texel bluring. This has no effect if the given radius is equal to
        the last set radius or the lightmaps has not yet generated.
        \return True if the bluring has been updated. Otherwise there is nothing to do.
        */
        bool updateBluring(u8 TexelBlurRadius);
        
        /**
        Updates the lightmap ambient color. This has no effect if the given color
        is equal to the last set ambient color or the lightmaps has not yet generated.
        \return True if the ambient color has been updated. Otherwise there is nothing to do.
        */
        bool updateAmbientColor(const video::color &AmbientColor);
        
        /**
        Sets the callback function. This function is called several times in during the
        lightmap generation process. With this you can control the progress and maybe cancel.
        Try to process not so much in this function because the time costs will grow!
        \param Callback: Callback function in the form of "bool Callback(f32 Progress)"
        where Progress is between 0.0 and 1.0.
        \see LightmapProgressCallback
        */
        static void setProgressCallback(const LightmapProgressCallback &Callback);
        
        /* === Inline functions === */
        
        /**
        Returns a pointer to the final lightmap-generated scene model. This is a static object, so it should not be used
        for animations. Dynamic objects - when "StayAlone" in the "SGetShadowObject" structure is true - has this resulting
        object as parent. 'StayAlone' objects are objects which also got a lightmap but which shall be further
        movable (e.g. doors or other movable objects).
        \see SGetShadowObject
        */
        inline scene::Mesh* getFinalModel() const
        {
            return FinalModel_;
        }
        inline const std::list<scene::Mesh*>& getSingleModels() const
        {
            return SingleModels_;
        }
        
        //! Returns the list with all generated final lightmap textures.
        inline const std::list<video::Texture*>& getLightmapTextures() const
        {
            return LightmapTextures_;
        }
        
        /**
        Sets the state callback. This function is called every time the current generation state changes.
        \param[in] Callback Specifies the callback function identifier.
        \see LightmapStateCallback
        */
        inline void setStateCallback(const LightmapStateCallback &Callback)
        {
            StateCallback_ = Callback;
        }
        
        //! Returns the generation flags.
        inline s32 getFlags() const
        {
            return State_.Flags;
        }
        //! Returns the texel blur radius which has been set the last time lightmap generation was used.
        inline u8 getTexelBlurRadius() const
        {
            return State_.TexelBlurRadius;
        }
        //! Returns the count of threads used for lightmap generation.
        inline u8 getThreadCount() const
        {
            return State_.ThreadCount;
        }
        //! Returns true if the lightmap generation has been completed successful when started last time.
        inline bool hasGeneratedSuccessful() const
        {
            return State_.HasGeneratedSuccessful;
        }
        //! Returns the lightmap ambient color. By default (20, 20, 20, 255).
        inline const video::color& getAmbientColor() const
        {
            return State_.AmbientColor;
        }
        
    private:
        
        /* === Friends === */
        
        friend struct LightmapGen::SAxisData;
        friend struct LightmapGen::SModel;
        
        friend void LMapRasterizePixelCallback(
            s32 x, s32 y, const LightmapGen::SRasterizerVertex &Vertex, void* UserData
        );
        friend void LMapBlurPixelCallback(s32 x, s32 y, void* UserData);
        
        friend THREAD_PROC(RasterizerThreadProc);
        
        /* === Structures === */
        
        struct SP_EXPORT SInternalState
        {
            SInternalState();
            ~SInternalState();
            
            /* Functions */
            void validateFlags();
            
            /* Inline functions */
            inline bool useGPU() const
            {
                return (Flags & LIGHTMAPFLAG_GPU_ACCELERATION) != 0;
            }
            inline bool useRadiosity() const
            {
                return (Flags & LIGHTMAPFLAG_RADIOSITY) != 0;
            }
            
            /* Members */
            s32 Flags;
            video::color AmbientColor;
            u8 TexelBlurRadius;
            u8 ThreadCount;
            bool HasGeneratedSuccessful;
        };
        
        /* === Functions === */
        
        void estimateEntireProgress(bool BlurEnabled);
        
        void createFacesLightmaps(LightmapGen::SModel* Model);
        
        void generateLightTexelsSingleThreaded(LightmapGen::SLight* Light);
        void generateLightTexelsMultiThreaded(LightmapGen::SLight* Light);
        
        void rasterizeTriangle(const LightmapGen::SLight* Light, const LightmapGen::STriangle &Triangle);
        
        void processTexelLighting(
            LightmapGen::SLightmapTexel* Texel, const LightmapGen::SLight* Light,
            const dim::vector3df &Position, const dim::vector3df &Normal
        );
        
        void shadeAllLightmaps();
        void shadeAllLightmapsOnCPU();
        void shadeAllLightmapsOnGPU();
        
        void partitionScene(f32 DefaultDensity);
        
        void createNewLightmap();
        void putFaceIntoLightmap(LightmapGen::SFace* Face);
        
        void buildFinalMesh(LightmapGen::SModel* Model);
        void buildAllFinalModels();
        
        void blurLightmapTexels(LightmapGen::SModel* Model, s32 Factor);
        
        void blurAllLightmaps(u8 TexelBlurRadius);
        void createFinalLightmapTextures(const video::color &AmbientColor);
        
        void updateStateInfo(const ELightmapGenerationStates State, const io::stringc &Info = "");
        
        void clearLightmapObjects();
        
        /* === Static functions === */
        
        static bool processRunning(s32 BoostFactor = 1);
        
        /* === Members === */
        
        scene::Mesh* FinalModel_;
        std::list<scene::Mesh*> SingleModels_;
        
        scene::CollisionGraph CollSys_;
        scene::CollisionMesh* CollMesh_;
        
        std::list<LightmapGen::SLight*> LightSources_;
        std::list<LightmapGen::SModel*> GetShadowObjects_;
        
        std::list<LightmapGen::SLightmap*> Lightmaps_;          //!< Lightmap objects.
        std::list<video::Texture*> LightmapTextures_;           //!< Final lightmap textures.
        
        std::map<scene::Mesh*, LightmapGen::SModel*> ModelMap_;
        
        LightmapGen::SLightmap* CurLightmap_;
        LightmapGen::TRectNode* CurRectRoot_;
        
        SInternalState State_;
        LightmapGen::ShaderDispatcher GPUDispatcher_;
        
        LightmapStateCallback StateCallback_;
        
        static LightmapProgressCallback ProgressCallback_;
        
        static s32 Progress_;
        static s32 ProgressMax_;
        static s32 ProgressShadedTriangleNum_;
        
        static video::color AmbientColor_;
        static dim::size2di LightmapSize_;
        
};


} // /namespace tool

} // /namespace sp


#endif

#endif



// ================================================================================
