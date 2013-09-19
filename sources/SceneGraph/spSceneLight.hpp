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

class SceneGraph;


/*
 * Structures
 */

/*
Light attenuation configuration structure.
\since Version 3.3
*/
struct SLightAttenuation
{
    SLightAttenuation(f32 Cnst = 1.0f, f32 Lin = 0.1f, f32 Quad = 0.4f) :
        Constant    (Cnst   ),
        Linear      (Lin    ),
        Quadratic   (Quad   )
    {
    }
    ~SLightAttenuation()
    {
    }
    
    /* Functions */
    inline void setRadius(f32 Radius)
    {
        if (Radius > math::ROUNDING_ERROR)
        {
            Constant    = 1.0f;
            Linear      = 1.0f / Radius;
            Quadratic   = 1.0f / Radius;
        }
    }
    inline f32 getRadius() const
    {
        return 1.0f / Linear;
    }
    
    /* Members */
    f32 Constant;
    f32 Linear;
    f32 Quadratic;
};

/**
Spot light cone structure. It contains the inner and outer cone angles.
\since Version 3.3
*/
struct SP_EXPORT SLightCone
{
    SLightCone(f32 Inner = 30.0f, f32 Outer = 60.0f) :
        InnerAngle(Inner),
        OuterAngle(Outer)
    {
    }
    ~SLightCone()
    {
    }
    
    /* Members */
    f32 InnerAngle;
    f32 OuterAngle;
};

/**
Light color structured. It contains an ambient, a diffuse and a specular color.
\since Version 3.3
*/
struct SLightColor
{
    SLightColor(
        const video::color &Ambt = 255, const video::color &Diff = 200, const video::color Spec = 0) :
        Ambient (Ambt),
        Diffuse (Diff),
        Specular(Spec)
    {
    }
    ~SLightColor()
    {
    }
    
    /* Members */
    video::color Ambient;
    video::color Diffuse;
    video::color Specular;
};


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
        
        /* === Functions === */
        
        /**
        Sets the new light color.
        \see SLightColor
        \since Version 3.3
        */
        void setColor(const SLightColor &Color);
        
        /**
        Sets the light's colors.
        \param Diffuse: Diffuse color. This is the 'main' lighting color which is multiplied by each vertex's color.
        \param Ambient: Ambient color. This is the darkest color. Also if the color is full bright it will not let the object
        be complete white.
        \param Specular: Specular color. If this color is not bright enough each shininess value for the Entity objects
        will not cause any shine.
        \depreacted Use setColor instead.
        */
        void setLightingColor(const video::color &Diffuse, const video::color &Ambient = 255, const video::color &Specular = 0);
        //! \deprecated Use "getColor" instead.
        void getLightingColor(video::color &Diffuse, video::color &Ambient, video::color &Specular) const;
        
        //! \deprecated Use the new "setSpotCone" function instead.
        void setSpotCone(const f32 InnerConeAngle, const f32 OuterConeAngle);
        //! \deprecated Use the new "getSpotCone" function instead.
        void getSpotCone(f32 &InnerConeAngle, f32 &OuterConeAngle) const;
        
        void setSpotConeInner(f32 Angle);
        void setSpotConeOuter(f32 Angle);
        
        /**
        Sets the new spot light cone angles.
        \see SLightCone
        \since Version 3.3
        */
        void setSpotCone(const SLightCone &SpotCone);
        
        /**
        Returns a view frustum if this is a spot light.
        \param[out] Frustum Specifies the resulting view frustum. Although the spot light is actual a cone,
        the resulting model is a frustum which can be used to render a shadow map from the light's point of view.
        \param[out] GlobalPosition Specifies the global position which can be computed on the fly.
        This can be used for frustum/frustum culling tests.
        \return True if the frustum could be computed. This requires that this light is a spot light. Otherwise false.
        */
        bool getSpotFrustum(scene::ViewFrustum &Frustum, dim::vector3df &GlobalPosition) const;
        
        /**
        Enables or disables the volumetric technic for lighting. This technic is only usable when the light
        is a Point or a Spot light. Three parameters called "Attenuation" are used for this computation.
        */
        void setVolumetric(bool IsVolumetric);
        
        /**
        Sets the volumetric radius. This function computes the threee attenuation
        parameters automatically by only one value: the Radius.
        */
        void setVolumetricRadius(f32 Radius);
        f32 getVolumetricRadius() const;
        
        /**
        Sets the volumetric range or the three attenuation values.
        Here you have to compute your own attenuations.
        \deprecated Use "setAttenuation" instead.
        */
        void setVolumetricRange(f32 Constant, f32 Linear, f32 Quadratic);
        //! \deprecated Use "getAttenuation" instead.
        void getVolumetricRange(f32 &Constant, f32 &Linear, f32 &Quadratic) const;
        
        //! Sets the light's direction. Only usable for Directional or Spot lights.
        void setDirection(const dim::vector3df &Direction);
        void setDirection(const dim::matrix4f &Matrix);
        
        //! Enables or disables the light
        void setVisible(bool isVisible);
        
        //! Copies the light objects and returns the pointer to the new instance. Don't forget to delete this object!
        Light* copy() const;
        
        /**
        Updates the light. This function is called in the "renderScene" function of the SceneManager class.
        You do not have to call this function.
        */
        virtual void render();
        
        /* === Static functions === */
        
        /**
        Sets the render-context usage for fixed-function light sources.
        \param[in] UseAllRCs Specifies whether all render-contexts are to be used or only the active one.
        By default all render contexts are affected.
        \note Disable this state when you change light states (color, visiblity etc.) every frame
        and you have several render contexts!
        */
        static void setRCUsage(bool UseAllRCs);
        
        /* === Inline functions === */
        
        //! Sets the light shading model.
        inline void setLightModel(const ELightModels Type)
        {
            LightModel_ = Type;
        }
        //! Returns the light shading model.
        inline ELightModels getLightModel() const
        {
            return LightModel_;
        }
        
        //! Sets the diffuse light color.
        inline void setDiffuseColor(const video::color &Color)
        {
            setLightingColor(Color, Color_.Ambient, Color_.Specular);
        }
        //! Returns the diffuse light color.
        inline const video::color& getDiffuseColor() const
        {
            return Color_.Diffuse;
        }
        
        //! Sets the ambient light color.
        inline void setAmbientColor(const video::color &Color)
        {
            setLightingColor(Color_.Diffuse, Color, Color_.Specular);
        }
        //! Returns the ambient light color.
        inline const video::color& getAmbientColor() const
        {
            return Color_.Ambient;
        }
        
        //! Sets the specular light color.
        inline void setSpecularColor(const video::color &Color)
        {
            setLightingColor(Color_.Diffuse, Color_.Ambient, Color);
        }
        //! Returns the specular light color.
        inline const video::color& getSpecularColor() const
        {
            return Color_.Specular;
        }
        
        /**
        Returns the light color
        \see SLightColor
        \since Version 3.3
        */
        inline const SLightColor& getColor() const
        {
            return Color_;
        }
        
        /**
        Sets the new light attenuation settings.
        \see SLightAttenuation
        \since Version 3.3
        */
        inline void setAttenuation(const SLightAttenuation &Attn)
        {
            Attn_ = Attn;
        }
        /**
        Returns the light attenuation settings
        \see SLightAttenuation
        \since Version 3.3
        */
        inline const SLightAttenuation& getAttenuation() const
        {
            return Attn_;
        }
        
        /**
        Returns the spot light cone angles.
        \see SLightCone
        \since Version 3.3
        */
        inline const SLightCone& getSpotCone() const
        {
            return SpotCone_;
        }
        
        /**
        Returns the inner spot cone angle (in degrees).
        \deprecated Use getSpotCone() instead.
        */
        inline f32 getSpotConeInner() const
        {
            return SpotCone_.InnerAngle;
        }
        /**
        Returns the outer spot cone angle (in degrees).
        \deprecated Use getSpotCone() instead.
        */
        inline f32 getSpotConeOuter() const
        {
            return SpotCone_.OuterAngle;
        }
        
        //! Returns true if this is a volumetric light. By default false.
        inline bool getVolumetric() const
        {
            return IsVolumetric_;
        }
        
        /**
        Enables or disables shadow mapping.
        \param[in] Enable Specifies whether shadow mapping is to be enabled or disabled for this light source. By default false.
        \note This can only be used in combination with the integrated advanced-renderer.
        \see video::AdvancedRenderer
        \since Version 3.2
        */
        inline void setShadow(bool Enable)
        {
            IsShadowMapping_ = Enable;
        }
        /**
        Returns true if shadow mapping is enabled for this light source.
        \since Version 3.2
        */
        inline bool getShadow() const
        {
            return IsShadowMapping_;
        }
        
        /**
        Enables or disables global illumiation.
        \param[in] Enable Specifies whether global illumination is to be enabled or disabled for this light source. By default false.
        \note This can only be used in combination with the integrated advanced-renderer.
        \see video::AdvancedRenderer
        \since Version 3.3
        */
        inline void setGlobalIllumination(bool Enable)
        {
            IsGlobalIllumination_ = Enable;
        }
        //! Returns true if global illumination is enabled for this light source.
        inline bool getGlobalIllumination() const
        {
            return IsGlobalIllumination_;
        }
        
        //! Returns the light's direction. This is only used for spot- and directional lights (LIGHT_SPOT, LIGHT_DIRECTIONAL).
        inline dim::vector3df getDirection() const
        {
            return Direction_;
        }
        
        //! Returns the projection matrix. This is only used for spot-lights (LIGHT_SPOT).
        inline dim::matrix4f getProjectionMatrix() const
        {
            return ProjectionMatrix_;
        }
        
    protected:
        
        friend class SceneGraph;
        
        /* === Members ===*/
        
        u32 LightID_;                       //!< Renderer ID number for this light.
        ELightModels LightModel_;           //!< Lighting model: Directional, Point, Spot.
        SLightColor Color_;                 //!< Light color (ambient, diffuse, specular).
        
        dim::vector3df Direction_;          //!< Spot- and directional light direction.
        SLightCone SpotCone_;               //!< Spot light cone angles.
        dim::matrix4f ProjectionMatrix_;
        
        bool IsVolumetric_;
        bool IsShadowMapping_;
        bool IsGlobalIllumination_;
        
        SLightAttenuation Attn_;
        
    private:
        
        /* === Functions === */
        
        void registerLight();
        void updateProjectionMatrix();
        
        /* === Members === */
        
        static bool UseAllRCs_;
        
};


} // /namespace scene

} // /namespace sp


#endif



// ================================================================================
