/*
 * Shadow mapper file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "RenderSystem/DeferredRenderer/spShadowMapper.hpp"
#include "RenderSystem/spRenderSystem.hpp"
#include "SceneGraph/spSceneGraph.hpp"


namespace sp
{

extern video::RenderSystem* __spVideoDriver;

namespace video
{


#define DefineCubeMapRotation(x1, y1, z1,   x2, y2, z2,   x3, y3, z3) \
    dim::matrix4f(dim::vector4df(x1, y1, z1), dim::vector4df(x2, y2, z2), dim::vector4df(x3, y3, z3), dim::vector4df())

const dim::matrix4f ShadowMapper::CUBEMAP_ROTATIONS[6] =
{
    DefineCubeMapRotation( 0, 0, -1,   0, 1,  0,    1,  0,  0), // Positive X
    DefineCubeMapRotation( 0, 0,  1,   0, 1,  0,   -1,  0,  0), // Negative X
    DefineCubeMapRotation( 1, 0,  0,   0, 0, -1,    0,  1,  0), // Positive Y
    DefineCubeMapRotation( 1, 0,  0,   0, 0,  1,    0, -1,  0), // Negative Y
    DefineCubeMapRotation( 1, 0,  0,   0, 1,  0,    0,  0,  1), // Positive Z
    DefineCubeMapRotation(-1, 0,  0,   0, 1,  0,    0,  0, -1)  // Negative Z
};

#undef DefineCubeMapRotation

scene::Camera ShadowMapper::ViewCam_(dim::rect2di(), 0.1f, 1000.0f, 90.0f);

ShadowMapper::ShadowMapper() :
    ShadowMapArray_     (0      ),
    ShadowCubeMapArray_ (0      ),
    TexSize_            (256    ),
    MaxPointLightCount_ (0      ),
    MaxSpotLightCount_  (0      ),
    UseVSM_             (false  )
{
}
ShadowMapper::~ShadowMapper()
{
    deleteShadowMaps();
}

void ShadowMapper::createShadowMaps(s32 TexSize, u32 MaxPointLightCount, u32 MaxSpotLightCount, bool UseVSM)
{
    if (TexSize <= 0)
    {
        #ifdef SP_DEBUGMODE
        io::Log::debug("ShadowMapper::createShadowMaps");
        #endif
        return;
    }
    
    /* Delete old shadow maps and setup light count */
    deleteShadowMaps();
    
    TexSize_            = math::RoundPow2(TexSize);
    UseVSM_             = UseVSM;
    
    MaxPointLightCount_ = MaxPointLightCount;
    MaxSpotLightCount_  = MaxSpotLightCount;
    
    /* Prepare texture creation flags */
    video::STextureCreationFlags CreationFlags;
    
    CreationFlags.Size      = dim::size2di(TexSize_);
    CreationFlags.Format    = (UseVSM_ ? video::PIXELFORMAT_GRAYALPHA : video::PIXELFORMAT_GRAY);
    CreationFlags.WrapMode  = video::TEXWRAP_CLAMP;
    CreationFlags.HWFormat  = video::HWTEXFORMAT_FLOAT16;
    CreationFlags.MipMaps   = UseVSM_;
    
    /* Create new point light shadow map */
    if (MaxPointLightCount_ > 0)
    {
        CreationFlags.Depth     = MaxPointLightCount_ * 6;
        CreationFlags.Dimension = video::TEXTURE_CUBEMAP_ARRAY;
        
        ShadowCubeMapArray_ = __spVideoDriver->createTexture(CreationFlags);
        ShadowCubeMapArray_->setRenderTarget(true);
    }
    
    /* Create new spot light shadow map */
    if (MaxSpotLightCount_ > 0)
    {
        CreationFlags.Depth     = MaxSpotLightCount_;
        CreationFlags.Dimension = video::TEXTURE_2D_ARRAY;
        
        ShadowMapArray_ = __spVideoDriver->createTexture(CreationFlags);
        ShadowMapArray_->setRenderTarget(true);
    }
}

void ShadowMapper::deleteShadowMaps()
{
    __spVideoDriver->deleteTexture(ShadowCubeMapArray_);
    __spVideoDriver->deleteTexture(ShadowMapArray_);
}

bool ShadowMapper::renderShadowMap(scene::SceneGraph* Graph, scene::Camera* Cam, scene::Light* LightObj, u32 Index)
{
    #ifdef SP_COMPILE_WITH_CG
    
    if (Graph && LightObj)
    {
        switch (LightObj->getLightModel())
        {
            case scene::LIGHT_POINT:
                return renderPointLightShadowMap(Graph, Cam, LightObj, Index);
            case scene::LIGHT_SPOT:
                return renderSpotLightShadowMap(Graph, Cam, LightObj, Index);
            default:
                break;
        }
    }
    
    #else
    
    io::Log::error("Can not render shadow maps without \"Cg Toolkit\"");
    
    #endif
    
    return false;
}

bool ShadowMapper::renderCubeMap(
    scene::SceneGraph* Graph, scene::Camera* Cam, Texture* Tex, const dim::vector3df &Position)
{
    /* Check for valid inputs */
    if ( !Graph || !Cam || !Tex || ( Tex->getDimension() != TEXTURE_CUBEMAP && Tex->getDimension() != TEXTURE_CUBEMAP_ARRAY ) )
        return false;
    
    /* Setup camera transformation */
    scene::Camera* PrevCam = Graph->getActiveCamera();
    const dim::matrix4f CamDir(Cam->getRotationMatrix());
    
    ShadowMapper::ViewCam_.setViewport(dim::rect2di(0, 0, Tex->getSize().Width, Tex->getSize().Height));
    ShadowMapper::ViewCam_.setPosition(Position);
    
    /* Render the scene for all 6 directions */
    renderCubeMapDirection(Graph, &ShadowMapper::ViewCam_, Tex, CamDir, CUBEMAP_POSITIVE_X);
    renderCubeMapDirection(Graph, &ShadowMapper::ViewCam_, Tex, CamDir, CUBEMAP_NEGATIVE_X);
    renderCubeMapDirection(Graph, &ShadowMapper::ViewCam_, Tex, CamDir, CUBEMAP_POSITIVE_Y);
    renderCubeMapDirection(Graph, &ShadowMapper::ViewCam_, Tex, CamDir, CUBEMAP_NEGATIVE_Y);
    renderCubeMapDirection(Graph, &ShadowMapper::ViewCam_, Tex, CamDir, CUBEMAP_POSITIVE_Z);
    renderCubeMapDirection(Graph, &ShadowMapper::ViewCam_, Tex, CamDir, CUBEMAP_NEGATIVE_Z);
    
    Graph->setActiveCamera(PrevCam);
    
    return true;
}

bool ShadowMapper::renderCubeMapDirection(
    scene::SceneGraph* Graph, scene::Camera* Cam, Texture* Tex,
    dim::matrix4f CamDir, const ECubeMapDirections Direction)
{
    if ( !Graph || !Cam || !Tex || ( Tex->getDimension() != TEXTURE_CUBEMAP && Tex->getDimension() != TEXTURE_CUBEMAP_ARRAY ) )
        return false;
    
    switch (Direction)
    {
        case CUBEMAP_POSITIVE_X:
            CamDir.rotateY(90);     break;
        case CUBEMAP_NEGATIVE_X:
            CamDir.rotateY(-90);    break;
        case CUBEMAP_POSITIVE_Y:
            CamDir.rotateX(-90);    break;
        case CUBEMAP_NEGATIVE_Y:
            CamDir.rotateX(90);     break;
        case CUBEMAP_POSITIVE_Z:
                                    break;
        case CUBEMAP_NEGATIVE_Z:
            CamDir.rotateY(180);    break;
    }
    
    Cam->setRotationMatrix(CamDir);
    
    Tex->setCubeMapFace(Direction);
    
    __spVideoDriver->setRenderTarget(Tex);
    {
        __spVideoDriver->clearBuffers();
        Graph->renderScene(Cam);
    }
    __spVideoDriver->setRenderTarget(0);
    
    return true;
}

bool ShadowMapper::renderCubeMap(
    scene::SceneGraph* Graph, Texture* Tex, const dim::vector3df &Position)
{
    /* Check for valid inputs */
    if ( !Graph || !Tex || ( Tex->getDimension() != TEXTURE_CUBEMAP && Tex->getDimension() != TEXTURE_CUBEMAP_ARRAY ) )
        return false;
    
    /* Setup camera transformation */
    scene::Camera* PrevCam = Graph->getActiveCamera();
    
    ShadowMapper::ViewCam_.setViewport(dim::rect2di(0, 0, Tex->getSize().Width, Tex->getSize().Height));
    ShadowMapper::ViewCam_.setPosition(Position);
    
    /* Render the scene for all 6 directions */
    renderCubeMapDirection(Graph, Tex, CUBEMAP_POSITIVE_X);
    renderCubeMapDirection(Graph, Tex, CUBEMAP_NEGATIVE_X);
    renderCubeMapDirection(Graph, Tex, CUBEMAP_POSITIVE_Y);
    renderCubeMapDirection(Graph, Tex, CUBEMAP_NEGATIVE_Y);
    renderCubeMapDirection(Graph, Tex, CUBEMAP_POSITIVE_Z);
    renderCubeMapDirection(Graph, Tex, CUBEMAP_NEGATIVE_Z);
    
    __spVideoDriver->setRenderTarget(0);
    
    Graph->setActiveCamera(PrevCam);
    
    return true;
}


/*
 * ======= Private: =======
 */

bool ShadowMapper::renderPointLightShadowMap(
    scene::SceneGraph* Graph, scene::Camera* Cam, scene::Light* LightObj, u32 Index)
{
    if (!ShadowCubeMapArray_ || Index >= MaxPointLightCount_)
    {
        #ifdef SP_DEBUGMODE
        io::Log::debug("ShadowMapper::renderPointLightShadowMap");
        #endif
        return false;
    }
    
    ShadowCubeMapArray_->setArrayLayer(Index);
    
    //todo
    
    return true;
}

bool ShadowMapper::renderSpotLightShadowMap(
    scene::SceneGraph* Graph, scene::Camera* Cam, scene::Light* LightObj, u32 Index)
{
    if (!ShadowMapArray_ || Index >= MaxSpotLightCount_)
    {
        #ifdef SP_DEBUGMODE
        io::Log::debug("ShadowMapper::renderSpotLightShadowMap");
        #endif
        return false;
    }
    
    /* Setup depth camera for light source */
    DepthCam_.setPosition(LightObj->getPosition(true));
    DepthCam_.setRotationMatrix(LightObj->getRotationMatrix(true));
    DepthCam_.setPerspective(dim::rect2di(0, 0, TexSize_, TexSize_), 0.01f, 1000.0f, LightObj->getSpotConeOuter());
    
    /* Check if the spot-light view-frustum is inside the camera's view frustum */
    if (Cam && checkLightFrustumCulling(Cam))
        return false;
    
    
    ShadowMapArray_->setArrayLayer(Index);
    
    //todo
    
    return true;
}

bool ShadowMapper::checkLightFrustumCulling(scene::Camera* Cam) const
{
    return !Cam->getViewFrustum().isFrustumInside(DepthCam_.getViewFrustum());
}

void ShadowMapper::renderCubeMapDirection(
    scene::SceneGraph* Graph, Texture* Tex, const ECubeMapDirections Direction)
{
    /* Setup cubemap face for camera rotation and texture */
    ShadowMapper::ViewCam_.setRotationMatrix(ShadowMapper::CUBEMAP_ROTATIONS[Direction]);
    Tex->setCubeMapFace(Direction);
    
    /* Set render target and clear depth buffer */
    __spVideoDriver->setRenderTarget(Tex);
    __spVideoDriver->clearBuffers(video::BUFFER_DEPTH);
    
    /* Render the scene */
    Graph->renderScene(&ShadowMapper::ViewCam_);
}


} // /namespace video

} // /namespace sp



// ================================================================================
