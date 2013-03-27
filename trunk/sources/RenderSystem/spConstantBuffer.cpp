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


ConstantBuffer::ConstantBuffer(ShaderClass* Owner, const io::stringc &Name, u32 Index) :
    Shader_         (Owner              ),
    Usage_          (HWBUFFER_DYNAMIC   ),
    HasUsageChanged_(false              ),
    Size_           (0                  ),
    Name_           (Name               ),
    Index_          (Index              )
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

void ConstantBuffer::setBufferUsage(const EHWBufferUsage Usage)
{
    if (Usage != Usage_)
    {
        HasUsageChanged_ = true;
        Usage_ = Usage;
    }
}


} // /namespace scene

} // /namespace sp



// ================================================================================
