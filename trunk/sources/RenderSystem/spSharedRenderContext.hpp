/*
 * Shared render context header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_SHAREDRENDERCONTEXT_H__
#define __SP_SHAREDRENDERCONTEXT_H__


#include "Base/spStandard.hpp"


namespace sp
{
namespace video
{


//! Shared render context base class used for multi-threaded resource creation.
class SP_EXPORT SharedRenderContext
{
    
    public:
        
        SharedRenderContext()
        {
        }
        virtual ~SharedRenderContext()
        {
        }
        
        /* Functions */
        
        //! Activates the shared render context.
        virtual bool activate()
        {
            return false;
        }
        
        //! Deactivates the shared render context.
        virtual bool deactivate()
        {
            return false;
        }
        
};


} // /namespace video

} // /namespace sp


#endif



// ================================================================================
