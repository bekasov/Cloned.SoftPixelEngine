/*
 * Light scene node file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "SceneGraph/spSceneLight.hpp"
#include "Platform/spSoftPixelDeviceOS.hpp"
#include "Base/spBaseExceptions.hpp"


namespace sp
{

extern video::RenderSystem* GlbRenderSys;

namespace scene
{


bool __spLightIDList[MAX_COUNT_OF_SCENELIGHTS] = { 0 };

bool Light::UseAllRCs_ = true;

Light::Light(const ELightModels Type) :
    SceneNode               (NODE_LIGHT ),
    LightID_                (0          ),
    LightModel_             (Type       ),
    Direction_              (0, 0, 1    ),
    IsVolumetric_           (false      ),
    IsShadowMapping_        (false      ),
    IsGlobalIllumination_   (false      )
{
    if (GlbRenderSys)
    {
        registerLight();
        updateProjectionMatrix();
    }
    else
        throw io::RenderSystemException("Light");
}
Light::~Light()
{
    GlbRenderSys->setLightStatus(LightID_, false, true);
    __spLightIDList[LightID_] = false;
}

void Light::setColor(const SLightColor &Color)
{
    Color_ = Color;
    GlbRenderSys->setLightColor(LightID_, Color_.Diffuse, Color_.Ambient, Color_.Specular, Light::UseAllRCs_);
}

void Light::setLightingColor(const video::color &Diffuse, const video::color &Ambient, const video::color &Specular)
{
    //!DEPRECATED!
    setColor(SLightColor(Ambient, Diffuse, Specular));
}
void Light::getLightingColor(video::color &Diffuse, video::color &Ambient, video::color &Specular) const
{
    //!DEPRECATED!
    Diffuse     = Color_.Diffuse;
    Ambient     = Color_.Ambient;
    Specular    = Color_.Specular;
}

void Light::setSpotCone(const SLightCone &SpotCone)
{
    SpotCone_ = SpotCone;
    updateProjectionMatrix();
}

void Light::setSpotCone(const f32 InnerConeAngle, const f32 OuterConeAngle)
{
    //!DEPRECATED!
    setSpotCone(SLightCone(InnerConeAngle, OuterConeAngle));
}
void Light::getSpotCone(f32 &InnerConeAngle, f32 &OuterConeAngle) const
{
    //!DEPRECATED!
    InnerConeAngle = SpotCone_.InnerAngle;
    OuterConeAngle = SpotCone_.OuterAngle;
}

void Light::setSpotConeInner(f32 Angle)
{
    SpotCone_.InnerAngle = Angle;
    updateProjectionMatrix();
}
void Light::setSpotConeOuter(f32 Angle)
{
    SpotCone_.OuterAngle = Angle;
    updateProjectionMatrix();
}

bool Light::getSpotFrustum(scene::ViewFrustum &Frustum, dim::vector3df &GlobalPosition) const
{
    if (LightModel_ == LIGHT_SPOT)
    {
        const dim::matrix4f LightMat(getTransformMatrix(true));
        Frustum.setFrustum(LightMat, getProjectionMatrix());
        GlobalPosition = LightMat.getPosition();
        return true;
    }
    return false;
}

void Light::setVolumetric(bool IsVolumetric)
{
    if (!IsVolumetric)
    {
        f32 tmp1 = 1.0f, tmp2 = 0.0f;
        
        /* Update the renderer for the light */
        GlbRenderSys->updateLight(
            LightID_, LightModel_, IsVolumetric_,
            Direction_, SpotCone_.InnerAngle, SpotCone_.OuterAngle,
            tmp1, tmp2, tmp2
        );
    }
    
    IsVolumetric_ = IsVolumetric;
}

/*
                1
 _________________________________

  Att0 + (Att1 * d) + (Att2 * d²)
*/

void Light::setVolumetricRadius(f32 Radius)
{
    Attn_.setRadius(Radius);
}
f32 Light::getVolumetricRadius() const
{
    return Attn_.getRadius();
}

void Light::setVolumetricRange(f32 Constant, f32 Linear, f32 Quadratic)
{
    //!DEPRECATED!
    Attn_.Constant  = Constant;
    Attn_.Linear    = Linear;
    Attn_.Quadratic = Quadratic;
}
void Light::getVolumetricRange(f32 &Constant, f32 &Linear, f32 &Quadratic) const
{
    //!DEPRECATED!
    Constant    = Attn_.Constant;
    Linear      = Attn_.Linear;
    Quadratic   = Attn_.Quadratic;
}

void Light::setDirection(const dim::vector3df &Direction)
{
    Direction_ = Direction;
    Direction_.normalize();
}
void Light::setDirection(const dim::matrix4f &Matrix)
{
    Direction_ = Matrix * dim::vector3df(0, 0, 1);
    Direction_.normalize();
}

void Light::setVisible(bool isVisible)
{
    Node::setVisible(isVisible);
    GlbRenderSys->setLightStatus(LightID_, isVisible_, Light::UseAllRCs_);
}

Light* Light::copy() const
{
    /* Allocate a new light */
    Light* NewLight = new Light(LightModel_);
    
    /* Copy the root attributes */
    copyRoot(NewLight);
    
    /* Copy the light configurations */
    NewLight->Direction_        = Direction_;
    NewLight->SpotCone_         = SpotCone_;
    NewLight->IsVolumetric_     = IsVolumetric_;
    NewLight->IsShadowMapping_  = IsShadowMapping_;
    NewLight->Color_            = Color_;
    NewLight->Attn_             = Attn_;
    
    /* Return the new light */
    return NewLight;
}

void Light::render()
{
    /* Matrix transformation */
    spWorldMatrix = getTransformMatrix(true);
    
    /* Update the render matrix */
    GlbRenderSys->updateModelviewMatrix();
    
    /* Update the renderer for the light */
    GlbRenderSys->updateLight(
        LightID_, LightModel_, IsVolumetric_,
        Direction_, SpotCone_.InnerAngle, SpotCone_.OuterAngle,
        Attn_.Constant, Attn_.Linear, Attn_.Quadratic
    );
}

void Light::setRCUsage(bool UseAllRCs)
{
    UseAllRCs_ = UseAllRCs;
}


/*
 * ======= Private: =======
 */

void Light::registerLight()
{
    u32 i = 0;
    
    /* Check if the there is still enough space for an additional light */
    for (; i < MAX_COUNT_OF_SCENELIGHTS; ++i)
    {
        if (!__spLightIDList[i])
        {
            __spLightIDList[i]  = true;
            LightID_            = i;
            break;
        }
    }
    
    /* Check if the light has an identity */
    if (i < MAX_COUNT_OF_SCENELIGHTS)
    {
        /* Add a dynamic light soruce */
        GlbRenderSys->addDynamicLightSource(
            LightID_, LightModel_,
            Color_.Diffuse, Color_.Ambient, Color_.Specular,
            Attn_.Constant, Attn_.Linear, Attn_.Quadratic
        );
    }
    else
    {
        io::Log::warning(
            "To many lights in use, only " + io::stringc(MAX_COUNT_OF_SCENELIGHTS) + " lights are available"
        );
    }
}

void Light::updateProjectionMatrix()
{
    switch (LightModel_)
    {
        case LIGHT_SPOT:
        {
            /* Check which projection matrix the renderer is using */
            if (GlbRenderSys->getProjectionMatrixType() == dim::MATRIX_LEFTHANDED)
                ProjectionMatrix_.setPerspectiveLH(getSpotConeOuter(), 1.0f, 0.01f, 1000.0f);
            else
                ProjectionMatrix_.setPerspectiveRH(getSpotConeOuter(), 1.0f, 0.01f, 1000.0f);
        }
        break;
    }
}


} // /namespace scene

} // /namespace sp



// ================================================================================
 
