/*
 * Simple scene graph file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "SceneGraph/spSceneGraphSimple.hpp"

#ifdef SP_COMPILE_WITH_SCENEGRAPH_SIMPLE


#include <boost/foreach.hpp>


namespace sp
{
namespace scene
{


SceneGraphSimple::SceneGraphSimple() :
    SceneGraph(SCENEGRAPH_SIMPLE)
{
}
SceneGraphSimple::~SceneGraphSimple()
{
}

void SceneGraphSimple::render()
{
    /* Update scene graph transformation */
    const dim::matrix4f BaseMatrix(getTransformation(true));
    
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


} // /namespace scene

} // /namespace sp


#endif



// ================================================================================
