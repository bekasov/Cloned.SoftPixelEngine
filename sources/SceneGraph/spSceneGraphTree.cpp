/*
 * Scene graph tree file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "SceneGraph/spSceneGraphTree.hpp"

#ifdef SP_COMPILE_WITH_SCENEGRAPH_TREE


#include "Platform/spSoftPixelDeviceOS.hpp"


namespace sp
{

extern video::RenderSystem* __spVideoDriver;

namespace scene
{


/*
 * Internal declerations
 */

bool cmpObjectSceneNodes(SceneNode* &obj1, SceneNode* &obj2);


/*
 * SceneGraphTree class
 */

SceneGraphTree::SceneGraphTree() :
    SceneGraph(SCENEGRAPH_TREE)
{
    hasChildTree_ = true;
}
SceneGraphTree::~SceneGraphTree()
{
}

void SceneGraphTree::addSceneNode(SceneNode* Object)
{
    if (Object)
    {
        NodeList_.push_back(Object);
        RootNodeList_.push_back(Object);
    }
}
void SceneGraphTree::removeSceneNode(SceneNode* Object)
{
    if (Object)
    {
        removeObjectFromList<SceneNode>(Object, NodeList_       );
        removeObjectFromList<SceneNode>(Object, RootNodeList_   );
    }
}

void SceneGraphTree::addSceneNode(Camera* Object)
{
    if (Object)
    {
        CameraList_.push_back(Object);
        RootNodeList_.push_back(Object);
    }
}
void SceneGraphTree::removeSceneNode(Camera* Object)
{
    if (Object)
    {
        removeObjectFromList<Camera>    (Object, CameraList_    );
        removeObjectFromList<SceneNode> (Object, RootNodeList_  );
    }
}

void SceneGraphTree::addSceneNode(Light* Object)
{
    if (Object)
    {
        LightList_.push_back(Object);
        RootNodeList_.push_back(Object);
    }
}
void SceneGraphTree::removeSceneNode(Light* Object)
{
    if (Object)
    {
        removeObjectFromList<Light>     (Object, LightList_     );
        removeObjectFromList<SceneNode> (Object, RootNodeList_  );
    }
}

void SceneGraphTree::addSceneNode(RenderNode* Object)
{
    if (Object)
    {
        RenderList_.push_back(Object);
        RootNodeList_.push_back(Object);
    }
}
void SceneGraphTree::removeSceneNode(RenderNode* Object)
{
    if (Object)
    {
        removeObjectFromList<RenderNode>(Object, RenderList_    );
        removeObjectFromList<SceneNode> (Object, RootNodeList_  );
    }
}

void SceneGraphTree::addRootNode(SceneNode* Object)
{
    if (Object)
        RootNodeList_.push_back(Object);
}
void SceneGraphTree::removeRootNode(SceneNode* Object)
{
    removeObjectFromList<SceneNode>(Object, RootNodeList_);
}

void SceneGraphTree::render()
{
    /* Update scene graph transformation */
    const dim::matrix4f BaseMatrix(getTransformation(true) * spWorldMatrix);
    
    /* Update object transformation */
    for (std::list<SceneNode*>::iterator it = RootNodeList_.begin(); it != RootNodeList_.end(); ++it)
    {
        spWorldMatrix = BaseMatrix;
        updateRootNode(*it);
    }
    
    /* Render objects */
    RootNodeList_.sort(cmpObjectSceneNodes);
    
    for (std::list<SceneNode*>::iterator it = RootNodeList_.begin(); it != RootNodeList_.end(); ++it)
    {
        spWorldMatrix.reset();
        renderRootNode(*it);
    }
}


/*
 * ======= Protected: =======
 */

void SceneGraphTree::updateRootNode(SceneNode* Object)
{
    if (!Object->getVisible())
        return;
    
    Object->updateTransformation();
    
    for (std::list<SceneNode*>::const_iterator it = Object->getSceneChildren().begin(); it != Object->getSceneChildren().end(); ++it)
        updateRootNode(*it);
}

void SceneGraphTree::renderRootNode(SceneNode* Object)
{
    if (!Object->getVisible())
        return;
    
    /* Update individual object type */
    switch (Object->getType())
    {
        case NODE_MESH:
            setActiveMesh(static_cast<Mesh*>(Object));
        case NODE_BILLBOARD:
        case NODE_TERRAIN:
            static_cast<RenderNode*>(Object)->render();
            break;
            
        case NODE_LIGHT:
            static_cast<Light*>(Object)->render();
            break;
            
        default:
            Object->loadTransformation();
            break;
    }
    
    /* Render children */
    Object->getSceneChildren().sort(cmpObjectSceneNodes);
    
    for (std::list<SceneNode*>::const_iterator it = Object->getSceneChildren().begin(); it != Object->getSceneChildren().end(); ++it)
        renderRootNode(*it);
}


} // /namespace scene

} // /namespace sp


#endif



// ================================================================================
