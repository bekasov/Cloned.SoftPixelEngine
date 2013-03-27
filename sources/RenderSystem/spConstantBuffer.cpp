/*
 * Shader constant buffer file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "RenderSystem/spConstantBuffer.hpp"
#include "Base/spBaseExceptions.hpp"


namespace sp
{
namespace video
{


ConstantBuffer::ConstantBuffer(ShaderClass* Owner, const io::stringc &Name) :
    Shader_ (Owner          ),
    Name_   (Name           ),
    Usage_  (HWBUFFER_STATIC)
{
    if (!Shader_)
        throw io::NullPointerException("ConstantBuffer");
    if (Name_.empty())
        throw io::DefaultException("\"ConstantBuffer\" must not have empty name");
}
ConstantBuffer::~ConstantBuffer()
{
}

bool ConstantBuffer::updateBuffer(const void* Buffer, u32 Size)
{
    return false; // do nothing
}

bool ConstantBuffer::valid() const
{
    return false; // do nothing
}


} // /namespace scene

} // /namespace sp



// ================================================================================
