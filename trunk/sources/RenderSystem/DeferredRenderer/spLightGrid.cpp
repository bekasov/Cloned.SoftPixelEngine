/*
 * Light grid file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "RenderSystem/DeferredRenderer/spLightGrid.hpp"

#if defined(SP_COMPILE_WITH_DEFERREDRENDERER)


#include "RenderSystem/spRenderSystem.hpp"
#include "SceneGraph/spSceneLight.hpp"
#include "Base/spSharedObjects.hpp"


namespace sp
{

extern video::RenderSystem* __spVideoDriver;

namespace video
{


LightGrid::LightGrid() :
    TLITexture_(0)
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

    /* Create new buffer texture */
    video::STextureCreationFlags CreationFlags;
    {
        CreationFlags.Size          = 1;
        CreationFlags.Format        = video::PIXELFORMAT_GRAYALPHA;
        CreationFlags.HWFormat      = video::HWTEXFORMAT_INT32;
        CreationFlags.BufferType    = video::IMAGEBUFFER_UBYTE;//!!!IMAGEBUFFER_INT
        CreationFlags.Dimension     = video::TEXTURE_BUFFER;
    }
    TLITexture_ = __spVideoDriver->createTexture(CreationFlags);

    //...

    return true;
}

void LightGrid::deleteGrid()
{
    if (TLITexture_)
        __spVideoDriver->deleteTexture(TLITexture_);
}

void LightGrid::fillLightIntoGrid(scene::Light* Obj)
{

    //todo

}


} // /namespace video

} // /namespace sp


#endif



// ================================================================================
