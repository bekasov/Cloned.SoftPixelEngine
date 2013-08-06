/*
 * Scene graph family tree file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "SceneGraph/spSceneGraphFamilyTree.hpp"

#ifdef SP_COMPILE_WITH_SCENEGRAPH_FAMILY_TREE


#include "Platform/spSoftPixelDeviceOS.hpp"

#include <boost/foreach.hpp>


namespace sp
{

extern video::RenderSystem* GlbRenderSys;

namespace scene
{


/*
 * Internal declerations
 */

bool compareSceneNodes(SceneNode* obj1, SceneNode* obj2);


/*
 * SceneGraphTree class
 */

SceneGraphFamilyTree::SceneGraphFamilyTree() :
    SceneGraph(SCENEGRAPH_FAMILY_TREE)
{
    hasChildTree_ = true;
}
SceneGraphFamilyTree::~SceneGraphFamilyTree()
{
}

void SceneGraphFamilyTree::addSceneNode(SceneNode* Object)
{
    if (Object)
    {
        NodeList_.push_back(Object);
        RootNodeList_.push_back(Object);
    }
}
void SceneGraphFamilyTree::removeSceneNode(SceneNode* Object)
{
    if (Object)
    {
        removeObjectFromList<SceneNode>(Object, NodeList_       );
        removeObjectFromList<SceneNode>(Object, RootNodeList_   );
    }
}

void SceneGraphFamilyTree::addSceneNode(Camera* Object)
{
    if (Object)
    {
        CameraList_.push_back(Object);
        RootNodeList_.push_back(Object);
    }
}
void SceneGraphFamilyTree::removeSceneNode(Camera* Object)
{
    if (Object)
    {
        removeObjectFromList<Camera>    (Object, CameraList_    );
        removeObjectFromList<SceneNode> (Object, RootNodeList_  );
    }
}

void SceneGraphFamilyTree::addSceneNode(Light* Object)
{
    if (Object)
    {
        LightList_.push_back(Object);
        RootNodeList_.push_back(Object);
    }
}
void SceneGraphFamilyTree::removeSceneNode(Light* Object)
{
    if (Object)
    {
        removeObjectFromList<Light>     (Object, LightList_     );
        removeObjectFromList<SceneNode> (Object, RootNodeList_  );
    }
}

void SceneGraphFamilyTree::addSceneNode(RenderNode* Object)
{
    if (Object)
    {
        RenderList_.push_back(Object);
        RootNodeList_.push_back(Object);
    }
}
void SceneGraphFamilyTree::removeSceneNode(RenderNode* Object)
{
    if (Object)
    {
        removeObjectFromList<RenderNode>(Object, RenderList_    );
        removeObjectFromList<SceneNode> (Object, RootNodeList_  );
    }
}

void SceneGraphFamilyTree::addRootNode(SceneNode* Object)
{
    if (Object)
        RootNodeList_.push_back(Object);
}
void SceneGraphFamilyTree::removeRootNode(SceneNode* Object)
{
    removeObjectFromList<SceneNode>(Object, RootNodeList_);
}

void SceneGraphFamilyTree::render()
{
    /* Update scene graph transformation */
    const dim::matrix4f BaseMatrix(getTransformMatrix(true) * spWorldMatrix);
    
    /* Update object transformation */
    foreach (SceneNode* Node, RootNodeList_)
    {
        spWorldMatrix = BaseMatrix;
        updateRootNode(Node);
    }
    
    /* Render objects */
    std::sort(RootNodeList_.begin(), RootNodeList_.end(), compareSceneNodes);
    
    foreach (SceneNode* Node, RootNodeList_)
    {
        spWorldMatrix.reset();
        renderRootNode(Node);
    }
}


/*
 * ======= Protected: =======
 */

void SceneGraphFamilyTree::updateRootNode(SceneNode* Object)
{
    if (!Object->getVisible())
        return;
    
    Object->updateTransformation();
    
    foreach (SceneNode* Node, Object->getSceneChildren())
        updateRootNode(Node);
}

void SceneGraphFamilyTree::renderRootNode(SceneNode* Object)
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
    std::sort(Object->getSceneChildren().begin(), Object->getSceneChildren().end(), compareSceneNodes);
    
    foreach (SceneNode* Node, Object->getSceneChildren())
        renderRootNode(Node);
}


} // /namespace scene

} // /namespace sp


#endif



// ================================================================================
