/*
 * Movie file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "RenderSystem/spRenderSystemMovie.hpp"
#include "RenderSystem/spMovieSequencePacket.hpp"
#include "Platform/spSoftPixelDeviceOS.hpp"

#include <time.h>


namespace sp
{

extern video::RenderSystem* __spVideoDriver;

namespace video
{


#if defined(SP_PLATFORM_WINDOWS)

#define mcrAVIData ((SMovieSequencePacket*)DataPointer_)

Movie::Movie(const io::stringc &Filename, const s32 Resolution)
    : DataPointer_(0), Filename_(Filename), Speed_(1.0f)
{
    reload(Filename, Resolution);
}
Movie::~Movie()
{
    close();
}

bool Movie::reload(const io::stringc &Filename, const s32 Resolution)
{
    if (DataPointer_)
        close();
    
    /* Information message */
    io::Log::message("Load movie: \"" + Filename + "\"");
    io::Log::upperTab();
    
    /* General settings */
    SMovieSequencePacket* MovieData = new SMovieSequencePacket;
    
    DataPointer_                = MovieData;
    
    MovieData->Next             = MovieData->Frame      = 0;
    MovieData->State            = MOVIESTATE_STOPED;
    MovieData->Time             = MovieData->LastTime   = 0;
    MovieData->RawData          = 0;
    MovieData->Resolution       = Resolution;
    
    MovieData->hDeviceContext   = CreateCompatibleDC(0);
    MovieData->hDrawDIB         = DrawDibOpen();
    
    AVIFileInit();
    
    /* Open video stream */
    if (AVIStreamOpenFromFile(&MovieData->pVideoStream, Filename.c_str(), streamtypeVIDEO, 0, OF_READ, 0) != 0)
        return exitWithError("Could not open video stream");
    if (AVIStreamInfo(MovieData->pVideoStream, &MovieData->VideoStreamInfo, sizeof(MovieData->VideoStreamInfo)) != 0)
        return exitWithError("Video stream information process failed");
    
    MovieData->VideoLastFrame = AVIStreamLength(MovieData->pVideoStream);
    
    if (MovieData->VideoLastFrame == -1)
        return exitWithError("Video stream length is invalid");
    
    MovieData->VideoMPF = AVIStreamSampleToTime(MovieData->pVideoStream, MovieData->VideoLastFrame) / MovieData->VideoLastFrame;
    
    if (MovieData->VideoMPF == -1)
        return exitWithError("Video stream sample is invalid");
    
    #if 0
    
    /* Open audio stream */
    if (AVIStreamOpenFromFile(&MovieData->pAudioStream, Filename.c_str(), streamtypeAUDIO, 0, OF_READ, 0))
        return exitWithError("Could not open audio stream");
    if (AVIStreamInfo(MovieData->pAudioStream, &MovieData->AudioStreamInfo, sizeof(MovieData->AudioStreamInfo)))
        return exitWithError("Audio stream information process failed");
    
    MovieData->AudioLastFrame = AVIStreamLength(MovieData->pAudioStream);
    
    if (MovieData->AudioLastFrame == -1)
        return exitWithError("Audio stream length is invalid");
    
    MovieData->AudioMPF = AVIStreamSampleToTime(MovieData->pAudioStream, MovieData->AudioLastFrame) / MovieData->AudioLastFrame;
    
    if (MovieData->AudioMPF == -1)
        return exitWithError("Audio stream sample is invalid");
    
    #endif
    
    /* Start configuration process */
    MovieData->Size.Width   = MovieData->VideoStreamInfo.rcFrame.right  - MovieData->VideoStreamInfo.rcFrame.left;
    MovieData->Size.Height  = MovieData->VideoStreamInfo.rcFrame.bottom - MovieData->VideoStreamInfo.rcFrame.top;
    
    MovieData->BitmapInfoHeader.biSize          = sizeof(BITMAPINFOHEADER);
    MovieData->BitmapInfoHeader.biPlanes        = 1;
    MovieData->BitmapInfoHeader.biBitCount      = 24;
    MovieData->BitmapInfoHeader.biWidth         = MovieData->Resolution;
    MovieData->BitmapInfoHeader.biHeight        = MovieData->Resolution;
    MovieData->BitmapInfoHeader.biCompression   = BI_RGB;
    
    /* Create the movie bitmap context */
    MovieData->hBitmap = CreateDIBSection(
        MovieData->hDeviceContext,
        (BITMAPINFO*)(&MovieData->BitmapInfoHeader),
        DIB_RGB_COLORS,
        (void**)(&MovieData->RawData),
        0, 0
    );

    if (!MovieData->hBitmap)
        return exitWithError("Could not create device independent bitmap (DIB) for video stream");
    
    SelectObject(MovieData->hDeviceContext, MovieData->hBitmap);
    
    MovieData->BitmapInfoHeader.biWidth         = MovieData->Size.Width;
    MovieData->BitmapInfoHeader.biHeight        = MovieData->Size.Height;
    MovieData->BitmapInfoHeader.biSizeImage     = 0;
    MovieData->BitmapInfoHeader.biClrUsed       = 0;
    MovieData->BitmapInfoHeader.biClrImportant  = 0;
    
    /* Get first video frame */
    MovieData->pGetFrame = AVIStreamGetFrameOpen(MovieData->pVideoStream, &MovieData->BitmapInfoHeader);
    
    if (!MovieData->pGetFrame)
        return exitWithError("Could not open first video stream");
    
    io::Log::lowerTab();
    
    return true;
}

void Movie::close()
{
    if (DataPointer_)
    {
        /* Delete each movie object data */
        DeleteObject(mcrAVIData->hBitmap);
        DrawDibClose(mcrAVIData->hDrawDIB);
        
        AVIStreamGetFrameClose(mcrAVIData->pGetFrame);
        AVIStreamRelease(mcrAVIData->pVideoStream);
        AVIFileExit();
        
        /* Delete the movie data */
        delete mcrAVIData;
    }
}


void Movie::play(bool Looped)
{
    mcrAVIData->State   = MOVIESTATE_PLAYED;
    mcrAVIData->Time    = mcrAVIData->LastTime = clock();
    mcrAVIData->Looped  = Looped;
    
    #if 0
    if (AVIStreamStart(mcrAVIData->pAudioStream) == -1)
        io::printError("Could not start audio stream");
    #endif
}

void Movie::pause(bool Paused)
{
    if (Paused && mcrAVIData->State == MOVIESTATE_PLAYED)
    {
        mcrAVIData->State       = MOVIESTATE_PAUSED;
    }
    else if (!Paused && mcrAVIData->State == MOVIESTATE_PAUSED)
    {
        mcrAVIData->State       = MOVIESTATE_PLAYED;
        mcrAVIData->LastTime    = clock();
    }
}

void Movie::stop()
{
    mcrAVIData->State = MOVIESTATE_STOPED;
}

void Movie::setSeek(f32 Seek)
{
    mcrAVIData->Next = (s32)(Seek * mcrAVIData->VideoLastFrame * mcrAVIData->VideoMPF);
}
f32 Movie::getSeek() const
{
    return (f32)mcrAVIData->Frame / mcrAVIData->VideoLastFrame;
}

void Movie::setSpeed(f32 Speed)
{
    Speed_ = Speed;
}
f32 Movie::getSpeed() const
{
    return Speed_;
}

dim::size2di Movie::getSize() const
{
    return mcrAVIData->Size;
}
u32 Movie::getLength() const
{
    return mcrAVIData->VideoLastFrame;
}
bool Movie::finish() const
{
    return mcrAVIData->Frame >= mcrAVIData->VideoLastFrame;
}
bool Movie::valid() const
{
    return DataPointer_ != 0;
}

void Movie::renderTexture(Texture* Tex)
{
    if (!Tex || Tex->getImageBuffer()->getType() != IMAGEBUFFER_UBYTE || mcrAVIData->State != MOVIESTATE_PLAYED)
        return;
    
    /* Proceed to the next frame */
    mcrAVIData->Time        = clock();
    mcrAVIData->Next        += (s32)( Speed_ * (mcrAVIData->Time - mcrAVIData->LastTime) );
    mcrAVIData->LastTime    = clock();
    mcrAVIData->Frame       = mcrAVIData->Next / mcrAVIData->VideoMPF;
    
    /* Check if the movie is finish */
    if (finish())
    {
        mcrAVIData->Next = mcrAVIData->Frame = 0;
        
        if (!mcrAVIData->Looped)
        {
            mcrAVIData->State = MOVIESTATE_STOPED;
            return;
        }
    }
    
    /* Get frame's video stream */
    LPBITMAPINFOHEADER pBitmapInfoHeader = (LPBITMAPINFOHEADER)AVIStreamGetFrame(mcrAVIData->pGetFrame, mcrAVIData->Frame);
    
    if (!pBitmapInfoHeader)
        return;
    
    mcrAVIData->VideoData = (s8*)pBitmapInfoHeader + pBitmapInfoHeader->biSize + pBitmapInfoHeader->biClrUsed * sizeof(RGBQUAD);
    
    DrawDibDraw(
        mcrAVIData->hDrawDIB, mcrAVIData->hDeviceContext,
        0, 0,
        mcrAVIData->Resolution, mcrAVIData->Resolution,
        pBitmapInfoHeader, mcrAVIData->VideoData,
        0, 0,
        mcrAVIData->Size.Width, mcrAVIData->Size.Height,
        0
    );
    
    //flipDataBuffer(MovieData->RawData, MovieData->Resolution * MovieData->Resolution * 3);
    
    /* Copy to texture */
    if (Tex->getSize() == dim::size2di(mcrAVIData->Resolution) && Tex->getImageBuffer()->getFormatSize() == 3)
        Tex->setupImageBuffer(mcrAVIData->RawData);
}

#else

Movie::Movie(const io::stringc &Filename, const s32 Resolution)
    : DataPointer_(0), Filename_(Filename), Speed_(1.0f)
{
}
Movie::~Movie()
{
}

bool Movie::reload(const io::stringc &Filename, const s32 Resolution)
{
    io::Log::warning("Loading movies is not supported under GNU/Linux yet");
    return false;
}
void Movie::close()
{
}

void Movie::play(bool Looped)
{
}
void Movie::pause(bool Paused)
{
}
void Movie::stop()
{
}

void Movie::setSeek(f32 Seek)
{
}
f32 Movie::getSeek() const
{
    return 0.0f;
}

void Movie::setSpeed(f32 Speed)
{
    Speed_ = Speed;
}
f32 Movie::getSpeed() const
{
    return Speed_;
}

dim::size2di Movie::getSize() const
{
    return dim::size2di(0);
}
u32 Movie::getLength() const
{
    return 0;
}
bool Movie::finish() const
{
    return true;
}
bool Movie::valid() const
{
    return DataPointer_ != 0;
}

void Movie::renderTexture(Texture* hTexture)
{
    // do nothing, no movie-texture-render support for linux
    // -> vfw lib is only available for windows
}

#endif

void Movie::flipDataBuffer(void* buffer, u32 Len)
{
    register u8* Buf = static_cast<u8*>(buffer);
    register u8* End = Buf + Len;
    register u8 Tmp;
    
    while (Buf < End)
    {
        Tmp     = Buf[0];
        Buf[0]  = Buf[2];
        Buf[2]  = Tmp;
        Buf += 3;
    }
}

bool Movie::exitWithError(const io::stringc &ErrorMessage)
{
    io::Log::error(ErrorMessage);
    io::Log::lowerTab();
    return false;
}


} // /namespace video

} // /namespace sp



// ================================================================================
