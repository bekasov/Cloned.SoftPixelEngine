/*
 * Asset container header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_ASSET_CONTAINER_H__
#define __SP_ASSET_CONTAINER_H__


#include "Base/spStandard.hpp"
#include "Base/spInputOutputFileSystem.hpp"

#include <list>
#include <map>


namespace sp
{
namespace io
{


//! \todo TO BE CONTINUED ...
class SP_EXPORT AssetContainer
{
    
    public:
        
        AssetContainer();
        ~AssetContainer();
        
        /* === Functions === */
        
        bool read(const io::stringc &AssetContainerFilename, const io::stringc &RootPath = "");
        bool write(const io::stringc &AssetContainerFilename, const io::stringc &RootPath = "");
        
        void addFile(const io::stringc &AssetFilename);
        io::File* getFile(io::FileSystem &FileSys, const io::stringc &AssetFilename);
        
        void setCipherKey(const io::stringc &Key);
        
        void clear();
        
        /* === Inline functions === */
        
        inline io::stringc getCipherKey() const
        {
            return CipherKey_;
        }
        
    private:
        
        /* === Macros === */
        
        static const u32 MAGIC_NUMBER;
        static const u16 VERSION_NUMBER;
        
        /* === Structures === */
        
        #if defined(_MSC_VER)
        #   pragma pack(push, packing)
        #   pragma pack(1)
        #   define SP_PACK_STRUCT
        #elif defined(__GNUC__)
        #   define SP_PACK_STRUCT __attribute__((packed))
        #else
        #   define SP_PACK_STRUCT
        #endif
        
        struct SHeader
        {
            SHeader() :
                Magic       (0),
                Version     (0),
                AssetCount  (0)
            {
            }
            ~SHeader()
            {
            }
            
            /* Members */
            u32 Magic;      //!< "SPAC".
            u16 Version;    //!< 100 -> v.1.0.
            u32 AssetCount; //!< Count of assets.
        }
        SP_PACK_STRUCT;
        
        #ifdef _MSC_VER
        #   pragma pack(pop, packing)
        #endif
        
        #undef SP_PACK_STRUCT
        
        struct SAsset
        {
            SAsset() :
                Size    (0),
                Offset  (0),
                FileObj (0)
            {
            }
            ~SAsset()
            {
            }
            
            /* Members */
            io::stringc Filename;
            u32 Size;
            u32 Offset;
            io::File* FileObj;
        };
        
        /* === Members === */
        
        SHeader Header_;
        
        io::FileSystem FileSys_;
        io::File* AssetFile_;
        
        io::stringc CipherKey_;
        io::stringc RootPath_;
        
        std::list<SAsset> AssetsList_;
        std::map<std::string, SAsset*> AssetsMap_;
        
};


} // /namespace io

} // /namespace sp


#endif



// ================================================================================
