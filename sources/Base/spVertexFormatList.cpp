/*
 * Vertex format list file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "Base/spVertexFormatList.hpp"


namespace sp
{
namespace video
{


/* === VertexFormatDefault === */

VertexFormatDefault::VertexFormatDefault() :
    VertexFormat(
        "Default",
        VERTEXFORMAT_COORD | VERTEXFORMAT_COLOR | VERTEXFORMAT_NORMAL | VERTEXFORMAT_TEXCOORDS
    )
{
    Coord_  = SVertexAttribute(3, "POSITION");
    Normal_ = SVertexAttribute(3, "NORMAL");
    Color_  = SVertexAttribute(4, "COLOR", DATATYPE_UNSIGNED_BYTE, true, true);
    
    TexCoords_.push_back(SVertexAttribute(2, "TEXCOORD0"));
    TexCoords_.push_back(SVertexAttribute(2, "TEXCOORD1"));
    TexCoords_.push_back(SVertexAttribute(2, "TEXCOORD2"));
    TexCoords_.push_back(SVertexAttribute(2, "TEXCOORD3"));
    
    constructFormat();
}
VertexFormatDefault::~VertexFormatDefault()
{
}

u32 VertexFormatDefault::getFormatSize() const
{
    return 60;
}


/* === VertexFormatReduced === */

VertexFormatReduced::VertexFormatReduced() :
    VertexFormat(
        "Reduced",
        VERTEXFORMAT_COORD | VERTEXFORMAT_NORMAL | VERTEXFORMAT_TEXCOORDS
    )
{
    Coord_  = SVertexAttribute(3, "POSITION");
    Normal_ = SVertexAttribute(3, "NORMAL");
    
    TexCoords_.push_back(SVertexAttribute(2, "TEXCOORD0"));
    
    constructFormat();
}
VertexFormatReduced::~VertexFormatReduced()
{
}

u32 VertexFormatReduced::getFormatSize() const
{
    return 32;
}


/* === VertexFormatExtended === */

VertexFormatExtended::VertexFormatExtended() :
    VertexFormat(
        "Extended",
        VERTEXFORMAT_COORD | VERTEXFORMAT_COLOR | VERTEXFORMAT_NORMAL | VERTEXFORMAT_BINORMAL | VERTEXFORMAT_TANGENT | VERTEXFORMAT_TEXCOORDS
    )
{
    Coord_  = SVertexAttribute(3, "POSITION");
    Normal_ = SVertexAttribute(3, "NORMAL");
    Color_  = SVertexAttribute(4, "COLOR", DATATYPE_UNSIGNED_BYTE, true, true);
    
    TexCoords_.push_back(SVertexAttribute(3, "TEXCOORD0"));
    TexCoords_.push_back(SVertexAttribute(3, "TEXCOORD1"));
    TexCoords_.push_back(SVertexAttribute(3, "TEXCOORD2"));
    TexCoords_.push_back(SVertexAttribute(3, "TEXCOORD3"));
    
    Binormal_   = SVertexAttribute(3, "BINORMAL");
    Tangent_    = SVertexAttribute(3, "TANGENT");
    
    constructFormat();
}
VertexFormatExtended::~VertexFormatExtended()
{
}

u32 VertexFormatExtended::getFormatSize() const
{
    return 100;
}


/* === VertexFormatFull === */

VertexFormatFull::VertexFormatFull() :
    VertexFormat(
        "Full",
        VERTEXFORMAT_COORD | VERTEXFORMAT_COLOR | VERTEXFORMAT_NORMAL | VERTEXFORMAT_BINORMAL |
            VERTEXFORMAT_TANGENT | VERTEXFORMAT_TEXCOORDS | VERTEXFORMAT_FOGCOORD
    )
{
    Coord_  = SVertexAttribute(3, "POSITION");
    Normal_ = SVertexAttribute(3, "NORMAL");
    Color_  = SVertexAttribute(4, "COLOR", DATATYPE_UNSIGNED_BYTE, true, true);
    
    for (s32 i = 0; i < 8; ++i)
        TexCoords_.push_back(SVertexAttribute(3, "TEXCOORD" + io::stringc(i)));
    
    FogCoord_   = SVertexAttribute(1, "BLENDWEIGHT");
    Binormal_   = SVertexAttribute(3, "BINORMAL");
    Tangent_    = SVertexAttribute(3, "TANGENT");
    
    constructFormat();
}
VertexFormatFull::~VertexFormatFull()
{
}

u32 VertexFormatFull::getFormatSize() const
{
    return 152;
}


} // /namespace video

} // /namespace sp



// ================================================================================
