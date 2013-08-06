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

extern video::RenderSystem* GlbRenderSys;

namespace video
{


const color color::empty    (  0,   0,   0,   0);
const color color::red      (255,   0,   0, 255);
const color color::green    (  0, 255,   0, 255);
const color color::blue     (  0,   0, 255, 255);
const color color::yellow   (255, 255,   0, 255);
const color color::pink     (255,   0, 255, 255);
const color color::black    (  0,   0,   0, 255);
const color color::white    (255, 255, 255, 255);


const color color::EMPTY    (  0,   0,   0,   0);
const color color::RED      (255,   0,   0, 255);
const color color::GREEN    (  0, 255,   0, 255);
const color color::BLUE     (  0,   0, 255, 255);
const color color::YELLOW   (255, 255,   0, 255);
const color color::PINK     (255,   0, 255, 255);
const color color::BLACK    (  0,   0,   0, 255);
const color color::WHITE    (255, 255, 255, 255);


MaterialStates::MaterialStates() :
    ColorDiffuse_           (255                ),
    ColorAmbient_           (50                 ),
    ColorSpecular_          (255                ),
    ColorEmission_          (0                  ),
    
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
MaterialStates::MaterialStates(const MaterialStates* Other)
{
    copy(Other);
}
MaterialStates::~MaterialStates()
{
    if (GlbRenderSys)
        GlbRenderSys->updateMaterialStates(this, true);
}

void MaterialStates::copy(const MaterialStates* Other)
{
    if (!Other)
        return;
    
    ColorDiffuse_       = Other->ColorDiffuse_;
    ColorAmbient_       = Other->ColorAmbient_;
    ColorSpecular_      = Other->ColorSpecular_;
    ColorEmission_      = Other->ColorEmission_;
    
    Shininess_          = Other->Shininess_;
    AlphaReference_     = Other->AlphaReference_;
    OffsetFactor_       = Other->OffsetFactor_;
    OffsetUnits_        = Other->OffsetUnits_;
    
    Shading_            = Other->Shading_;
    DepthMethod_        = Other->DepthMethod_;
    AlphaMethod_        = Other->AlphaMethod_;
    BlendSource_        = Other->BlendSource_;
    BlendTarget_        = Other->BlendTarget_;
    RenderModeFront_    = Other->RenderModeFront_;
    RenderModeBack_     = Other->RenderModeBack_;
    RenderFace_         = Other->RenderFace_;
    
    isColorMaterial_    = Other->isColorMaterial_;
    isLighting_         = Other->isLighting_;
    isBlending_         = Other->isBlending_;
    isZBuffer_          = Other->isZBuffer_;
    isFog_              = Other->isFog_;
    isPolygonOffset_    = Other->isPolygonOffset_;
    UserMaterialProc_   = Other->UserMaterialProc_;
    
    update();
}

bool MaterialStates::compare(const MaterialStates* Other) const
{
    /* Check simple attributes */
    if (!Other || isColorMaterial_ != Other->isColorMaterial_ || isFog_ != Other->isFog_)
        return false;
    
    /* Check face culling */
    if ( RenderFace_ != Other->RenderFace_ ||
         ( RenderFace_ == FACE_FRONT && RenderModeFront_ != Other->RenderModeFront_ ) ||
         ( RenderFace_ == FACE_BACK && RenderModeBack_ != Other->RenderModeBack_ ) ||
         ( RenderFace_ == FACE_BOTH && ( RenderModeFront_ != Other->RenderModeFront_ || RenderModeBack_ != Other->RenderModeBack_ ) ) )
    {
        return false;
    }
    
    /* Check lighting material */
    if ( isLighting_ != Other->isLighting_ || ( __isLighting && isLighting_ &&
         ( !math::equal(Shininess_, Other->Shininess_) || ColorDiffuse_ != Other->ColorDiffuse_ ||
           ColorAmbient_ != Other->ColorAmbient_ || ColorSpecular_ != Other->ColorSpecular_ || ColorEmission_ != Other->ColorEmission_ ) ) )
    {
        return false;
    }
    
    /* Check blending function */
    if ( isBlending_ != Other->isBlending_ ||
         ( isBlending_ && ( BlendSource_ != Other->BlendSource_ || BlendTarget_ != Other->BlendTarget_ ) ) )
    {
        return false;
    }
    
    /* Check depth function */
    if ( isZBuffer_ != Other->isZBuffer_ ||
         ( isZBuffer_ && DepthMethod_ != Other->DepthMethod_ ) )
    {
        return false;
    }
    
    /* Check polygon offset */
    if ( isPolygonOffset_ != Other->isPolygonOffset_ ||
         ( isPolygonOffset_ && ( OffsetFactor_ != Other->OffsetFactor_ || OffsetUnits_ != Other->OffsetUnits_ ) ) )
    {
        return false;
    }
    
    /* Check alpha function */
    if ( AlphaMethod_ != Other->AlphaMethod_ ||
         ( AlphaMethod_ != CMPSIZE_ALWAYS && math::equal(AlphaReference_, Other->AlphaReference_) ) )
    {
        return false;
    }
    
    return true;
}

void MaterialStates::update()
{
    if (GlbRenderSys)
        GlbRenderSys->updateMaterialStates(this);
}

void MaterialStates::setAlphaMethod(const ESizeComparisionTypes Method, f32 AlphaReference)
{
    math::clamp<f32>(AlphaReference, 0.0f, 1.0f);
    
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
