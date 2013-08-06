/*
 * Vertex format file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "Base/spVertexFormat.hpp"
#include "RenderSystem/spRenderSystem.hpp"
#include "RenderSystem/Direct3D11/spDirect3D11RenderSystem.hpp"

#include <boost/foreach.hpp>


namespace sp
{

extern video::RenderSystem* GlbRenderSys;

namespace video
{


VertexFormat::VertexFormat() :
    Flags_      (0),
    InputLayout_(0)
{
}
VertexFormat::VertexFormat(const io::stringc &Name, s32 Flags) :
    Flags_      (Flags  ),
    Name_       (Name   ),
    InputLayout_(0      )
{
}
VertexFormat::~VertexFormat()
{
    /* Delete vertex input layout (only used for Direct3D11) */
    GlbRenderSys->updateVertexInputLayout(this, false);
}

io::stringc VertexFormat::getIdentifier() const
{
    if (Name_.size())
        return "vertex format \"" + Name_ + "\"";
    return "anonymous vertex format";
}

u32 VertexFormat::getFormatSize() const
{
    u32 Size;
    getFormatSize(Size);
    return Size;
}

s32 VertexFormat::getDataTypeSize(const ERendererDataTypes Type)
{
    static const s32 SizeList[] = { 4, 8, 1, 2, 4, 1, 2, 4 };
    return SizeList[Type];
}


/*
 * ======= Protected: =======
 */

void VertexFormat::getFormatSize(u32 &Size) const
{
    Size = 0;
    
    if (Flags_ & VERTEXFORMAT_COORD)
        addComponentFormatSize(Size, Coord_);
    if (Flags_ & VERTEXFORMAT_COLOR)
        addComponentFormatSize(Size, Color_);
    if (Flags_ & VERTEXFORMAT_NORMAL)
        addComponentFormatSize(Size, Normal_);
    if (Flags_ & VERTEXFORMAT_BINORMAL)
        addComponentFormatSize(Size, Binormal_);
    if (Flags_ & VERTEXFORMAT_TANGENT)
        addComponentFormatSize(Size, Tangent_);
    if (Flags_ & VERTEXFORMAT_FOGCOORD)
        addComponentFormatSize(Size, FogCoord_);
    if (Flags_ & VERTEXFORMAT_TEXCOORDS)
    {
        for (std::vector<SVertexAttribute>::const_iterator it = TexCoords_.begin(); it != TexCoords_.end(); ++it)
            addComponentFormatSize(Size, *it);
    }
    if (Flags_ & VERTEXFORMAT_UNIVERSAL)
    {
        for (std::vector<SVertexAttribute>::const_iterator it = Universals_.begin(); it != Universals_.end(); ++it)
            addComponentFormatSize(Size, *it);
    }
}

void VertexFormat::addComponentFormatSize(u32 &Size, const SVertexAttribute &Attrib) const
{
    if (!Attrib.isReference)
        Size += Attrib.Size * getDataTypeSize(Attrib.Type);
}

void VertexFormat::constructComponent(
    const EVertexFormatFlags Type, SVertexAttribute &Attrib, s32 &Offset, s32 MinSize, s32 MaxSize)
{
    if ((Flags_ & Type) && !Attrib.isReference)
    {
        math::clamp(Attrib.Size, MinSize, MaxSize);
        Attrib.Offset = Offset;
        Offset += getDataTypeSize(Attrib.Type) * Attrib.Size;
    }
}

void VertexFormat::constructFormat()
{
    if (GlbRenderSys->getRendererType() == RENDERER_DIRECT3D9)
    {
        Color_.Size = 4;
        Color_.Type = DATATYPE_UNSIGNED_BYTE;
    }
    
    s32 Offset = 0;
    
    constructComponent(VERTEXFORMAT_COORD, Coord_, Offset, 2, 4);
    constructComponent(VERTEXFORMAT_NORMAL, Normal_, Offset, 3, 3);
    constructComponent(VERTEXFORMAT_COLOR, Color_, Offset, 3, 4);
    
    foreach (SVertexAttribute &Attrib, TexCoords_)
        constructComponent(VERTEXFORMAT_TEXCOORDS, Attrib, Offset, 1, 4);
    
    constructComponent(VERTEXFORMAT_FOGCOORD, FogCoord_, Offset, 1, 1);
    constructComponent(VERTEXFORMAT_TANGENT, Tangent_, Offset, 3, 3);
    constructComponent(VERTEXFORMAT_BINORMAL, Binormal_, Offset, 3, 3);
    
    foreach (SVertexAttribute &Attrib, Universals_)
        constructComponent(VERTEXFORMAT_UNIVERSAL, Attrib, Offset, 1, 4);
    
    /* Update vertex input layout (only used for Direct3D11) */
    GlbRenderSys->updateVertexInputLayout(this, true);
}


} // /namespace video

} // /namespace sp



// ================================================================================
