/*
 * File asset header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_INPUTOUTPUT_FILE_ASSET_H__
#define __SP_INPUTOUTPUT_FILE_ASSET_H__


#include "Base/spStandard.hpp"

#if defined(SP_PLATFORM_ANDROID)


#include "Base/spInputOutputFile.hpp"

#include <android/asset_manager.h>


namespace sp
{

class SoftPixelDeviceAndroid;

namespace audio { class OpenSLESSound; }

namespace io
{


//! Asset files are used for reading resource files on Android. Writing is not allowed!
class SP_EXPORT FileAsset : public File
{
    
    public:
        
        FileAsset();
        ~FileAsset();
        
        /* Functions */
        
        bool open(const io::stringc &Filename, const EFilePermission Permission = FILE_READWRITE);
        void close();
        
        s32 writeBuffer(const void* Buffer, u32 Size, u32 Count = 1);
        s32 readBuffer(void* Buffer, u32 Size, u32 Count = 1) const;
        
        void setSeek(s32 Pos, const EFileSeekTypes PosType = FILEPOS_BEGIN);
        s32 getSeek() const;
        
        bool isEOF() const;
        
        u32 getSize() const;
        void* getHandle();
        
        bool opened() const;
        
        /* Static functions */
        
        static bool findFile(const io::stringc &Filename);
        
    private:
        
        friend class sp::SoftPixelDeviceAndroid;
        friend class sp::audio::OpenSLESSound;
        
        /* Members */
        
        AAsset* Asset_;
        mutable s32 Pos_;
        
        static AAssetManager* AssetManager_;
        
};


} // /namespace io

} // /namespace sp


#endif

#endif



// ================================================================================
