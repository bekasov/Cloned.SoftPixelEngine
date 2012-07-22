/*
 * Direct3D9 texture file
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


class Direct3D9Driver;

class SP_EXPORT Direct3D9Texture : public Texture
{
    
    public:
        
        Direct3D9Texture();
        Direct3D9Texture(
            IDirect3DTexture9* d3dTexture, IDirect3DCubeTexture9* d3dCubeTexture, IDirect3DVolumeTexture9* d3dVolumeTexture,
            const STextureCreationFlags &CreationFlags
        );
        ~Direct3D9Texture();
        
        bool valid() const;
        
        /* Extra option functions */
        
        void setColorIntensity(f32 Red, f32 Green, f32 Blue);
        
        /* Binding & unbinding */
        
        void bind(s32 Level = 0) const;
        void unbind(s32 Level = 0) const;
        
        /* Functions for updating image buffer */
        
        void shareImageBuffer();
        void updateImageBuffer();
        
    private:
        
        friend class Direct3D9RenderSystem;
        friend class Direct3D9Shader;
        
        /* Private functions */
        
        void clear();
        
        void updateFormat();
        
        void updateFormatSize();
        s32 getFormatSize(const EPixelFormats Format) const;
        
        void recreateHWTexture();
        
        void updateImageTexture();
        void updateImageCubeTexture(s32 Face);
        void updateImageVolumeTexture();
        
        void writeImageSurfaceBuffer(u8* DestImageBuffer, const u8* ImageBuffer, s32 Width, s32 Height);
        void readImageSurfaceBuffer(const u8* DestImageBuffer, u8* ImageBuffer, s32 Width, s32 Height);
        
        bool createMipMaps(s32 Level = 1);
        void generateMipMapLevel(s32* src, s32* dst, s32 Width, s32 Height);
        
        void updateBaseTexture();
        
        /* Members */
        
        IDirect3DBaseTexture9* pDirect3DBaseTexture_;
        
        IDirect3DTexture9* pDirect3DTexture_;
        IDirect3DCubeTexture9* pDirect3DCubeTexture_;
        IDirect3DVolumeTexture9* pDirect3DVolumeTexture_;
        
};


} // /namespace video

} // /namespace sp


#endif

#endif



// ================================================================================
