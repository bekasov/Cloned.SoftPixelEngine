/*
 * Scene graph family tree header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_SCENEGRAPH_FAMILY_TREE_H__
#define __SP_SCENEGRAPH_FAMILY_TREE_H__


#include "Base/spStandard.hpp"

#ifdef SP_COMPILE_WITH_SCENEGRAPH_FAMILY_TREE


#include "SceneGraph/spSceneGraph.hpp"


namespace sp
{
namespace scene
{


/**
The SceneGraphFamilyTree is used for simple scenes with a child tree hierarchy. Each root object will be passed through
and all other objects ordered under will be passed in a tree hierarchy. Advantage is that the parent system does not affect
the performance and large tree hierarchies are no problems. Disadvantage is that depth sorting to avoid alpha blending
problems is more difficult.
*/
class SP_EXPORT SceneGraphFamilyTree : public SceneGraph
{
    
    public:
        
        SceneGraphFamilyTree();
        virtual ~SceneGraphFamilyTree();
        
        /* Functions */
        
        void addSceneNode(SceneNode*    Object);
        void addSceneNode(Camera*       Object);
        void addSceneNode(Light*        Object);
        void addSceneNode(RenderNode*   Object);
        
        void removeSceneNode(SceneNode*     Object);
        void removeSceneNode(Camera*        Object);
        void removeSceneNode(Light*         Object);
        void removeSceneNode(RenderNode*    Object);
        
        void addRootNode(SceneNode* Object);
        void removeRootNode(SceneNode* Object);
        
        virtual void render();
        
    protected:
        
        /* Functions */
        
        void updateRootNode(SceneNode* Object);
        void renderRootNode(SceneNode* Object);
        
        /* Members */
        
        std::list<SceneNode*> RootNodeList_;
        
};


} // /namespace scene

} // /namespace sp


#endif

#endif



// ================================================================================
