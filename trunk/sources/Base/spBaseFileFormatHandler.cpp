/*
 * Base file format handler file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "Base/spBaseFileFormatHandler.hpp"


namespace sp
{
namespace io
{


BaseFileFormatHandler::BaseFileFormatHandler(const io::stringc &Filename) :
    File_       (0          ),
    Filename_   (Filename   )
{
}
BaseFileFormatHandler::BaseFileFormatHandler(io::File* File) :
    File_       (File                               ),
    Filename_   (File_ ? File_->getFilename() : ""  )
{
}
BaseFileFormatHandler::~BaseFileFormatHandler()
{
}


/*
 * ======= Protected: =======
 */

bool BaseFileFormatHandler::openForReading()
{
    File_ = FileSys_.readResourceFile(Filename_);
    return File_ != 0;
}
bool BaseFileFormatHandler::openForWriting()
{
    File_ = FileSys_.openFile(Filename_, io::FILE_WRITE);
    return File_ != 0;
}


} // /namespace io

} // /namespace sp



// ================================================================================
