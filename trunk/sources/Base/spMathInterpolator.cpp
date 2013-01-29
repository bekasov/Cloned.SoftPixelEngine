/*
 * Interpolator file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "Base/spMathInterpolator.hpp"
#include "Base/spMemoryManagement.hpp"


namespace sp
{
namespace math
{


namespace InterpolatorManager
{

static std::list<Interpolator*> InterpList;

SP_EXPORT void add(Interpolator* Interp)
{
    if (Interp && !MemoryManager::hasElement(InterpList, Interp))
        InterpList.push_back(Interp);
}

SP_EXPORT void remove(Interpolator* Interp)
{
    MemoryManager::removeElement(InterpList, Interp);
}

SP_EXPORT void update()
{
    for (std::list<Interpolator*>::iterator it = InterpList.begin(); it != InterpList.end();)
    {
        (*it)->update();
        
        if ((*it)->playing())
            ++it;
        else
            it = InterpList.erase(it);
    }
}

} // /namespace InterpolatorManager


} // /namespace math

} // /namespace sp



// ================================================================================
