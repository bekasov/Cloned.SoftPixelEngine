/*
 * Cg shader class file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "Framework/Cg/spCgShaderClass.hpp"

#if defined(SP_COMPILE_WITH_CG)


#include "Framework/Cg/spCgShaderProgram.hpp"
#include "Platform/spSoftPixelDeviceOS.hpp"


namespace sp
{

extern video::RenderSystem* GlbRenderSys;

namespace video
{


CgShaderClass::CgShaderClass(VertexFormat* VertexInputLayout) :
    ShaderClass()
{
}
CgShaderClass::~CgShaderClass()
{
}

void CgShaderClass::bind(const scene::MaterialNode* Object)
{
    if (ObjectCallback_)
        ObjectCallback_(this, Object);
    GlbRenderSys->setSurfaceCallback(SurfaceCallback_);
    
    if (VertexShader_)
        static_cast<CgShaderProgram*>(VertexShader_)->bind();
    if (PixelShader_)
        static_cast<CgShaderProgram*>(PixelShader_)->bind();
    if (GeometryShader_)
        static_cast<CgShaderProgram*>(GeometryShader_)->bind();
    if (HullShader_)
        static_cast<CgShaderProgram*>(HullShader_)->bind();
    if (DomainShader_)
        static_cast<CgShaderProgram*>(DomainShader_)->bind();
}

void CgShaderClass::unbind()
{
    if (VertexShader_)
        static_cast<CgShaderProgram*>(VertexShader_)->unbind();
    if (PixelShader_)
        static_cast<CgShaderProgram*>(PixelShader_)->unbind();
    if (GeometryShader_)
        static_cast<CgShaderProgram*>(GeometryShader_)->unbind();
    if (HullShader_)
        static_cast<CgShaderProgram*>(HullShader_)->unbind();
    if (DomainShader_)
        static_cast<CgShaderProgram*>(DomainShader_)->unbind();
}

bool CgShaderClass::link()
{
    return
        VertexShader_       &&   VertexShader_  ->valid()     &&
        ( !PixelShader_     || ( PixelShader_   ->valid() ) ) &&
        ( !GeometryShader_  || ( GeometryShader_->valid() ) ) &&
        ( !HullShader_      || ( HullShader_    ->valid() ) ) &&
        ( !DomainShader_    || ( DomainShader_  ->valid() ) );
}


} // /namespace scene

} // /namespace sp


#endif



// ================================================================================
