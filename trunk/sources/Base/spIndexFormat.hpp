/*
 * Index format header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_HWBUFFER_INDEXFORMAT_H__
#define __SP_HWBUFFER_INDEXFORMAT_H__


#include "Base/spStandard.hpp"
#include "Base/spVertexFormat.hpp"


namespace sp
{
namespace video
{


//! This is the index format class which specifies how indices are stored on the VRAM.
class SP_EXPORT IndexFormat
{
    
    public:
        
        IndexFormat();
        ~IndexFormat();
        
        /* Functions */
        
        void setDataType(const ERendererDataTypes Type);
        
        /* Inline functions */
        
        //! Returns the size in bytes of this index format.
        inline u32 getFormatSize() const
        {
            return FormatSize_;
        }
        
        //! Returns the data type of this index format.
        inline ERendererDataTypes getDataType() const
        {
            return Type_;
        }
        
    private:
        
        /* Members */
        
        s32 FormatSize_;
        ERendererDataTypes Type_;
        
};


} // /namespace video

} // /namespace sp


#endif



// ================================================================================
