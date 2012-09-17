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

//! \todo This is unfinished
class SP_EXPORT ShadowMapper
{
    
    public:
        
        ShadowMapper();
        virtual ~ShadowMapper();
        
        /* === Functions === */
        
        virtual void createShadowMaps(u32 MaxPointLightCount, u32 MaxSpotLightCount);
        virtual void deleteShadowMaps();
        
        virtual bool renderShadowMap(scene::SceneGraph* Graph, scene::Light* LightObj, u32 Index);
        
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
        inline video::Texture* getSpotLightTexArray()
        {
            return SpotLightTexArray_;
        }
        //! Returns the point light cube texture array for shadow mapping.
        inline video::Texture* getPointLightTexArray()
        {
            return PointLightTexArray_;
        }
        
    protected:
        
        /* === Macros === */
        
        static const dim::matrix4f CUBEMAP_ROTATIONS[6];
        
        /* === Functions === */
        
        //! \warning Does not check for null pointers!
        bool renderPointLightShadowMap(scene::SceneGraph* Graph, scene::Light* LightObj, u32 Index);
        //! \warning Does not check for null pointers!
        bool renderSpotLightShadowMap(scene::SceneGraph* Graph, scene::Light* LightObj, u32 Index);
        
        static void renderCubeMapDirection(
            scene::SceneGraph* Graph, Texture* Tex, const ECubeMapDirections Direction
        );
        
        /* === Members === */
        
        video::Texture* PointLightTexArray_;
        video::Texture* SpotLightTexArray_;
        
        u32 MaxPointLightCount_;
        u32 MaxSpotLightCount_;
        
        static scene::Camera ViewCam_;
        
};


} // /namespace video

} // /namespace sp


#endif



// ================================================================================
