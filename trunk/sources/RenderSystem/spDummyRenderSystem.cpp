/*
 * Dummy render system file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "RenderSystem/spDummyRenderSystem.hpp"
#include "SceneGraph/spSceneCamera.hpp"
#include "Base/spInternalDeclarations.hpp"
#include "Platform/spSoftPixelDeviceOS.hpp"


namespace sp
{

extern scene::SceneGraph* __spSceneManager;

namespace video
{


DummyRenderSystem::DummyRenderSystem() : RenderSystem(RENDERER_DUMMY)
{
    createDefaultVertexFormats();
}
DummyRenderSystem::~DummyRenderSystem()
{
    clear();
}


/*
 * ======= Initialization functions =======
 */

void DummyRenderSystem::setupConfiguration()
{
}


/*
 * ======= Renderer information =======
 */

io::stringc DummyRenderSystem::getRenderer() const
{
    return "Null device";
}
io::stringc DummyRenderSystem::getVersion() const
{
    return "Dummy";
}
io::stringc DummyRenderSystem::getVendor() const
{
    return "SoftPixel Engine";
}
io::stringc DummyRenderSystem::getShaderVersion() const
{
    return "";
}

bool DummyRenderSystem::queryVideoSupport(const EVideoFeatureQueries Query) const
{
    return false;
}

s32 DummyRenderSystem::getMultitexCount() const
{
    return 0;
}
s32 DummyRenderSystem::getMaxAnisotropicFilter() const
{
    return 0;
}
s32 DummyRenderSystem::getMaxLightCount() const
{
    return 0;
}


/*
 * ======= Video buffer control functions =======
 */

void DummyRenderSystem::clearBuffers(const s32 ClearFlags)
{
}
void DummyRenderSystem::flipBuffers()
{
}


/*
 * ======= Rendering functions =======
 */

void DummyRenderSystem::setupMaterialStates(const MaterialStates* Material)
{
}

void DummyRenderSystem::setRenderState(const video::ERenderStates Type, s32 State)
{
    switch (Type)
    {
        case RENDER_TEXTURE:
            __isTexturing = (State != 0); break;
        default:
            if (Type >= 0 && Type < 16)
                RenderStates_[Type] = State;
            break;
    }
}

s32 DummyRenderSystem::getRenderState(const video::ERenderStates Type) const
{
    switch (Type)
    {
        case RENDER_TEXTURE:
            return (s32)__isTexturing;
        default:
            if (Type >= 0 && Type < 16)
                return RenderStates_[Type];
            break;
    }
    
    return 0;
}


/*
 * ======= Texture loading and creating =======
 */

Texture* DummyRenderSystem::loadTexture(ImageLoader* Loader)
{
    if (!Loader)
        return RenderSystem::createTexture(DEF_TEXTURE_SIZE);
    
    Texture* NewTexture = 0;
    
    /* Load iamge data */
    SImageDataRead* ImageData = Loader->loadImageData();
    
    if (ImageData)
    {
        /* Setup texture creation flags */
        STextureCreationFlags CreationFlags(TexGenFlags_);
        
        CreationFlags.Filename      = Loader->getFilename();
        CreationFlags.Size          = dim::size2di(ImageData->Width, ImageData->Height);
        CreationFlags.ImageBuffer   = ImageData->ImageBuffer;
        CreationFlags.Format        = ImageData->Format;
        
        /* Create engine texture */
        NewTexture = new Texture(CreationFlags);
        
        /* Delete image data */
        MemoryManager::deleteMemory(ImageData);
    }
    else
    {
        /* Create an empty texture */
        NewTexture = RenderSystem::createTexture(DEF_TEXTURE_SIZE);
        
        io::Log::lowerTab();
        return NewTexture;
    }
    
    /* Add the texture to the list */
    TextureList_.push_back(NewTexture);
    
    io::Log::lowerTab();
    return NewTexture;
}

Texture* DummyRenderSystem::copyTexture(const Texture* Tex)
{
    if (!Tex)
        return RenderSystem::createTexture(DEF_TEXTURE_SIZE);
    
    /* Setup texture creation flags */
    STextureCreationFlags CreationFlags;
    
    CreationFlags.Filename      = Tex->getFilename();
    CreationFlags.Size          = Tex->getSize();
    CreationFlags.ImageBuffer   = Tex->getImageBuffer();
    CreationFlags.MagFilter     = Tex->getMagFilter();
    CreationFlags.MinFilter     = Tex->getMinFilter();
    CreationFlags.MipMapFilter  = Tex->getMipMapFilter();
    CreationFlags.Format        = Tex->getFormat();
    CreationFlags.MipMaps       = Tex->getMipMapping();
    CreationFlags.WrapMode      = Tex->getWrapMode();
    
    /* Allocate the new texture */
    Texture* NewTexture = new Texture(CreationFlags);
    
    /* Add the texture to the texture list */
    TextureList_.push_back(NewTexture);
    
    /* Return the texture & exit the function */
    return NewTexture;
}

Texture* DummyRenderSystem::createTexture(const STextureCreationFlags &CreationFlags)
{
    /* Create new dummy texture */
    Texture* NewTexture = new Texture(CreationFlags);
    TextureList_.push_back(NewTexture);
    return NewTexture;
}


/*
 * ======= Matrix controll =======
 */

void DummyRenderSystem::updateModelviewMatrix()
{
}


/*
 * ======= Private: =======
 */

void DummyRenderSystem::clear()
{
    MemoryManager::deleteList(ShaderList_);
    MemoryManager::deleteList(ShaderClassList_);
}


} // /namespace video

} // /namespace sp



// ================================================================================
