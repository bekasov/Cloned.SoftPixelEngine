/*
 * Simple scene graph header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_SCENEGRAPH_SIMPLE_H__
#define __SP_SCENEGRAPH_SIMPLE_H__


#include "Base/spStandard.hpp"

#ifdef SP_COMPILE_WITH_SCENEGRAPH_SIMPLE


#include "SceneGraph/spSceneGraph.hpp"


namespace sp
{
namespace scene
{


/**
The SceneGraphSimple is used for (as the name implies) simple scenes. Each object will be passed through
and no kind of tree hierarchy optimization will be carried out.
*/
class SP_EXPORT SceneGraphSimple : public SceneGraph
{
    
    public:
        
        SceneGraphSimple();
        virtual ~SceneGraphSimple();
        
        /* Functions */
        
        virtual void render();
        
};


} // /namespace scene

} // /namespace sp


#endif

#endif



// ================================================================================
