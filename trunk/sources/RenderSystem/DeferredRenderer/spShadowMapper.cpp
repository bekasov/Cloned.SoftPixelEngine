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
    PointLightTexArray_ (0),
    SpotLightTexArray_  (0),
    MaxPointLightCount_ (0),
    MaxSpotLightCount_  (0)
{
}
ShadowMapper::~ShadowMapper()
{
    deleteShadowMaps();
}

void ShadowMapper::createShadowMaps(u32 MaxPointLightCount, u32 MaxSpotLightCount)
{
    /* Delete old shadow maps and setup light count */
    deleteShadowMaps();
    
    MaxPointLightCount_ = MaxPointLightCount;
    MaxSpotLightCount_  = MaxSpotLightCount;
    
    /* Create new point light shadow map */
    if (MaxPointLightCount_ > 0)
    {
        //todo
    }
    
    /* Create new spot light shadow map */
    if (MaxSpotLightCount_ > 0)
    {
        //todo
    }
}

void ShadowMapper::deleteShadowMaps()
{
    __spVideoDriver->deleteTexture(PointLightTexArray_);
    __spVideoDriver->deleteTexture(SpotLightTexArray_);
}

bool ShadowMapper::renderShadowMap(scene::SceneGraph* Graph, scene::Light* LightObj, u32 Index)
{
    if (Graph && LightObj)
    {
        switch (LightObj->getLightModel())
        {
            case scene::LIGHT_POINT:
                return renderPointLightShadowMap(Graph, LightObj, Index);
            case scene::LIGHT_SPOT:
                return renderSpotLightShadowMap(Graph, LightObj, Index);
            default:
                break;
        }
    }
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

bool ShadowMapper::renderPointLightShadowMap(scene::SceneGraph* Graph, scene::Light* LightObj, u32 Index)
{
    if (!PointLightTexArray_ || Index >= MaxPointLightCount_)
        return false;
    
    //todo
    
    return true;
}

bool ShadowMapper::renderSpotLightShadowMap(scene::SceneGraph* Graph, scene::Light* LightObj, u32 Index)
{
    if (!SpotLightTexArray_ || Index >= MaxSpotLightCount_)
        return false;
    
    //todo
    
    return true;
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
