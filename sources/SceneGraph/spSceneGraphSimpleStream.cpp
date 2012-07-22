/*
 * Simple scene graph streaming file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "SceneGraph/spSceneGraphSimpleStream.hpp"

#ifdef SP_COMPILE_WITH_SCENEGRAPH_SIMPLE_STREAM


#include <boost/foreach.hpp>


namespace sp
{
namespace scene
{


SceneGraphSimpleStream::SceneGraphSimpleStream() :
    SceneGraphSimple()
{
    GraphType_ = SCENEGRAPH_SIMPLE_STREAM;
}
SceneGraphSimpleStream::~SceneGraphSimpleStream()
{
}

#define SEMAPHORE(l)                \
    if (Object)                     \
    {                               \
        SemaphoreQueues_.lock();    \
        l.push_back(Object);        \
        SemaphoreQueues_.unlock();  \
    }

void SceneGraphSimpleStream::addSceneNode(SceneNode* Object)
{
    SEMAPHORE(QueueAddNodes_)
}
void SceneGraphSimpleStream::removeSceneNode(SceneNode* Object)
{
    SEMAPHORE(QueueRemoveNodes_)
}

void SceneGraphSimpleStream::addSceneNode(Camera* Object)
{
    SEMAPHORE(QueueAddCameras_)
}
void SceneGraphSimpleStream::removeSceneNode(Camera* Object)
{
    SEMAPHORE(QueueRemoveCameras_)
}

void SceneGraphSimpleStream::addSceneNode(Light* Object)
{
    SEMAPHORE(QueueAddLights_)
}
void SceneGraphSimpleStream::removeSceneNode(Light* Object)
{
    SEMAPHORE(QueueRemoveLights_)
}

void SceneGraphSimpleStream::addSceneNode(RenderNode* Object)
{
    SEMAPHORE(QueueAddRenderNodes_)
}
void SceneGraphSimpleStream::removeSceneNode(RenderNode* Object)
{
    SEMAPHORE(QueueRemoveRenderNodes_)
}

#undef SEMAPHORE

void SceneGraphSimpleStream::render()
{
    #define QUEUE_ADD(t, l, q)                      \
        if (!q.empty())                             \
        {                                           \
            l.insert(l.end(), q.begin(), q.end());  \
            q.clear();                              \
        }
    
    #define QUEUE_REMOVE(t, l, q)                   \
        if (!q.empty())                             \
        {                                           \
            foreach (t* Object, q)                  \
                removeObjectFromList<t>(Object, l); \
            q.clear();                              \
        }
    
    /* Render scene in default way */
    SceneGraphSimple::render();
    
    /* Start streaming objects */
    SemaphoreQueues_.lock();
    
    QUEUE_ADD   (SceneNode, NodeList_, QueueAddNodes_)
    QUEUE_REMOVE(SceneNode, NodeList_, QueueRemoveNodes_)
    
    QUEUE_ADD   (Light, LightList_, QueueAddLights_)
    QUEUE_REMOVE(Light, LightList_, QueueRemoveLights_)
    
    QUEUE_ADD   (Camera, CameraList_, QueueAddCameras_)
    QUEUE_REMOVE(Camera, CameraList_, QueueRemoveCameras_)
    
    QUEUE_ADD   (RenderNode, RenderList_, QueueAddRenderNodes_)
    QUEUE_REMOVE(RenderNode, RenderList_, QueueRemoveRenderNodes_)
    
    SemaphoreQueues_.unlock();
    
    #undef QUEUE_ADD
    #undef QUEUE_REMOVE
}


} // /namespace scene

} // /namespace sp


#endif



// ================================================================================
