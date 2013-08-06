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

const f32 Light::DEF_SPOTANGLE_INNER = 30.0f;
const f32 Light::DEF_SPOTANGLE_OUTER = 60.0f;

bool Light::UseAllRCs_ = true;

Light::Light(const ELightModels Type) :
    SceneNode               (NODE_LIGHT                 ),
    LightID_                (0                          ),
    LightModel_             (Type                       ),
    Direction_              (0, 0, 1                    ),
    SpotInnerConeAngle_     (Light::DEF_SPOTANGLE_INNER ),
    SpotOuterConeAngle_     (Light::DEF_SPOTANGLE_OUTER ),
    isVolumetric_           (false                      ),
    hasShadow_              (false                      ),
    AttenuationConstant_    (1.0f                       ),
    AttenuationLinear_      (0.1f                       ),
    AttenuationQuadratic_   (0.4f                       ),
    DiffuseColor_           (200                        ),
    AmbientColor_           (255                        ),
    SpecularColor_          (0                          )
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

void Light::setLightingColor(const video::color &Diffuse, const video::color &Ambient, const video::color &Specular)
{
    /* General settings */
    DiffuseColor_   = Diffuse;
    AmbientColor_   = Ambient;
    SpecularColor_  = Specular;
    
    /* Update lighting colors */
    GlbRenderSys->setLightColor(LightID_, DiffuseColor_, AmbientColor_, SpecularColor_, Light::UseAllRCs_);
}
void Light::getLightingColor(video::color &Diffuse, video::color &Ambient, video::color &Specular) const
{
    Diffuse     = DiffuseColor_;
    Ambient     = AmbientColor_;
    Specular    = SpecularColor_;
}

void Light::setSpotCone(const f32 InnerConeAngle, const f32 OuterConeAngle)
{
    SpotInnerConeAngle_ = InnerConeAngle;
    SpotOuterConeAngle_ = OuterConeAngle;
    updateProjectionMatrix();
}
void Light::getSpotCone(f32 &InnerConeAngle, f32 &OuterConeAngle) const
{
    InnerConeAngle = SpotInnerConeAngle_;
    OuterConeAngle = SpotOuterConeAngle_;
}

void Light::setSpotConeInner(f32 Angle)
{
    SpotInnerConeAngle_ = Angle;
    updateProjectionMatrix();
}
void Light::setSpotConeOuter(f32 Angle)
{
    SpotOuterConeAngle_ = Angle;
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

void Light::setVolumetric(bool isVolumetric)
{
    if (!isVolumetric)
    {
        f32 tmp1 = 1.0f, tmp2 = 0.0f;
        
        /* Update the renderer for the light */
        GlbRenderSys->updateLight(
            LightID_, LightModel_, isVolumetric_,
            Direction_, SpotInnerConeAngle_, SpotOuterConeAngle_,
            tmp1, tmp2, tmp2
        );
    }
    
    isVolumetric_ = isVolumetric;
}

/*
                1
 _________________________________

  Att0 + (Att1 * d) + (Att2 * d²)
*/

void Light::setVolumetricRadius(f32 Radius)
{
    if (Radius > math::ROUNDING_ERROR)
    {
        AttenuationConstant_    = 1.0f;
        AttenuationLinear_      = 1.0f / Radius;
        AttenuationQuadratic_   = 1.0f / Radius;
    }
}
f32 Light::getVolumetricRadius() const
{
    return 1.0f / AttenuationLinear_;
}

void Light::setVolumetricRange(f32 Constant, f32 Linear, f32 Quadratic)
{
    AttenuationConstant_    = Constant;
    AttenuationLinear_      = Linear;
    AttenuationQuadratic_   = Quadratic;
}
void Light::getVolumetricRange(f32 &Constant, f32 &Linear, f32 &Quadratic) const
{
    Constant    = AttenuationConstant_;
    Linear      = AttenuationLinear_;
    Quadratic   = AttenuationQuadratic_;
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
    NewLight->Direction_            = Direction_;
    NewLight->SpotInnerConeAngle_   = SpotInnerConeAngle_;
    NewLight->SpotOuterConeAngle_   = SpotOuterConeAngle_;
    
    NewLight->isVolumetric_         = isVolumetric_;
    NewLight->hasShadow_            = hasShadow_;
    
    NewLight->AttenuationConstant_  = AttenuationConstant_;
    NewLight->AttenuationLinear_    = AttenuationLinear_;
    NewLight->AttenuationQuadratic_ = AttenuationQuadratic_;
    
    NewLight->AmbientColor_         = AmbientColor_;
    NewLight->SpecularColor_        = SpecularColor_;
    
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
        LightID_, LightModel_, isVolumetric_,
        Direction_, SpotInnerConeAngle_, SpotOuterConeAngle_,
        AttenuationConstant_, AttenuationLinear_, AttenuationQuadratic_
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
            DiffuseColor_, AmbientColor_, SpecularColor_,
            AttenuationConstant_, AttenuationLinear_, AttenuationQuadratic_
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
 
