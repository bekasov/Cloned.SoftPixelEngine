/*
 * Shadow mapper header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_SHADOW_MAPPER_H__
#define __SP_SHADOW_MAPPER_H__


#include "Base/spStandard.hpp"
#include "Base/spDimension.hpp"
#include "RenderSystem/spTextureFlags.hpp"
#include "SceneGraph/spSceneCamera.hpp"


namespace sp
{

namespace scene
{
    class SceneGraph;
    class Light;
}

namespace video
{


class Texture;

/**
The shadow mapper class is used to render the textures for real-time shadow mapping effects.
Supported are shadow maps for PCF (percentage closer filtering), VSMs (variance shadow maps) and
RSMs (reflective shadow maps). This class also provides a couple of functions for generating
color cube maps. You can use this class to make your own shadow map rendering or to use it
with the integrated deferred renderer.
\since Version 3.2
*/
class SP_EXPORT ShadowMapper
{
    
    public:
        
        ShadowMapper();
        virtual ~ShadowMapper();
        
        /* === Functions === */
        
        /**
        Creates the shadow map texture arrays.
        \param[in] TexSize Specifies the size for each shadow map. Use values like 128, 256, 512 etc.
        \param[in] MaxPointLightCount Specifies the count of point-lights.
        \param[in] MaxSpotLightCount Specifies the count of spot-lights.
        \param[in] UseVSM Specifies whether VSM (variance shadow maps) are used or not. If true the shadow maps
        provide two components for each texel (PIXELFORMAT_GRAYALPHA). Otherwise only one (PIXELFORMAT_GRAY). By default true.
        \param[in] UseRSM Specifies whether RSM (reflective shadow maps) are used or not. If true additional textures are used,
        which store color- and normal information like a g-buffer. RSMs are used for real-time global illumination
        and can be combined with VSMs. This is a very time consuming effect and should only be used for a small
        count of light sources! By default false.
        \return True if the shadow map textures were created successful.
        */
        virtual bool createShadowMaps(
            s32 TexSize, u32 MaxPointLightCount, u32 MaxSpotLightCount, bool UseVSM = true, bool UseRSM = false
        );
        //! Deletes the shadow maps.
        virtual void deleteShadowMaps();
        
        /**
        Renders the shadow map for the specified light object with the given scene graph and its camera.
        \param Graph: Specifies the scene graph which is to be rendered into the shadow map.
        \param Cam: Specifies the view camera. The light's view frustum will be tested against the view frustum
        of this camera object. If a light's view frustum is not inside the shadow map will not be rendered.
        If you don't want this test, just set this pointer to null.
        \param LightObj: Specifies the light object from which point of view the scene is to be renderd.
        \param Index: Specifies the shadow map array index. This value begins with 0 for each light type
        i.e. there is an array for each light type (point, spot and directional lights).
        \return True if a shadow map has been rendered. Otherwise the parameters are invalid or the shadow map
        has not been rendered because of performance optimization.
        */
        virtual bool renderShadowMap(
            scene::SceneGraph* Graph, scene::Camera* Cam, scene::Light* LightObj, u32 Index
        );
        
        //! Binds all shadow map layers and returns the next valid texture layer index.
        virtual s32 bind(s32 StartSlot);
        //! Unbinds all shadow map layers and returns the next valid texture layer index.
        virtual s32 unbind(s32 StartSlot);
        
        /* === Static functions === */
        
        /**
        Renders a complete cube map for the specified texture object.
        \param Graph: Specifies the scene graph which is to be rendered.
        \param Cam: Specifies the camera object which from which point of view the scene is to be rendered.
        Don't use your standard camera object. Create a separate camera for rendering into cube maps,
        set its field-of-view to 90 degrees (scene::Camera::setFOV) and dont set its parent.
        \param Tex: Specifies the texture which is to be used. This texture needs to be a valid cube map.
        To achieve that call the "Texture::setDimension(DIMENSION_CUBEMAP)" function.
        \param Position: Specifies the position where the 'screenshot-like' cube map is to be generated.
        \return True of success otherwise false.
        */
        static bool renderCubeMap(
            scene::SceneGraph* Graph, scene::Camera* Cam, Texture* Tex, const dim::vector3df &Position
        );
        
        /**
        Renders the specified cube map side. This is used inside the renderCubeMap function.
        \param Graph: Specifies the scene graph which is to be rendered.
        \param Cam: Specifies the camera object which from which point of view the scene is to be rendered.
        \param Tex: Specifies the cube map Texture object. This texture needs to be a valid cube map.
        \param CamDir: Specifies the camera direction matrix. This function should be called
        six times (for all compass points). With each call the camera direction has to be a different orientation.
        \param Direction: Specifies the type of cube map direction.
        \return True of success otherwise false.
        \see renderCubeMap
        */
        static bool renderCubeMapDirection(
            scene::SceneGraph* Graph, scene::Camera* Cam, Texture* Tex,
            dim::matrix4f CamDir, const ECubeMapDirections Direction
        );
        
        /**
        Renders a complete cube map for the specified texture object without any view orientation.
        \param Graph: Specifies the scene graph which is to be rendered.
        \param Tex: Specifies the texture which is to be used. This texture needs to be a valid cube map.
        To achieve that call the "Texture::setDimension(DIMENSION_CUBEMAP)" function.
        \param Position: Specifies the position where the 'screenshot-like' cube map is to be generated.
        \return True of success otherwise false.
        */
        static bool renderCubeMap(
            scene::SceneGraph* Graph, Texture* Tex, const dim::vector3df &Position
        );
        
        /* === Inline functions === */
        
        //! Returns the spot light texture array for shadow mapping.
        inline Texture* getSpotLightTexArray()
        {
            return ShadowMapArray_.getDepthMap();
        }
        //! Returns the point light cube texture array for shadow mapping.
        inline Texture* getPointLightTexArray()
        {
            return ShadowCubeMapArray_.getDepthMap();
        }
        
        //! Returns true if VSM (variance shadow maps) are used.
        inline bool useVSM() const
        {
            return UseVSM_;
        }
        //! Returns true if RSM (reflective shadow maps) are used (for global illumination).
        inline bool useRSM() const
        {
            return UseRSM_;
        }
        
    protected:
        
        /* === Macros === */
        
        static const dim::matrix4f CUBEMAP_ROTATIONS[6];
        
        /* === Structures === */
        
        struct SP_EXPORT SShadowMap
        {
            SShadowMap();
            ~SShadowMap();
            
            /* Functions */
            void clear();
            
            void createTexture(u32 Index, const STextureCreationFlags &CreationFlags);
            void createRSMs(STextureCreationFlags CreationFlags);
            
            bool setupRenderTargets(bool UseRSM);
            
            /* Inline functions */
            inline Texture* getDepthMap() const
            {
                return TexList[0];
            }
            inline Texture* getColorMap() const
            {
                return TexList[1];
            }
            /*inline Texture* getNormalMap() const
            {
                return TexList[2];
            }*/
            
            inline bool valid() const
            {
                return TexList[0] != 0 && TexList[1] != 0;// && TexList[2] != 0;
            }
            
            /* Members */
            Texture* TexList[2];//[3];
        };
        
        /* === Functions === */
        
        //! \warning Does not check for null pointers!
        bool renderPointLightShadowMap(
            scene::SceneGraph* Graph, scene::Camera* Cam, scene::Light* LightObj, u32 Index
        );
        //! \warning Does not check for null pointers!
        bool renderSpotLightShadowMap(
            scene::SceneGraph* Graph, scene::Camera* Cam, scene::Light* LightObj, u32 Index
        );
        
        //! \warning Does not check for null pointers!
        bool checkLightFrustumCulling(scene::Camera* Cam, scene::Light* LightObj) const;
        
        void renderSceneIntoDepthTexture(scene::SceneGraph* Graph, SShadowMap &ShadowMap, u32 Index);
        void renderSceneIntoGBuffer     (scene::SceneGraph* Graph, SShadowMap &ShadowMap, u32 Index);
        
        static void renderCubeMapDirection(
            scene::SceneGraph* Graph, Texture* Tex, const ECubeMapDirections Direction
        );
        
        /* === Members === */
        
        SShadowMap ShadowMapArray_;
        SShadowMap ShadowCubeMapArray_;
        
        scene::Camera DepthCam_;
        
        s32 TexSize_;
        
        u32 MaxPointLightCount_;
        u32 MaxSpotLightCount_;
        
        bool UseVSM_; //!< Specifies whether VSMs (variance shadow maps) are used or not.
        bool UseRSM_; //!< Specifies whether RSMs (reflective shadow maps) are used or not.
        
        static scene::Camera ViewCam_;
        
};


} // /namespace video

} // /namespace sp


#endif



// ================================================================================
