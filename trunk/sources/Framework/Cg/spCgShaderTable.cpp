/*
 * Cg shader table file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "Framework/Cg/spCgShaderTable.hpp"

#if defined(SP_COMPILE_WITH_CG)


#include "Framework/Cg/spCgShaderProgram.hpp"
#include "Platform/spSoftPixelDeviceOS.hpp"


namespace sp
{

extern video::RenderSystem* __spVideoDriver;

namespace video
{


CgShaderTable::CgShaderTable(VertexFormat* VertexInputLayout) : ShaderTable()
{
}
CgShaderTable::~CgShaderTable()
{
}

void CgShaderTable::bind(const scene::MaterialNode* Object)
{
    if (ObjectCallback_)
        ObjectCallback_(this, Object);
    __spVideoDriver->setSurfaceCallback(SurfaceCallback_);
    
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

void CgShaderTable::unbind()
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

bool CgShaderTable::link()
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
