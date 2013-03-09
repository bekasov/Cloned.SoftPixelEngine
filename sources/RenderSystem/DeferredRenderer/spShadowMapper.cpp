/*
 * Shadow mapper file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "RenderSystem/DeferredRenderer/spShadowMapper.hpp"
#include "RenderSystem/spRenderSystem.hpp"
#include "SceneGraph/spSceneGraph.hpp"
#include "Base/spMathCollisionLibrary.hpp"


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
    TexSize_            (256    ),
    MaxPointLightCount_ (0      ),
    MaxSpotLightCount_  (0      ),
    UseVSM_             (false  ),
    UseRSM_             (false  )
{
}
ShadowMapper::~ShadowMapper()
{
    deleteShadowMaps();
}

bool ShadowMapper::createShadowMaps(
    s32 TexSize, u32 MaxPointLightCount, u32 MaxSpotLightCount, bool UseVSM, bool UseRSM)
{
    /*
    Check if the texture size is smaller then or equal to zero or the maximal
    count of lights are zero (note the bitwise OR operator).
    */
    if ( TexSize <= 0 || ( MaxPointLightCount | MaxSpotLightCount ) == 0 )
    {
        #ifdef SP_DEBUGMODE
        io::Log::debug("ShadowMapper::createShadowMaps");
        #endif
        return false;
    }
    
    /* Delete old shadow maps and setup light count */
    deleteShadowMaps();
    
    /* Store new settings */
    TexSize_            = math::RoundPow2(TexSize);
    UseVSM_             = UseVSM;
    UseRSM_             = UseRSM;
    
    MaxPointLightCount_ = MaxPointLightCount;
    MaxSpotLightCount_  = MaxSpotLightCount;
    
    /* Prepare texture creation flags */
    STextureCreationFlags CreationFlags;
    
    CreationFlags.Size      = dim::size2di(TexSize_);
    CreationFlags.Format    = (UseVSM_ ? PIXELFORMAT_GRAYALPHA : PIXELFORMAT_GRAY);
    CreationFlags.WrapMode  = TEXWRAP_CLAMP;
    CreationFlags.HWFormat  = HWTEXFORMAT_FLOAT16;
    CreationFlags.MipMaps   = UseVSM_;
    
    /* Create new point light shadow map */
    if (MaxPointLightCount_ > 0)
    {
        CreationFlags.Depth     = MaxPointLightCount_ * 6;
        CreationFlags.Dimension = TEXTURE_CUBEMAP_ARRAY;
        
        ShadowCubeMapArray_.DepthMap = __spVideoDriver->createTexture(CreationFlags);
        ShadowCubeMapArray_.DepthMap->setRenderTarget(true);
    }
    
    /* Create new spot light shadow map */
    if (MaxSpotLightCount_ > 0)
    {
        CreationFlags.Depth     = MaxSpotLightCount_;
        CreationFlags.Dimension = TEXTURE_2D_ARRAY;
        
        ShadowMapArray_.DepthMap = __spVideoDriver->createTexture(CreationFlags);
        ShadowMapArray_.DepthMap->setRenderTarget(true);
    }
    
    return true;
}

void ShadowMapper::deleteShadowMaps()
{
    ShadowMapArray_.clear();
    ShadowCubeMapArray_.clear();
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

void ShadowMapper::bind(s32 SpotLightLayer, s32 PointLightLayer)
{
    if (ShadowMapArray_.DepthMap && ShadowCubeMapArray_.DepthMap)
    {
        ShadowMapArray_.DepthMap->bind(SpotLightLayer);
        ShadowCubeMapArray_.DepthMap->bind(PointLightLayer);
    }
}

void ShadowMapper::unbind(s32 SpotLightLayer, s32 PointLightLayer)
{
    if (ShadowMapArray_.DepthMap && ShadowCubeMapArray_.DepthMap)
    {
        ShadowCubeMapArray_.DepthMap->unbind(PointLightLayer);
        ShadowMapArray_.DepthMap->unbind(SpotLightLayer);
    }
}

bool ShadowMapper::renderCubeMap(
    scene::SceneGraph* Graph, scene::Camera* Cam, Texture* Tex, const dim::vector3df &Position)
{
    /* Check for valid inputs */
    if ( !Graph || !Cam || !Tex || ( Tex->getDimension() != TEXTURE_CUBEMAP && Tex->getDimension() != TEXTURE_CUBEMAP_ARRAY ) )
    {
        #ifdef SP_DEBUGMODE
        io::Log::debug("ShadowMapper::renderCubeMap");
        #endif
        return false;
    }
    
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
    {
        #ifdef SP_DEBUGMODE
        io::Log::debug("ShadowMapper::renderCubeMapDirection");
        #endif
        return false;
    }
    
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
    {
        #ifdef SP_DEBUGMODE
        io::Log::debug("ShadowMapper::renderCubeMap");
        #endif
        return false;
    }
    
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
 * ======= Protected: =======
 */

bool ShadowMapper::renderPointLightShadowMap(
    scene::SceneGraph* Graph, scene::Camera* Cam, scene::Light* LightObj, u32 Index)
{
    if (!ShadowCubeMapArray_.DepthMap || Index >= MaxPointLightCount_)
    {
        #ifdef SP_DEBUGMODE
        io::Log::debug("ShadowMapper::renderPointLightShadowMap");
        #endif
        return false;
    }
    
    ShadowCubeMapArray_.DepthMap->setArrayLayer(Index);
    
    //todo
    
    return true;
}

bool ShadowMapper::renderSpotLightShadowMap(
    scene::SceneGraph* Graph, scene::Camera* Cam, scene::Light* LightObj, u32 Index)
{
    if (!ShadowMapArray_.DepthMap || Index >= MaxSpotLightCount_)
    {
        #ifdef SP_DEBUGMODE
        io::Log::debug("ShadowMapper::renderSpotLightShadowMap");
        #endif
        return false;
    }
    
    /* Setup depth camera for light source */
    DepthCam_.setPosition(LightObj->getPosition(true));
    DepthCam_.setRotationMatrix(LightObj->getRotationMatrix(true));
    DepthCam_.setPerspective(dim::rect2di(0, 0, TexSize_, TexSize_), 0.01f, 1000.0f, 90.0f);//LightObj->getSpotConeOuter());
    
    /* Check if the spot-light view-frustum is inside the camera's view frustum */
    #if 0
    if (Cam && checkLightFrustumCulling(Cam, LightObj))
    {
        #if 0 //!!!
        static s32 c;
        io::Log::message("CULLING " + io::stringc(++c));
        #endif
        return false;
    }
    #endif
    
    /* Render scene into shadow map */
    if (UseRSM_)
        renderSceneIntoGBuffer(Graph, ShadowMapArray_, Index);
    else
        renderSceneIntoDepthTexture(Graph, ShadowMapArray_, Index);
    
    return true;
}

bool ShadowMapper::checkLightFrustumCulling(scene::Camera* Cam, scene::Light* LightObj) const
{
    /* Get spot-light frustum */
    scene::ViewFrustum Frustum;
    dim::vector3df GlobalPosition;
    
    if (!LightObj->getSpotFrustum(Frustum, GlobalPosition))
        return false;
    
    /* Make frustum culling test */
    return !math::CollisionLibrary::checkPyramidPyramidOverlap(
        Cam->getPosition(true), Cam->getViewFrustum(), GlobalPosition, Frustum
    );
}

void ShadowMapper::renderSceneIntoDepthTexture(scene::SceneGraph* Graph, SShadowMap &ShadowMap, u32 Index)
{
    /* Setup texture array layer */
    ShadowMap.DepthMap->setArrayLayer(Index);
    
    /* Render shadow depth map */
    __spVideoDriver->setRenderTarget(ShadowMap.DepthMap);
    {
        __spVideoDriver->clearBuffers(BUFFER_DEPTH);
        
        /* Render scene plain (for depth only) */
        Graph->renderScenePlain(&DepthCam_);
    }
    __spVideoDriver->setRenderTarget(0);
}

void ShadowMapper::renderSceneIntoGBuffer(scene::SceneGraph* Graph, SShadowMap &ShadowMap, u32 Index)
{
    /* Setup texture array layer */
    ShadowMap.DepthMap->setArrayLayer(Index);
    ShadowMap.ColorMap->setArrayLayer(Index);
    ShadowMap.NormalMap->setArrayLayer(Index);
    
    /*
    Setup visibility mask for texture layers
    -> we only want to render diffuse, normal and depth
    */
    const s32 PrevVisibleMask = __spVideoDriver->getTexLayerVisibleMask();
    __spVideoDriver->setTexLayerVisibleMask(TEXLAYERFLAG_DIFFUSE | TEXLAYERFLAG_NORMAL);
    
    /* Render shadow g-buffer */
    __spVideoDriver->setRenderTarget(ShadowMap.DepthMap);
    {
        __spVideoDriver->clearBuffers(BUFFER_DEPTH);
        
        /* Render scene in usual way */
        Graph->renderScene(&DepthCam_);
    }
    __spVideoDriver->setRenderTarget(0);
    
    /* Reset visibility mask */
    __spVideoDriver->setTexLayerVisibleMask(PrevVisibleMask);
}

void ShadowMapper::renderCubeMapDirection(
    scene::SceneGraph* Graph, Texture* Tex, const ECubeMapDirections Direction)
{
    /* Setup cubemap face for camera rotation and texture */
    ShadowMapper::ViewCam_.setRotationMatrix(ShadowMapper::CUBEMAP_ROTATIONS[Direction]);
    Tex->setCubeMapFace(Direction);
    
    /* Set render target and clear depth buffer */
    __spVideoDriver->setRenderTarget(Tex);
    __spVideoDriver->clearBuffers(BUFFER_DEPTH);
    
    /* Render the scene */
    Graph->renderScene(&ShadowMapper::ViewCam_);
}


/*
 * SShadowMap structure
 */

ShadowMapper::SShadowMap::SShadowMap() :
    DepthMap    (0),
    ColorMap    (0),
    NormalMap   (0)
{
}
ShadowMapper::SShadowMap::~SShadowMap()
{
}

void ShadowMapper::SShadowMap::clear()
{
    __spVideoDriver->deleteTexture(DepthMap);
    __spVideoDriver->deleteTexture(ColorMap);
    __spVideoDriver->deleteTexture(NormalMap);
}


} // /namespace video

} // /namespace sp



// ================================================================================
