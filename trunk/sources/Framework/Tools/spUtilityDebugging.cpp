/*
 * Debugging file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "Framework/Tools/spUtilityDebugging.hpp"


namespace sp
{
namespace tool
{


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
        default:                            break;
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
        default:                            break;
    }
    return "";
}

SP_EXPORT io::stringc toString(const video::ETextureFilters TextureFilter)
{
    switch (TextureFilter)
    {
        case video::FILTER_LINEAR:  return "Linear";
        case video::FILTER_SMOOTH:  return "Smooth";
        default:                    break;
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
        default:                        break;
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
        default:                    break;
    }
    return "";
}

SP_EXPORT io::stringc toString(const video::ETextureTypes TextureType)
{
    switch (TextureType)
    {
        case video::TEXTURE_1D:             return "1D Texture";
        case video::TEXTURE_2D:             return "2D Texture";
        case video::TEXTURE_3D:             return "3D Texture";
        case video::TEXTURE_CUBEMAP:        return "Cube Texture";
        
        case video::TEXTURE_1D_ARRAY:       return "1D Texture Array";
        case video::TEXTURE_2D_ARRAY:       return "2D Texture Array";
        case video::TEXTURE_CUBEMAP_ARRAY:  return "Cube Texture Array";
        
        case video::TEXTURE_RECTANGLE:      return "Rectangle Texture";
        case video::TEXTURE_BUFFER:         return "Buffer Texture";
        
        case video::TEXTURE_1D_RW:          return "1D Texture RW";
        case video::TEXTURE_2D_RW:          return "2D Texture RW";
        case video::TEXTURE_3D_RW:          return "3D Texture RW";
        case video::TEXTURE_1D_ARRAY_RW:    return "1D Texture Array RW";
        case video::TEXTURE_2D_ARRAY_RW:    return "2D Texture Array RW";
        
        default:                            break;
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
        case video::HWTEXFORMAT_INT32:      return "32 Bit Integer";
        case video::HWTEXFORMAT_UINT32:     return "32 Bit Unsigned Integer";
        default:                            break;
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
        default:                        break;
    }
    return "";
}

SP_EXPORT io::stringc toString(const video::EAlphaBlendingTypes AlphaBlendingType)
{
    switch (AlphaBlendingType)
    {
        case video::BLENDING_BRIGHT:    return "Bright Blending";
        case video::BLENDING_DARK:      return "Dark Blending";
        default:                        break;
    }
    return "";
}

SP_EXPORT io::stringc toString(const video::EImageBufferTypes ImageBufferType)
{
    switch (ImageBufferType)
    {
        case video::IMAGEBUFFER_UBYTE:  return "Unsigned Byte";
        case video::IMAGEBUFFER_FLOAT:  return "Floating Point";
        default:                        break;
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
        default:                                break;
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
        default:                                        break;
    }
    return "";
}

#ifdef SP_COMPILE_WITH_NETWORKSYSTEM

SP_EXPORT io::stringc toString(const network::ENetworkAdapterTypes NetworkType)
{
    switch (NetworkType)
    {
        case network::NETADAPTER_OTHER:     return "Other";
        case network::NETADAPTER_ETHERNET:  return "Ethernet";
        case network::NETADAPTER_TOKENRING: return "Token Ring";
        case network::NETADAPTER_FDDI:      return "FDDI";
        case network::NETADAPTER_PPP:       return "PPP";
        case network::NETADAPTER_LOOPBACK:  return "Loopback";
        case network::NETADAPTER_SLIP:      return "Slip";
        default:                            break;
    }
    return "Unknown";
}

#endif

#ifdef SP_COMPILE_WITH_STORYBOARD

SP_EXPORT io::stringc toString(const tool::ELogicGates GateType)
{
    switch (GateType)
    {
        case LOGICGATE_AND:     return "AND";
        case LOGICGATE_NAND:    return "NAND";
        case LOGICGATE_OR:      return "OR";
        case LOGICGATE_NOR:     return "NOR";
        case LOGICGATE_XOR:     return "XOR";
        case LOGICGATE_XNOR:    return "XNOR";
        default:                break;
    }
    return "";
}

#endif

#ifdef SP_COMPILE_WITH_LIGHTMAPGENERATOR

SP_EXPORT io::stringc toString(const tool::ELightmapGenerationStates LightmapGenState)
{
    switch (LightmapGenState)
    {
        case LIGHTMAPSTATE_INITIALIZING:    return "Initializing";
        case LIGHTMAPSTATE_PARTITIONING:    return "Partitioning";
        case LIGHTMAPSTATE_SHADING:         return "Shading";
        case LIGHTMAPSTATE_BLURING:         return "Bluring";
        case LIGHTMAPSTATE_BAKING:          return "Baking";
        case LIGHTMAPSTATE_COMPLETED:       return "Completed";
        default:                            break;
    }
    return "";
}

#endif

} // /namespace Debugging


} // /namespace tool

} // /namespace sp



// ================================================================================
