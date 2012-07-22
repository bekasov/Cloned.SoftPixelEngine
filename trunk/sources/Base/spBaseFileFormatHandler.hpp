/*
 * Base file format handler header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_BASEFILEFORMATHANDLER_H__
#define __SP_BASEFILEFORMATHANDLER_H__


#include "Base/spStandard.hpp"
#include "Base/spInputOutput.hpp"
#include "Base/spMemoryManagement.hpp"


namespace sp
{
namespace io
{


//! Base loader class. Base class for scene::MeshLoader, video::ImageLoader and sound::SoundLoader.
class SP_EXPORT BaseFileFormatHandler
{
    
    public:
        
        virtual ~BaseFileFormatHandler();
        
        /* Inline functions */
        
        inline io::stringc getFilename() const
        {
            return Filename_;
        }
        
    protected:
        
        BaseFileFormatHandler(const io::stringc &Filename = "");
        BaseFileFormatHandler(io::File* File);
        
        /* Functions */
        
        bool openForReading();
        bool openForWriting();
        
        /* Members */
        
        //todo -> change to only one io::File pointer given by the "load..." function
        io::FileSystem FileSys_;
        io::File* File_;
        
        io::stringc Filename_;
        
};


} // /namespace io

} // /namespace sp


#endif



// ================================================================================
