/*
 * Simple scene graph file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "SceneGraph/spSceneGraphSimple.hpp"

#ifdef SP_COMPILE_WITH_SCENEGRAPH_SIMPLE


#include "RenderSystem/spRenderSystem.hpp"

#include <boost/foreach.hpp>


namespace sp
{

extern video::RenderSystem* __spVideoDriver;

namespace scene
{


SceneGraphSimple::SceneGraphSimple() :
    SceneGraph(SCENEGRAPH_SIMPLE)
{
    /* Setup plain material */
    MaterialPlain_.setBlending(false);
}
SceneGraphSimple::~SceneGraphSimple()
{
}

void SceneGraphSimple::render()
{
    /* Update scene graph transformation */
    const dim::matrix4f BaseMatrix(getTransformMatrix(true));
    
    /* Render lights */
    sortLightList(LightList_);
    
    s32 LightIndex = 0;
    
    foreach (Light* Node, LightList_)
    {
        if (!Node->getVisible())
            continue;
        if (++LightIndex > MAX_COUNT_OF_LIGHTS)
            break;
        
        spWorldMatrix = BaseMatrix;
        Node->render();
    }
    
    /* Render geometry */
    sortRenderList(RenderList_, BaseMatrix);
    
    foreach (RenderNode* Node, RenderList_)
    {
        if (!Node->getVisible())
            break;
        Node->render();
    }
}

void SceneGraphSimple::renderScenePlain(Camera* ActiveCamera)
{
    if (!ActiveCamera)
        return;
    
    /* Setup active camera for drawing the scene */
    ActiveCamera->setupRenderView();
    
    /* Update scene graph transformation */
    spWorldMatrix.reset();
    
    const dim::matrix4f BaseMatrix(getTransformMatrix(true));
    
    /* Setup default material states */
    __spVideoDriver->setupMaterialStates(&MaterialPlain_);
    
    /* Render geometry */
    foreach (RenderNode* Node, RenderList_)
    {
        if (Node->getType() != scene::NODE_MESH || !Node->getVisible())
            continue;
        
        /* Render mesh object plain */
        Mesh* MeshObj = static_cast<scene::Mesh*>(Node);
        
        /* Matrix transformation */
        MeshObj->updateTransformation();
        MeshObj->loadTransformation();
        
        /* Frustum culling */
        if (!MeshObj->getBoundingVolume().checkFrustumCulling(ActiveCamera->getViewFrustum(), __spVideoDriver->getWorldMatrix()))
            return;
        
        /* Update the render matrix */
        __spVideoDriver->updateModelviewMatrix();
        
        /* Setup shader class */
        __spVideoDriver->setupShaderClass(MeshObj, MeshObj->getShaderClass());
        
        /* Draw the current mesh object */
        foreach (video::MeshBuffer* Surface, MeshObj->getMeshBufferList())
            __spVideoDriver->drawMeshBufferPlain(Surface, true);
    }
    
    /* Finish rendering the scene */
    finishRenderScene();
}


} // /namespace scene

} // /namespace sp


#endif



// ================================================================================
