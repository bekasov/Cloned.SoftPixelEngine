/*
 * Material states files
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "Base/spMaterialStates.hpp"
#include "Base/spInternalDeclarations.hpp"
#include "Platform/spSoftPixelDeviceOS.hpp"


namespace sp
{

extern video::RenderSystem* __spVideoDriver;

namespace video
{


MaterialStates::MaterialStates() :
    ColorDiffuse_           (255, 255, 255, 255 ),
    ColorAmbient_           ( 50,  50,  50, 255 ),
    ColorSpecular_          (255, 255, 255, 255 ),
    ColorEmission_          (  0,   0,   0, 255 ),
    
    Shininess_              (128.0f             ),
    AlphaReference_         (1.0f               ),
    OffsetFactor_           (0.0f               ),
    OffsetUnits_            (0.0f               ),
    
    Shading_                (SHADING_GOURAUD    ),
    DepthMethod_            (CMPSIZE_LESSEQUAL  ),
    AlphaMethod_            (CMPSIZE_ALWAYS     ),
    BlendSource_            (BLEND_SRCALPHA     ),
    BlendTarget_            (BLEND_INVSRCALPHA  ),
    RenderModeFront_        (WIREFRAME_SOLID    ),
    RenderModeBack_         (WIREFRAME_SOLID    ),
    RenderFace_             (FACE_FRONT         ),
    
    isColorMaterial_        (true               ),
    isLighting_             (true               ),
    isBlending_             (true               ),
    isZBuffer_              (true               ),
    isFog_                  (true               ),
    isPolygonOffset_        (false              ),
    
    UserMaterialProc_       (0                  ),
    RefRasterizerState_     (0                  ),
    RefDepthStencilState_   (0                  ),
    RefBlendState_          (0                  )
{
}
MaterialStates::MaterialStates(const MaterialStates* other)
{
    copy(other);
}
MaterialStates::~MaterialStates()
{
    __spVideoDriver->updateMaterialStates(this, true);
}

void MaterialStates::copy(const MaterialStates* other)
{
    if (!other)
        return;
    
    ColorDiffuse_       = other->ColorDiffuse_;
    ColorAmbient_       = other->ColorAmbient_;
    ColorSpecular_      = other->ColorSpecular_;
    ColorEmission_      = other->ColorEmission_;
    
    Shininess_          = other->Shininess_;
    AlphaReference_     = other->AlphaReference_;
    OffsetFactor_       = other->OffsetFactor_;
    OffsetUnits_        = other->OffsetUnits_;
    
    Shading_            = other->Shading_;
    DepthMethod_        = other->DepthMethod_;
    AlphaMethod_        = other->AlphaMethod_;
    BlendSource_        = other->BlendSource_;
    BlendTarget_        = other->BlendTarget_;
    RenderModeFront_    = other->RenderModeFront_;
    RenderModeBack_     = other->RenderModeBack_;
    RenderFace_         = other->RenderFace_;
    
    isColorMaterial_    = other->isColorMaterial_;
    isLighting_         = other->isLighting_;
    isBlending_         = other->isBlending_;
    isZBuffer_          = other->isZBuffer_;
    isFog_              = other->isFog_;
    isPolygonOffset_    = other->isPolygonOffset_;
    UserMaterialProc_   = other->UserMaterialProc_;
    
    update();
}

bool MaterialStates::compare(const MaterialStates* other) const
{
    /* Check simple attributes */
    if (!other || isColorMaterial_ != other->isColorMaterial_ || isFog_ != other->isFog_)
        return false;
    
    /* Check face culling */
    if ( RenderFace_ != other->RenderFace_ ||
         ( RenderFace_ == FACE_FRONT && RenderModeFront_ != other->RenderModeFront_ ) ||
         ( RenderFace_ == FACE_BACK && RenderModeBack_ != other->RenderModeBack_ ) ||
         ( RenderFace_ == FACE_BOTH && ( RenderModeFront_ != other->RenderModeFront_ || RenderModeBack_ != other->RenderModeBack_ ) ) )
    {
        return false;
    }
    
    /* Check lighting material */
    if ( isLighting_ != other->isLighting_ || ( __isLighting && isLighting_ &&
         ( !math::Equal(Shininess_, other->Shininess_) || ColorDiffuse_ != other->ColorDiffuse_ ||
           ColorAmbient_ != other->ColorAmbient_ || ColorSpecular_ != other->ColorSpecular_ || ColorEmission_ != other->ColorEmission_ ) ) )
    {
        return false;
    }
    
    /* Check blending function */
    if ( isBlending_ != other->isBlending_ ||
         ( isBlending_ && ( BlendSource_ != other->BlendSource_ || BlendTarget_ != other->BlendTarget_ ) ) )
    {
        return false;
    }
    
    /* Check depth function */
    if ( isZBuffer_ != other->isZBuffer_ ||
         ( isZBuffer_ && DepthMethod_ != other->DepthMethod_ ) )
    {
        return false;
    }
    
    /* Check polygon offset */
    if ( isPolygonOffset_ != other->isPolygonOffset_ ||
         ( isPolygonOffset_ && ( OffsetFactor_ != other->OffsetFactor_ || OffsetUnits_ != other->OffsetUnits_ ) ) )
    {
        return false;
    }
    
    /* Check alpha function */
    if ( AlphaMethod_ != other->AlphaMethod_ ||
         ( AlphaMethod_ != CMPSIZE_ALWAYS && math::Equal(AlphaReference_, other->AlphaReference_) ) )
    {
        return false;
    }
    
    return true;
}

void MaterialStates::update()
{
    __spVideoDriver->updateMaterialStates(this);
}

void MaterialStates::setAlphaMethod(const ESizeComparisionTypes Method, f32 AlphaReference)
{
    math::Clamp<f32>(AlphaReference, 0.0f, 1.0f);
    
    AlphaReference_ = AlphaReference;
    AlphaMethod_    = Method;
    
    update();
}


/*
 * ======= Protected: =======
 */

void MaterialStates::setDefaultBlending(
    const EDefaultBlendingTypes Mode, EBlendingTypes &SrcBlend, EBlendingTypes &DestBlend)
{
    switch (Mode)
    {
        case BLEND_NORMAL:
            SrcBlend    = BLEND_SRCALPHA;
            DestBlend   = BLEND_INVSRCALPHA;
            break;
        case BLEND_BRIGHT:
            SrcBlend    = BLEND_SRCALPHA;
            DestBlend   = BLEND_ONE;
            break;
        case BLEND_DARK:
            SrcBlend    = BLEND_DESTCOLOR;
            DestBlend   = BLEND_ZERO;
            break;
    }
}


} // /namespace video

} // /namespace sp



// ================================================================================
