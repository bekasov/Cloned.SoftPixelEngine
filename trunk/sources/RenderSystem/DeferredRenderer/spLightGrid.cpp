/*
 * Light grid file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "RenderSystem/DeferredRenderer/spLightGrid.hpp"

#if defined(SP_COMPILE_WITH_DEFERREDRENDERER)


#include "RenderSystem/spRenderSystem.hpp"
#include "RenderSystem/spShaderResource.hpp"
#include "SceneGraph/spSceneLight.hpp"
#include "Base/spSharedObjects.hpp"


namespace sp
{

extern video::RenderSystem* __spVideoDriver;

namespace video
{


LightGrid::LightGrid() :
    TLITexture_         (0),
    TLIShaderResource_  (0)
{
}
LightGrid::~LightGrid()
{
    deleteGrid();
}

bool LightGrid::createGrid(const dim::size2di &Resolution, const dim::size2di &GridSize)
{
    /* Delete old grid */
    deleteGrid();

    if (__spVideoDriver->getRendererType() == video::RENDERER_OPENGL)//!!!
    {
        /* Create new buffer texture */
        video::STextureCreationFlags CreationFlags;
        {
            CreationFlags.Size          = 128;//1;
            CreationFlags.Format        = video::PIXELFORMAT_GRAYALPHA;
            CreationFlags.HWFormat      = video::HWTEXFORMAT_INT32;
            CreationFlags.BufferType    = video::IMAGEBUFFER_UBYTE;//!!!IMAGEBUFFER_INT
            CreationFlags.Dimension     = video::TEXTURE_BUFFER;
        }
        TLITexture_ = __spVideoDriver->createTexture(CreationFlags);

        #if 1//!!!
        ImageBuffer* buf = TLITexture_->getImageBuffer();
        u32* rawbuf = reinterpret_cast<u32*>(buf->getBuffer());

        for (u32 i = 0; i < 50; ++i)
        {
            if (i > 25)
                *(rawbuf++) = 100;
            else
                *(rawbuf++) = i;
            *(rawbuf++) = 0;
        }

        TLITexture_->updateImageBuffer();
        #endif

        //...

        return true;
    }
    else if (__spVideoDriver->getRendererType() == video::RENDERER_DIRECT3D11)//!!!
    {
        /* Create new shader resource */
        TLIShaderResource_ = __spVideoDriver->createShaderResource();

        if (!TLIShaderResource_)
            return false;

        #if 1//!!!
        dim::point2di rawbuf[50];

        for (u32 i = 0; i < 50; ++i)
        {
            if (i > 25)
                rawbuf[i].X = 100;
            else
                rawbuf[i].X = i;
            rawbuf[i].Y = 0;
        }

        TLIShaderResource_->setupBuffer<dim::point2di>(50, false, &rawbuf[0].X);
        #endif

        return true;
    }

    return false;
}

void LightGrid::deleteGrid()
{
    if (TLITexture_)
        __spVideoDriver->deleteTexture(TLITexture_);
    if (TLIShaderResource_)
        __spVideoDriver->deleteShaderResource(TLIShaderResource_);
}

void LightGrid::fillLightIntoGrid(scene::Light* Obj)
{

    //todo

}

s32 LightGrid::bind(s32 TexLayerBase)
{
    if (TLITexture_)
        TLITexture_->bind(TexLayerBase++);
    return TexLayerBase;
}
s32 LightGrid::unbind(s32 TexLayerBase)
{
    if (TLITexture_)
        TLITexture_->unbind(TexLayerBase++);
    return TexLayerBase;
}


} // /namespace video

} // /namespace sp


#endif



// ================================================================================
