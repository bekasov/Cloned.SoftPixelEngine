/*
 * Direct3D9 texture header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_RENDERSYSTEM_DIRECT3D9TEXTURE_H__
#define __SP_RENDERSYSTEM_DIRECT3D9TEXTURE_H__


#include "Base/spStandard.hpp"

#if defined(SP_COMPILE_WITH_DIRECT3D9)


#include "RenderSystem/spTextureBase.hpp"

#if defined(SP_PLATFORM_WINDOWS)
#   include <d3d9.h>
#   include <d3dx9.h>
#   if defined(SP_COMPILER_VC)
#       include <dxerr.h>
#   else
#       include <dxerr9.h>
#   endif
#endif


namespace sp
{
namespace video
{


union UDx9TexResource
{
    UDx9TexResource() :
        Res(0)
    {
    }
    
    /* Members */
    IDirect3DBaseTexture9*      Res;
    IDirect3DTexture9*          Tex2D;
    IDirect3DCubeTexture9*      TexCube;
    IDirect3DVolumeTexture9*    TexVolume;
};


class Direct3D9Driver;

class SP_EXPORT Direct3D9Texture : public Texture
{
    
    public:
        
        Direct3D9Texture(const STextureCreationFlags &CreationFlags);
        ~Direct3D9Texture();
        
        /* === Functions === */
        
        bool valid() const;
        
        void bind(s32 Level = 0) const;
        void unbind(s32 Level = 0) const;
        
        bool shareImageBuffer();
        bool updateImageBuffer();
        
    private:
        
        friend class Direct3D9RenderSystem;
        friend class Direct3D9Shader;
        
        /* === Functions === */
        
        bool createHWTextureResource(
            bool MipMaps, const ETextureTypes Type, dim::vector3di Size, const EPixelFormats Format,
            const u8* ImageData, const EHWTextureFormats HWFormat = HWTEXFORMAT_UBYTE8, bool isRenderTarget = false
        );
        bool createHWTexture();

        void updateTextureAttributes(s32 SamplerLayer) const;
        
        void updateImageTexture();
        void updateImageCubeTexture(s32 Face);
        void updateImageVolumeTexture();
        
        void writeImageSurfaceBuffer(u8* DestImageBuffer, const u8* ImageBuffer, s32 Width, s32 Height);
        void readImageSurfaceBuffer(const u8* DestImageBuffer, u8* ImageBuffer, s32 Width, s32 Height);
        
        bool createMipMaps(s32 Level = 1);
        void generateMipMapLevel(s32* src, s32* dst, s32 Width, s32 Height);
        
        bool is2D() const;
        bool isCube() const;
        bool isVolume() const;

        /* === Static functions === */

        static void setupTextureFormats(
            const EPixelFormats Format, const EHWTextureFormats HWFormat, D3DFORMAT &DxFormat, DWORD &Usage
        );
        
        /* === Members === */
        
        UDx9TexResource D3DResource_;

};


} // /namespace video

} // /namespace sp


#endif

#endif



// ================================================================================
