/*
 * Simple scene graph streaming header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_SCENEGRAPH_SIMPLE_STREAM_H__
#define __SP_SCENEGRAPH_SIMPLE_STREAM_H__


#include "Base/spStandard.hpp"

#ifdef SP_COMPILE_WITH_SCENEGRAPH_SIMPLE_STREAM


#include "SceneGraph/spSceneGraphSimple.hpp"


namespace sp
{
namespace scene
{


/**
This is thread-safe version of the default simple scene graph (SceneGraphSimple).
When new objects are hooked into the scene graph they will be streamed.
i.e. only when the render loop is over the new objects will be added to the root list
and the queue will be cleared.
\note Here you have to use only the second "renderScene" function which expects a Camera object.
\ingroup group_scenegraph
\since Version 3.0
*/
class SP_EXPORT SceneGraphSimpleStream : public SceneGraphSimple
{
    
    public:
        
        SceneGraphSimpleStream();
        virtual ~SceneGraphSimpleStream();
        
        /* Functions */
        
        void addSceneNode(SceneNode*    Object);
        void addSceneNode(Camera*       Object);
        void addSceneNode(Light*        Object);
        void addSceneNode(RenderNode*   Object);
        
        void removeSceneNode(SceneNode*     Object);
        void removeSceneNode(Camera*        Object);
        void removeSceneNode(Light*         Object);
        void removeSceneNode(RenderNode*    Object);
        
        virtual void render();
        
    protected:
        
        /* Members */
        
        std::list<SceneNode*>   QueueAddNodes_;
        std::list<Light*>       QueueAddLights_;
        std::list<Camera*>      QueueAddCameras_;
        std::list<RenderNode*>  QueueAddRenderNodes_;
        
        std::list<SceneNode*>   QueueRemoveNodes_;
        std::list<Light*>       QueueRemoveLights_;
        std::list<Camera*>      QueueRemoveCameras_;
        std::list<RenderNode*>  QueueRemoveRenderNodes_;
        
        CriticalSection SemaphoreQueues_;
        
};


} // /namespace scene

} // /namespace sp


#endif

#endif



// ================================================================================
