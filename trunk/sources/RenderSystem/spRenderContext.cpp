/*
 * Render context file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "RenderSystem/spRenderContext.hpp"
#include "RenderSystem/spSharedRenderContext.hpp"
#include "Base/spMemoryManagement.hpp"
#include "Base/spInputOutputControl.hpp"


namespace sp
{

extern io::InputControl* __spInputControl;

namespace video
{


RenderContext* RenderContext::ActiveRenderContext_ = 0;

RenderContext::RenderContext() :
    ParentWindow_   (0      ),
    ColorDepth_     (0      ),
    isFullscreen_   (false  )
{
    resetConfig();
}
RenderContext::~RenderContext()
{
    MemoryManager::deleteList(SharedContextList_);
}

void RenderContext::setWindowTitle(const io::stringc &Title)
{
    Title_ = Title;
}
io::stringc RenderContext::getWindowTitle() const
{
    return Title_;
}

void RenderContext::setWindowPosition(const dim::point2di &Position)
{
    // do nothing
}
dim::point2di RenderContext::getWindowPosition() const
{
    return 0;
}

dim::size2di RenderContext::getWindowSize() const
{
    return Resolution_;
}
dim::size2di RenderContext::getWindowBorder() const
{
    return 0;
}

bool RenderContext::isWindowActive() const
{
    return true;
}

SharedRenderContext* RenderContext::createSharedContext()
{
    SharedRenderContext* NewSharedContext = MemoryManager::createMemory<SharedRenderContext>("SharedRenderContext");
    SharedContextList_.push_back(NewSharedContext);
    return NewSharedContext;
}
void RenderContext::deleteSharedContext(SharedRenderContext* SharedContext)
{
    MemoryManager::removeElement(SharedContextList_, SharedContext, true);
}

void RenderContext::setFullscreen(bool Enable)
{
    io::Log::warning("Fullscreen mode switch not supported for this render system");
}


/*
 * ======= Protectd: =======
 */

void RenderContext::resetConfig()
{
    /* Reset all configuration */
    ParentWindow_   = 0;
    ColorDepth_     = 0;
    isFullscreen_   = false;
    Resolution_     = dim::size2di(0);
    Flags_          = SDeviceFlags();
}

RenderContext* RenderContext::getActiveRenderContext()
{
    return RenderContext::ActiveRenderContext_;
}

void RenderContext::setActiveRenderContext(RenderContext* Context)
{
    /* Update previous cursor position for correct cursor speed determination */
    if (__spInputControl && RenderContext::ActiveRenderContext_)
    {
        __spInputControl->updatePrevCursorPosition(
            RenderContext::ActiveRenderContext_->getWindowPosition() - Context->getWindowPosition()
        );
    }
    
    /* Activate new render context */
    RenderContext::ActiveRenderContext_ = Context;
}


} // /namespace video

} // /namespace sp



// ================================================================================
