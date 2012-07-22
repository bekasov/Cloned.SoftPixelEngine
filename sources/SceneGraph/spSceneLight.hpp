/*
 * Light scene node header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_SCENE_LIGHT_H__
#define __SP_SCENE_LIGHT_H__


#include "Base/spStandard.hpp"
#include "Base/spDimensionMatrix4.hpp"
#include "SceneGraph/spRenderNode.hpp"


namespace sp
{
namespace scene
{


/*
 * Macros
 */

static const s32 MAX_COUNT_OF_SCENELIGHTS = 0x0D31;

extern bool __spLightIDList[MAX_COUNT_OF_SCENELIGHTS];

class SceneGraph;


/**
Lights can be created for dynamic lighting and shading technics. But you will only see a maximum of 8 lights
because more are not supported by the current graphics cards! But you can create much more then 8.
But be aware of that you will only see 8 of them ;-). The engine is able to manage the lighting system good enough
that these ones who are near by the camera will be visible. Directional lights (scene::LIGHT_DIRECTIONAL)
have a priority in this sort process because they are never volumetric but endless visible.
*/
class SP_EXPORT Light : public SceneNode
{
    
    public:
        
        Light(const ELightModels Type = LIGHT_DIRECTIONAL);
        virtual ~Light();
        
        //! Sets the light's type. This is the same type when creating the light witht he SceneManager.
        void setLightingType(const ELightModels Type);
        inline ELightModels getLightingType() const
        {
            return LightingType_;
        }
        
        /**
        Sets the light's colors.
        \param Diffuse: Diffuse color. This is the 'main' lighting color which is multiplied by each vertex's color.
        \param Ambient: Ambient color. This is the darkest color. Also if the color is full bright it will not let the object
        be complete white.
        \param Specular: Specular color. If this color is not bright enough each shininess value for the Entity objects
        will not cause any shine.
        */
        void setLightingColor(const video::color &Diffuse, const video::color &Ambient = 255, const video::color &Specular = 0);
        void getLightingColor(video::color &Diffuse, video::color &Ambient, video::color &Specular) const;
        
        inline void setDiffuseColor(const video::color &Color)
        {
            setLightingColor(Color, AmbientColor_, SpecularColor_);
        }
        inline video::color getDiffuseColor() const
        {
            return DiffuseColor_;
        }
        
        inline void setAmbientColor(const video::color &Color)
        {
            setLightingColor(DiffuseColor_, Color, SpecularColor_);
        }
        inline video::color getAmbientColor() const
        {
            return AmbientColor_;
        }
        
        inline void setSpecularColor(const video::color &Color)
        {
            setLightingColor(DiffuseColor_, AmbientColor_, Color);
        }
        inline video::color getSpecularColor() const
        {
            return SpecularColor_;
        }
        
        /**
        Sets the spot light cone ranges. A spot light is formed like a cone or better two cones where the inner coner angle
        has to be smaller then the outer.
        \param InnterConeAngle: Angle of the inner spot-light's cone.
        \param OutterConeAngle: Angle of the outer spot-light's cone.
        */
        void setSpotCone(const f32 InnerConeAngle, const f32 OuterConeAngle);
        void getSpotCone(f32 &InnerConeAngle, f32 &OuterConeAngle) const;
        
        inline void setSpotConeInner(f32 Angle)
        {
            SpotInnerConeAngle_ = Angle;
        }
        inline f32 getSpotConeInner() const
        {
            return SpotInnerConeAngle_;
        }
        
        inline void setSpotConeOuter(f32 Angle)
        {
            SpotOuterConeAngle_ = Angle;
        }
        inline f32 getSpotConeOuter() const
        {
            return SpotOuterConeAngle_;
        }
        
        /**
        Enables or disables the volumetric technic for lighting. This technic is only usable when the light
        is a Point or a Spot light. Three parameters called "Attenuation" are used for this computation.
        */
        void setVolumetric(bool isVolumetric);
        inline bool getVolumetric() const
        {
            return isVolumetric_;
        }
        
        /**
        Sets the volumetric radius. This function computes the threee attenuation
        parameters automatically by only one value: the Radius.
        */
        void setVolumetricRadius(f32 Radius);
        f32 getVolumetricRadius() const;
        
        /**
        Sets the volumetric range or the three attenuation values.
        Here you have to compute your own attenuations.
        */
        void setVolumetricRange(f32 Constant, f32 Linear, f32 Quadratic);
        void getVolumetricRange(f32 &Constant, f32 &Linear, f32 &Quadratic) const;
        
        //! Sets the light's direction. Only usable for Directional or Spot lights.
        void setDirection(const dim::vector3df &Direction);
        void setDirection(const dim::matrix4f &Matrix);
        inline dim::vector3df getDirection() const
        {
            return Direction_;
        }
        
        //! Enables or disables the light
        void setVisible(bool isVisible);
        
        /* Updating & copying */
        
        Light* copy() const;
        
        /**
        Updates the light. This function is called in the "renderScene" function of the SceneManager class.
        You do not have to call this function.
        */
        virtual void render();
        
    protected:
        
        friend class SceneGraph;
        
        /* === Macros === */
        
        static const f32 DEF_SPOTANGLE_INNER;
        static const f32 DEF_SPOTANGLE_OUTER;
        
        /* === Members ===*/
        
        u32 LightID_;               //!< Renderer ID number for this light.
        ELightModels LightingType_; //!< Lighting model: Directional, Point, Spot.
        
        dim::vector3df Direction_;  //!< Spot- and directional light direction.
        f32 SpotInnerConeAngle_;    //!< Inner cone angle for spot lights.
        f32 SpotOuterConeAngle_;    //!< Outer cone angle for spot lights.
        
        bool isVolumetric_;
        f32 AttenuationConstant_;
        f32 AttenuationLinear_;
        f32 AttenuationQuadratic_;
        
        video::color DiffuseColor_, AmbientColor_, SpecularColor_;
        
    private:
        
        /* === Functions === */
        
        void registerLight();
        
};


} // /namespace scene

} // /namespace sp


#endif



// ================================================================================
