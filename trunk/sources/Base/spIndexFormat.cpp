/*
 * Index format file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "Base/spIndexFormat.hpp"
#include "Base/spInputOutputLog.hpp"


namespace sp
{
namespace video
{


IndexFormat::IndexFormat() :
    FormatSize_ (4                      ),
    Type_       (DATATYPE_UNSIGNED_INT  )
{
}
IndexFormat::IndexFormat(const IndexFormat &Other) :
    FormatSize_ (Other.FormatSize_  ),
    Type_       (Other.Type_        )
{
}
IndexFormat::~IndexFormat()
{
}

void IndexFormat::setDataType(const ERendererDataTypes Type)
{
    switch (Type)
    {
        case DATATYPE_UNSIGNED_BYTE:
            Type_       = Type;
            FormatSize_ = 1;
            break;
        case DATATYPE_UNSIGNED_SHORT:
            Type_       = Type;
            FormatSize_ = 2;
            break;
        case DATATYPE_UNSIGNED_INT:
            Type_       = Type;
            FormatSize_ = 4;
            break;
        default:
            io::Log::warning("Mesh buffer index formats can only have the following attribute types: ubyte, ushort and uint");
            break;
    }
}


} // /namespace video

} // /namespace sp



// ================================================================================
