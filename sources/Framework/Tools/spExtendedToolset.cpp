/*
 * Extended toolset file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "Framework/Tools/spExtendedToolset.hpp"
#include "FileFormats/Sound/spSoundLoader.hpp"
#include "Platform/spSoftPixelDeviceOS.hpp"
#include "Base/spSharedObjects.hpp"


namespace sp
{

extern SoftPixelDevice* __spDevice;
extern io::InputControl* __spInputControl;
extern scene::SceneGraph* __spSceneManager;
extern video::RenderSystem* __spVideoDriver;

namespace tool
{


namespace Toolset
{

#if defined(SP_PLATFORM_ANDROID) || defined(SP_PLATFORM_IOS)

static bool spMobileUIDrawCtrl(
    const dim::point2di &Position, s32 SizeInner, s32 SizeOuter, dim::point2df &Offset,
    s32 &Picked, const video::color &Color = video::color(64, 128, 255))
{
    /* General settings */
    bool Result = false;
    Offset = 0.0f;
    
    const dim::rect2di Rect(
        Position.X - SizeOuter, Position.Y - SizeOuter, Position.X + SizeOuter, Position.Y + SizeOuter
    );
    
    bool PickedMem[5] = { 0 };
    
    /* Update user input */
    for (s32 i = 0; i < 5; ++i)
    {
        if (!__spInputControl->isPointerActive(i))
            continue;
        
        const dim::point2di Pos(__spInputControl->getPointerPosition(i).cast<s32>());
        
        if (Rect.isPointCollided(Pos))// || Picked == i)
        {
            /* Apply controller translation */
            Result = true;
            PickedMem[i] = true;
            Picked = i;
            
            Offset = (Pos - Position).cast<f32>();
            
            /* Clamp controller position */
            math::Clamp(Offset.X, static_cast<f32>(SizeInner - SizeOuter), static_cast<f32>(SizeOuter - SizeInner));
            math::Clamp(Offset.Y, static_cast<f32>(SizeInner - SizeOuter), static_cast<f32>(SizeOuter - SizeInner));
            
            break;
        }
    }
    
    if (Picked >= 0 && !PickedMem[Picked])
        Picked = -1;
    
    /* Draw controller */
    const dim::point2di CtrlPosition(
        Position.X + static_cast<s32>(Offset.X),
        Position.Y + static_cast<s32>(Offset.Y)
    );
    
    __spVideoDriver->draw2DRectangle(Rect, video::color(Color.Red, Color.Green, Color.Blue, 128));
    __spVideoDriver->draw2DRectangle(
        dim::rect2di(CtrlPosition.X - SizeInner, CtrlPosition.Y - SizeInner, CtrlPosition.X + SizeInner, CtrlPosition.Y + SizeInner),
        //Color
        (Picked == 0 ? video::color(255, 0, 0) : (Picked == 1 ? video::color(0, 255, 0) : Color))
    );
    
    return Result;
}

SP_EXPORT void moveCameraFree(
    scene::Camera* Cam, const f32 MoveSpeed, const f32 TurnSpeed, const f32 MaxTurnDegree, bool UseArrowKeys)
{
    /* Static variables */
    static f32 Pitch, Yaw;
    static s32 PickedMove = -1, PickedTurn = -1;
    dim::point2df CtrlVec;
    
    /* Check for default camera usage */
    if (!Cam && !( Cam = __spSceneManager->getActiveCamera() ) )
        return;
    
    /* Draw controller in 2D */
    __spVideoDriver->beginDrawing2D();
    
    const dim::size2di ScrSize(__spDevice->getResolution());
    
    /* Control translation movement */
    if (spMobileUIDrawCtrl(dim::point2di(100, ScrSize.Height - 100), 35, 75, CtrlVec, PickedMove))
        Cam->move(dim::vector3df(CtrlVec.X, 0, -CtrlVec.Y) * MoveSpeed * 0.01f);
    
    /* Control rotation movement */
    if (spMobileUIDrawCtrl(dim::point2di(ScrSize.Width - 100, ScrSize.Height - 100), 35, 75, CtrlVec, PickedTurn))
    {
        Pitch   += CtrlVec.Y * TurnSpeed * 0.1f;
        Yaw     += CtrlVec.X * TurnSpeed * 0.1f;
    }
    
    if (!math::Equal(MaxTurnDegree, 0.0f))
    {
        if (Pitch > MaxTurnDegree)
            Pitch = MaxTurnDegree;
        else if (Pitch < -MaxTurnDegree)
            Pitch = -MaxTurnDegree;
    }
    
    Cam->setRotation(dim::vector3df(Pitch, Yaw, 0));
    
    __spVideoDriver->endDrawing2D();
}

#else

SP_EXPORT void moveCameraFree(
    scene::Camera* Cam, const f32 MoveSpeed, const f32 TurnSpeed, const f32 MaxTurnDegree, bool UseArrowKeys)
{
    /* Static variables */
    static f32 Pitch, Yaw;
    
    /* Check for default camera usage */
    if (!Cam && !( Cam = __spSceneManager->getActiveCamera() ) )
        return;
    
    /* Control translation movement */
    if ( ( UseArrowKeys && __spInputControl->keyDown(io::KEY_RIGHT) ) || __spInputControl->keyDown(io::KEY_D) )
        Cam->move(dim::vector3df(MoveSpeed, 0, 0));
    if ( ( UseArrowKeys && __spInputControl->keyDown(io::KEY_LEFT) ) || __spInputControl->keyDown(io::KEY_A) )
        Cam->move(dim::vector3df(-MoveSpeed, 0, 0));
    if ( ( UseArrowKeys && __spInputControl->keyDown(io::KEY_UP) ) || __spInputControl->keyDown(io::KEY_W) )
        Cam->move(dim::vector3df(0, 0, MoveSpeed));
    if ( ( UseArrowKeys && __spInputControl->keyDown(io::KEY_DOWN) ) || __spInputControl->keyDown(io::KEY_S) )
        Cam->move(dim::vector3df(0, 0, -MoveSpeed));
    
    /* Control rotation movement */
    Pitch += static_cast<f32>(__spInputControl->getCursorSpeed().Y) * TurnSpeed;
    Yaw   += static_cast<f32>(__spInputControl->getCursorSpeed().X) * TurnSpeed;
    
    if (!math::Equal(MaxTurnDegree, 0.0f))
    {
        if (Pitch > MaxTurnDegree)
            Pitch = MaxTurnDegree;
        else if (Pitch < -MaxTurnDegree)
            Pitch = -MaxTurnDegree;
    }
    
    Cam->setRotation(dim::vector3df(Pitch, Yaw, 0));
    
    __spInputControl->setCursorPosition(dim::point2di(gSharedObjects.ScreenWidth/2, gSharedObjects.ScreenHeight/2));
}

#endif

} // /namespace Toolset


namespace Debugging
{

SP_EXPORT io::stringc toString(const dim::vector3df &Value)
{
    return "( " + io::stringc(Value.X) + " | " + io::stringc(Value.Y) + " | " + io::stringc(Value.Z) + " )";
}

SP_EXPORT io::stringc toString(const dim::quaternion &Value)
{
    return "( " + io::stringc(Value.X) + " | " + io::stringc(Value.Y) + " | " + io::stringc(Value.Z) + " | " + io::stringc(Value.W) + " )";
}

SP_EXPORT io::stringc toString(const dim::matrix4f &Value)
{
    return
        "[ " + io::stringc(Value[0]) + "\t" + io::stringc(Value[4]) + "\t" + io::stringc(Value[ 8]) + "\t" + io::stringc(Value[12]) + " ]\n" +
        "[ " + io::stringc(Value[1]) + "\t" + io::stringc(Value[5]) + "\t" + io::stringc(Value[ 9]) + "\t" + io::stringc(Value[13]) + " ]\n" +
        "[ " + io::stringc(Value[2]) + "\t" + io::stringc(Value[6]) + "\t" + io::stringc(Value[10]) + "\t" + io::stringc(Value[14]) + " ]\n" +
        "[ " + io::stringc(Value[3]) + "\t" + io::stringc(Value[7]) + "\t" + io::stringc(Value[11]) + "\t" + io::stringc(Value[15]) + " ]";
}

SP_EXPORT io::stringc toString(const dim::matrix2f &Value)
{
    return
        "[ " + io::stringc(Value[0]) + "\t" + io::stringc(Value[2]) + " ]\n" +
        "[ " + io::stringc(Value[1]) + "\t" + io::stringc(Value[3]) + " ]";
}

SP_EXPORT io::stringc toString(const video::color &Value)
{
    return
         "( " + io::stringc(static_cast<s32>(Value.Red  )) +
        " | " + io::stringc(static_cast<s32>(Value.Green)) +
        " | " + io::stringc(static_cast<s32>(Value.Blue )) +
        " | " + io::stringc(static_cast<s32>(Value.Alpha)) +
        " )";
}

SP_EXPORT io::stringc toString(const video::EPixelFormats PixelFormat)
{
    switch (PixelFormat)
    {
        case video::PIXELFORMAT_ALPHA:      return "Alpha";
        case video::PIXELFORMAT_GRAY:       return "Gray";
        case video::PIXELFORMAT_GRAYALPHA:  return "Gray Alpha";
        case video::PIXELFORMAT_RGB:        return "RGB";
        case video::PIXELFORMAT_BGR:        return "BGR";
        case video::PIXELFORMAT_RGBA:       return "RGBA";
        case video::PIXELFORMAT_BGRA:       return "BGRA";
        case video::PIXELFORMAT_DEPTH:      return "Depth";
    }
    return "";
}

SP_EXPORT io::stringc toString(const video::EImageFileFormats ImageFileFormat)
{
    switch (ImageFileFormat)
    {
        case video::IMAGEFORMAT_UNKNOWN:    return "Unknown";
        case video::IMAGEFORMAT_BMP:        return "BMP";
        case video::IMAGEFORMAT_JPG:        return "JPG";
        case video::IMAGEFORMAT_PNG:        return "PNG";
        case video::IMAGEFORMAT_TGA:        return "TGA";
        case video::IMAGEFORMAT_PCX:        return "PCX";
        case video::IMAGEFORMAT_DDS:        return "DDS";
        case video::IMAGEFORMAT_WAD:        return "WAD";
        case video::IMAGEFORMAT_GIF:        return "GIF";
    }
    return "";
}

SP_EXPORT io::stringc toString(const video::ETextureFilters TextureFilter)
{
    switch (TextureFilter)
    {
        case video::FILTER_LINEAR: return "Linear";
        case video::FILTER_SMOOTH: return "Smooth";
    }
    return "";
}

SP_EXPORT io::stringc toString(const video::ETextureMipMapFilters TextureMipMapFilter)
{
    switch (TextureMipMapFilter)
    {
        case video::FILTER_BILINEAR:    return "Bi-Linear";
        case video::FILTER_TRILINEAR:   return "Tri-Linear";
        case video::FILTER_ANISOTROPIC: return "Anisotropic";
    }
    return "";
}

SP_EXPORT io::stringc toString(const video::ETextureWrapModes TextureWrapMode)
{
    switch (TextureWrapMode)
    {
        case video::TEXWRAP_REPEAT: return "Repeat";
        case video::TEXWRAP_MIRROR: return "Mirror";
        case video::TEXWRAP_CLAMP:  return "Clamp";
    }
    return "";
}

SP_EXPORT io::stringc toString(const video::ETextureDimensions TextureDimension)
{
    switch (TextureDimension)
    {
        case video::TEXTURE_1D:             return "1D Texture";
        case video::TEXTURE_2D:             return "2D Texture";
        case video::TEXTURE_3D:             return "3D Texture";
        case video::TEXTURE_CUBEMAP:        return "Cube Texture";
        case video::TEXTURE_1D_ARRAY:       return "1D Texture Array";
        case video::TEXTURE_2D_ARRAY:       return "2D Texture Array";
        case video::TEXTURE_CUBEMAP_ARRAY:  return "Cube Texture Array";
    }
    return "";
}

SP_EXPORT io::stringc toString(const video::EHWTextureFormats HWTextureFormat)
{
    switch (HWTextureFormat)
    {
        case video::HWTEXFORMAT_UBYTE8:     return "8 Bit Unsigned Byte";
        case video::HWTEXFORMAT_FLOAT16:    return "16 Bit Floating Point";
        case video::HWTEXFORMAT_FLOAT32:    return "32 Bit Floating Point";
    }
    return "";
}

SP_EXPORT io::stringc toString(const video::ECubeMapDirections CubeMapDirection)
{
    switch (CubeMapDirection)
    {
        case video::CUBEMAP_POSITIVE_X: return "Positive X";
        case video::CUBEMAP_NEGATIVE_X: return "Negative X";
        case video::CUBEMAP_POSITIVE_Y: return "Positive Y";
        case video::CUBEMAP_NEGATIVE_Y: return "Negative Y";
        case video::CUBEMAP_POSITIVE_Z: return "Positive Z";
        case video::CUBEMAP_NEGATIVE_Z: return "Negative Z";
    }
    return "";
}

SP_EXPORT io::stringc toString(const video::EAlphaBlendingTypes AlphaBlendingType)
{
    switch (AlphaBlendingType)
    {
        case video::BLENDING_BRIGHT:    return "Bright Blending";
        case video::BLENDING_DARK:      return "Dark Blending";
    }
    return "";
}

SP_EXPORT io::stringc toString(const video::EImageBufferTypes ImageBufferType)
{
    switch (ImageBufferType)
    {
        case video::IMAGEBUFFER_UBYTE: return "Unsigned Byte";
        case video::IMAGEBUFFER_FLOAT: return "Floating Point";
    }
    return "";
}

SP_EXPORT io::stringc toString(const video::ERendererDataTypes RendererDataType)
{
    switch (RendererDataType)
    {
        case video::DATATYPE_FLOAT:             return "Float";
        case video::DATATYPE_DOUBLE:            return "Double";
        case video::DATATYPE_BYTE:              return "Byte";
        case video::DATATYPE_SHORT:             return "Short";
        case video::DATATYPE_INT:               return "Integer";
        case video::DATATYPE_UNSIGNED_BYTE:     return "Unsigned Byte";
        case video::DATATYPE_UNSIGNED_SHORT:    return "Unsigned Short";
        case video::DATATYPE_UNSIGNED_INT:      return "Unsigned Integer";
    }
    return "";
}

SP_EXPORT io::stringc toString(const audio::EWaveBufferFormats WaveFormat)
{
    switch (WaveFormat)
    {
        case audio::WAVEFORMAT_PCM:                     return "PCM";
        case audio::WAVEFORMAT_MS_ADPCM:                return "MS ADPCM";
        case audio::WAVEFORMAT_IEEE_FLOAT:              return "IEEE FLOAT";
        case audio::WAVEFORMAT_IBM_CVSD:                return "IBM CVSD";
        case audio::WAVEFORMAT_ALAW:                    return "ALAW";
        case audio::WAVEFORMAT_MULAW:                   return "MULAW";
        case audio::WAVEFORMAT_OKI_ADPCM:               return "OKI ADPCM";
        case audio::WAVEFORMAT_DVI_IMA_ADPCM:           return "DVI IMA DPCM";
        case audio::WAVEFORMAT_MEDIASPACE_ADPCM:        return "MEDIASPACE ADPCM";
        case audio::WAVEFORMAT_SIERRA_ADPCM:            return "SIERRA ADPCM";
        case audio::WAVEFORMAT_G723_ADPCM:              return "G723 ADPCM";
        case audio::WAVEFORMAT_DIGISTD:                 return "DIGISTD";
        case audio::WAVEFORMAT_DIGIFIX:                 return "DIGIFIX";
        case audio::WAVEFORMAT_DIALOGIC_OKI_ADPCM:      return "DIALOGIC OKI ADPCM";
        case audio::WAVEFORMAT_YAMAHA_ADPCM:            return "YAMAHA ADPCM";
        case audio::WAVEFORMAT_SONARC:                  return "SONARC";
        case audio::WAVEFORMAT_DSPGROUP_TRUESPEECH:     return "DSPGROUP TRUESPEECH";
        case audio::WAVEFORMAT_ECHOSC1:                 return "ECHOSC1";
        case audio::WAVEFORMAT_AUDIOFILE_AF36:          return "AUDIOFILE AF36";
        case audio::WAVEFORMAT_APTX:                    return "APTX";
        case audio::WAVEFORMAT_AUDIOFILE_AF10:          return "AUDIOFILE AF10";
        case audio::WAVEFORMAT_DOLBY_AC2:               return "DOLBY AC2";
        case audio::WAVEFORMAT_GSM610:                  return "GSM610";
        case audio::WAVEFORMAT_ANTEX_ADPCME:            return "ANTEX ADPCME";
        case audio::WAVEFORMAT_CONTROL_RES_VQLPC1:      return "CONTROL RES VQLPC (1)";
        case audio::WAVEFORMAT_CONTROL_RES_VQLPC2:      return "CONTROL RES VQLPC (2)";
        case audio::WAVEFORMAT_DIGIADPCM:               return "DIGIADPCM";
        case audio::WAVEFORMAT_CONTROL_RES_CR10:        return "CONTROL RES CR10";
        case audio::WAVEFORMAT_NMS_VBXADPCM:            return "NMS VBXADPCM";
        case audio::WAVEFORMAT_CS_IMAADPCM:             return "CS IMAADPCM (Roland RDAC)";
        case audio::WAVEFORMAT_G721_ADPCM:              return "G721 ADPCM";
        case audio::WAVEFORMAT_MPEG_1_Layer_I_II:       return "MPEG-1 Layer I, II";
        case audio::WAVEFORMAT_MPEG_1_Layer_III:        return "MPEG-1 Layer III (MP3)";
        case audio::WAVEFORMAT_XBOX_ADPCM:              return "Xbox ADPCM";
        case audio::WAVEFORMAT_CREATIVE_ADPCM:          return "CREATIVE ADPCM";
        case audio::WAVEFORMAT_CREATIVE_FASTSPEECH8:    return "CREATIVE FASTSPEECH8";
        case audio::WAVEFORMAT_CREATIVE_FASTSPEECH10:   return "CREATIVE FASTSPEECH10";
        case audio::WAVEFORMAT_FM_TOWNS_SND:            return "FM TOWNS SND";
        case audio::WAVEFORMAT_OLIGSM:                  return "OLIGSM";
        case audio::WAVEFORMAT_OLIADPCM:                return "OLIADPCM";
        case audio::WAVEFORMAT_OLICELP:                 return "OLICELP";
        case audio::WAVEFORMAT_OLISBC:                  return "OLISBC";
        case audio::WAVEFORMAT_OLIOPR:                  return "OLIOPR";
    }
    return "";
}

#ifdef SP_COMPILE_WITH_NETWORKSYSTEM

SP_EXPORT io::stringc toString(const network::ENetworkAdapterTypes NetworkType)
{
    switch (NetworkType)
    {
        case network::NETADAPTER_OTHER:
            return "Other";
        case network::NETADAPTER_ETHERNET:
            return "Ethernet";
        case network::NETADAPTER_TOKENRING:
            return "Token Ring";
        case network::NETADAPTER_FDDI:
            return "FDDI";
        case network::NETADAPTER_PPP:
            return "PPP";
        case network::NETADAPTER_LOOPBACK:
            return "Loopback";
        case network::NETADAPTER_SLIP:
            return "Slip";
        default:
            break;
    }
    return "Unknown";
}

#endif

#ifdef SP_COMPILE_WITH_STORYBOARD

SP_EXPORT io::stringc toString(const tool::EStoryboardLogicGates GateType)
{
    switch (GateType)
    {
        case LOGICGATE_AND:
            return "AND";
        case LOGICGATE_NAND:
            return "NAND";
        case LOGICGATE_OR:
            return "OR";
        case LOGICGATE_NOR:
            return "NOR";
        case LOGICGATE_XOR:
            return "XOR";
        case LOGICGATE_XNOR:
            return "XNOR";
    }
    return "";
}

#endif

} // /namespace Debugging


} // /namespace tool

} // /namespace sp



// ================================================================================
