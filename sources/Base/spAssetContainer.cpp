/*
 * Asset container file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "Base/spAssetContainer.hpp"


namespace sp
{
namespace io
{


const u32 AssetContainer::MAGIC_NUMBER      = *(u32*)"SPAC";
const u16 AssetContainer::VERSION_NUMBER    = 100;

AssetContainer::AssetContainer()
{
}
AssetContainer::~AssetContainer()
{
}

bool AssetContainer::read(const io::stringc &AssetContainerFilename, const io::stringc &RootPath)
{
    clear();
    
    io::Log::message("Read asset container");
    io::Log::ScopedTab AutoTab;
    
    /* Open archive file */
    AssetFile_ = FileSys_.openFile(AssetContainerFilename, io::FILE_READ);
    
    if (!AssetFile_)
        return false;
    
    /* Read header */
    AssetFile_->readBuffer(&Header_, sizeof(SHeader));
    
    if (Header_.Magic != AssetContainer::MAGIC_NUMBER)
    {
        io::Log::error("Invalid magic number in asset container");
        return false;
    }
    
    if (Header_.Version != AssetContainer::VERSION_NUMBER)
    {
        io::Log::error("Unsupported version in asset container");
        return false;
    }
    
    //TODO ...
    
    return true;
}

bool AssetContainer::write(const io::stringc &AssetContainerFilename, const io::stringc &RootPath)
{
    io::Log::message("Write asset container");
    io::Log::ScopedTab AutoTab;
    
    /* Open archive file */
    FileSys_.closeFile(AssetFile_);
    AssetFile_ = FileSys_.openFile(AssetContainerFilename, io::FILE_WRITE);
    
    if (!AssetFile_)
        return false;
    
    //TODO ...
    
    return false;
}

void AssetContainer::addFile(const io::stringc &AssetFilename)
{
    
}

io::File* AssetContainer::getFile(io::FileSystem &FileSys, const io::stringc &AssetFilename)
{
    return 0;
}

void AssetContainer::setCipherKey(const io::stringc &Key)
{
    CipherKey_ = Key;
}

void AssetContainer::clear()
{
    Header_.AssetCount = 0;
    AssetsList_.clear();
    AssetsMap_.clear();
}


} // /namespace io

} // /namespace sp



// ================================================================================
