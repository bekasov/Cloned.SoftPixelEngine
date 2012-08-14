/*
 * Universal vertex format file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "Base/spVertexFormatUniversal.hpp"


namespace sp
{
namespace video
{


VertexFormatUniversal::VertexFormatUniversal() :
    VertexFormat(   ),
    FormatSize_ (0  )
{
}
VertexFormatUniversal::~VertexFormatUniversal()
{
}

u32 VertexFormatUniversal::getFormatSize() const
{
    return FormatSize_;
}

void VertexFormatUniversal::addCoord(const ERendererDataTypes Type, s32 Size)
{
    addAttribute(VERTEXFORMAT_COORD, Coord_, Size, Type, "POSITION", (Type == DATATYPE_FLOAT && Size == 3));
}
void VertexFormatUniversal::addColor(const ERendererDataTypes Type, s32 Size)
{
    addAttribute(VERTEXFORMAT_COLOR, Color_, Size, Type, "COLOR", (Type == DATATYPE_UNSIGNED_BYTE && Size == 4));
}
void VertexFormatUniversal::addNormal(const ERendererDataTypes Type)
{
    addAttribute(VERTEXFORMAT_NORMAL, Normal_, 3, Type, "NORMAL", (Type == DATATYPE_FLOAT));
}
void VertexFormatUniversal::addBinormal(const ERendererDataTypes Type)
{
    addAttribute(VERTEXFORMAT_BINORMAL, Binormal_, 3, Type, "BINORMAL", (Type == DATATYPE_FLOAT));
}
void VertexFormatUniversal::addTangent(const ERendererDataTypes Type)
{
    addAttribute(VERTEXFORMAT_TANGENT, Tangent_, 3, Type, "TANGENT", (Type == DATATYPE_FLOAT));
}
void VertexFormatUniversal::addTexCoord(const ERendererDataTypes Type, s32 Size)
{
    TexCoords_.resize(TexCoords_.size() + 1);
    addAttribute(VERTEXFORMAT_TEXCOORDS, TexCoords_[TexCoords_.size() - 1], Size, Type, "TEXCOORD" + io::stringc(TexCoords_.size() - 1));
}
void VertexFormatUniversal::addFogCoord(const ERendererDataTypes Type)
{
    addAttribute(VERTEXFORMAT_FOGCOORD, FogCoord_, 1, Type, "", (Type == DATATYPE_FLOAT));
}
void VertexFormatUniversal::addUniversal(
    const ERendererDataTypes Type, s32 Size, const io::stringc &Name, bool Normalize, const EVertexFormatFlags Attribute)
{
    Universals_.resize(Universals_.size() + 1);
    addAttribute(VERTEXFORMAT_UNIVERSAL, Universals_.back(), Size, Type, Name, false, Normalize);
    
    /* Link with special attribute */
    switch (Attribute)
    {
        case VERTEXFORMAT_COORD:
            addVirtualAttribute(Attribute, Coord_); break;
        case VERTEXFORMAT_COLOR:
            addVirtualAttribute(Attribute, Color_); break;
        case VERTEXFORMAT_NORMAL:
            addVirtualAttribute(Attribute, Normal_); break;
        case VERTEXFORMAT_BINORMAL:
            addVirtualAttribute(Attribute, Binormal_); break;
        case VERTEXFORMAT_TANGENT:
            addVirtualAttribute(Attribute, Tangent_); break;
        case VERTEXFORMAT_FOGCOORD:
            addVirtualAttribute(Attribute, FogCoord_); break;
        case VERTEXFORMAT_TEXCOORDS:
            TexCoords_.resize(TexCoords_.size() + 1);
            addVirtualAttribute(Attribute, TexCoords_[TexCoords_.size() - 1]); break;
        default:
            break;
    }
    
    updateConstruction();
}

void VertexFormatUniversal::remove(const EVertexFormatFlags Type)
{
    switch (Type)
    {
        case VERTEXFORMAT_TEXCOORDS:
            TexCoords_.pop_back();
            if (TexCoords_.empty())
                removeFlag(Type);
            break;
            
        case VERTEXFORMAT_UNIVERSAL:
            Universals_.pop_back();
            if (Universals_.empty())
                removeFlag(Type);
            break;
            
        default:
            removeFlag(Type);
            break;
    }
    
    updateConstruction();
}


/*
 * ======= Private: =======
 */

void VertexFormatUniversal::updateConstruction()
{
    constructFormat();
    VertexFormat::getFormatSize(FormatSize_);
}

void VertexFormatUniversal::addAttribute(
    const EVertexFormatFlags Flag, SVertexAttribute &Attrib, s32 Size, const ERendererDataTypes Type,
    const io::stringc &Name, bool hasDefaultSetting, bool Normalize)
{
    addFlag(Flag);
    Attrib = SVertexAttribute(Size, Name, Type, hasDefaultSetting, Normalize);
    updateConstruction();
}

void VertexFormatUniversal::addVirtualAttribute(const EVertexFormatFlags Attribute, SVertexAttribute &DestAttrib)
{
    DestAttrib = Universals_.back();
    DestAttrib.isReference = true;
    addFlag(Attribute);
}


} // /namespace video

} // /namespace sp



// ================================================================================
