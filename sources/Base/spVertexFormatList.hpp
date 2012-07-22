/*
 * Vertex format list header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_HWBUFFER_VERTEXFORMATLIST_H__
#define __SP_HWBUFFER_VERTEXFORMATLIST_H__


#include "Base/spStandard.hpp"
#include "Base/spVertexFormatUniversal.hpp"


namespace sp
{
namespace video
{


/**
Default predefined vertex format with the following attributes:
Coordinate (float[3]), Normal (float[3]), Color (unsigned char[4]), TexCoord[0 - 3] (float[2]).
*/
class SP_EXPORT VertexFormatDefault : public VertexFormat
{
    
    public:
        
        VertexFormatDefault();
        ~VertexFormatDefault();
        
        //! With this vertex format each vertex needs 60 bytes on VRAM.
        u32 getFormatSize() const;
        
};


/**
Reduced predefined vertex format with the following attributes:
Coordinate (float[3]), Normal (float[3]), TexCoord0 (float[2]).
*/
class SP_EXPORT VertexFormatReduced : public VertexFormat
{
    
    public:
        
        VertexFormatReduced();
        ~VertexFormatReduced();
        
        //! With this vertex format each vertex needs 32 bytes on VRAM.
        u32 getFormatSize() const;
        
};


/**
Extended predefined vertex format with the following attributes:
Coordinate (float[3]), Normal (float[3]), Binormal (float[3]), Tangent (float[3]), Color (unsigned char[4]), TexCoord[0 - 3] (float[3]).
*/
class SP_EXPORT VertexFormatExtended : public VertexFormat
{
    
    public:
        
        VertexFormatExtended();
        ~VertexFormatExtended();
        
        //! With this vertex format each vertex needs 100 bytes on VRAM.
        u32 getFormatSize() const;
        
};


/**
Full predefined vertex format with the following attributes:
Coordinate (float[3]), Normal (float[3]), Binormal (float[3]), Tangent (float[3]),
Color (unsigned char[4]), FogCoord (float), TexCoord[0 - 7] (float[3]).
*/
class SP_EXPORT VertexFormatFull : public VertexFormat
{
    
    public:
        
        VertexFormatFull();
        ~VertexFormatFull();
        
        //! With this vertex format each vertex needs 152 bytes on VRAM.
        u32 getFormatSize() const;
        
};


} // /namespace video

} // /namespace sp


#endif



// ================================================================================
