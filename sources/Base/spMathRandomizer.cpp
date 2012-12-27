/*
 * Randomizer file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "Base/spMathRandomizer.hpp"

#include <stdlib.h>
#include <time.h>


namespace sp
{
namespace math
{


namespace Randomizer
{

SP_EXPORT void seedRandom(bool isTimeBase)
{
    srand(isTimeBase ? static_cast<u32>(time(0)) : 1);
}

SP_EXPORT s32 randInt()
{
    return rand();
}

SP_EXPORT s32 randInt(const s32 Max)
{
    if (Max < 0)
        return -( rand() % (1 - Max) );
    return rand() % (1 + Max);
}

SP_EXPORT s32 randInt(const s32 Min, const s32 Max)
{
    return Min + randInt(Max - Min);
}

SP_EXPORT bool randBool(const u32 Probability)
{
    return randInt(static_cast<s32>(Probability)) == 0;
}

SP_EXPORT f32 randFloat()
{
    return f32(rand()) / f32(RAND_MAX);
}

SP_EXPORT f32 randFloat(const f32 Max)
{
    return randFloat() * Max;
}

SP_EXPORT f32 randFloat(const f32 Min, const f32 Max)
{
    return Min + randFloat() * (Max - Min);
}

SP_EXPORT video::color randColor()
{
    return video::color(randInt(255), randInt(255), randInt(255));
}

SP_EXPORT dim::vector3df randVector()
{
    dim::vector3df Vec(
        randFloat(-1.0f, 1.0f), randFloat(-1.0f, 1.0f), randFloat(-1.0f, 1.0f)
    );
    
    if (Vec.empty())
        return dim::vector3df(0.0f, 0.0f, 1.0f);
    
    return Vec.normalize();
}

} // /namespace Randomizer


} // /namespace math

} // /namespace sp



// ================================================================================
