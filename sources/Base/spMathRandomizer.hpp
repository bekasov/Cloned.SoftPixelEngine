/*
 * Randomizer header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_MATH_RANDOMIZER_H__
#define __SP_MATH_RANDOMIZER_H__


#include "Base/spStandard.hpp"
#include "Base/spMaterialColor.hpp"
#include "Base/spDimensionVector3D.hpp"


namespace sp
{
namespace math
{


//! Namepsace for all random-generation functions
namespace Randomizer
{

/**
Seeds a random to have always other random values when the program starts.
\param isTimeBase: Specifies whether the current time shall be used as the base for further random value generations.
*/
SP_EXPORT void seedRandom(bool isTimeBase = true);

//! Returns a random value in the range [0 .. RAND_MAX].
SP_EXPORT s32 randInt();

//! Returns a random value in the range [0 .. Max]. This value can also be negative.
SP_EXPORT s32 randInt(const s32 Max);

//! Returns a random value in the range [Min .. Max]. These values can also be negative.
SP_EXPORT s32 randInt(const s32 Min, const s32 Max);

//! Returns true if the specified probability occurs. This is equivalent to "randInt(Probability) == 0".
SP_EXPORT bool randBool(const u32 Probability);

//! Returns a random value in the range [0.0 .. 1.0].
SP_EXPORT f32 randFloat();

//! Returns a floating point random value in the range [0.0 .. Max].
SP_EXPORT f32 randFloat(const f32 Max);

//! Returns a floating point random value in the range [Min .. Max].
SP_EXPORT f32 randFloat(const f32 Min, const f32 Max);

//! Returns a random color. Alpha is always 255.
SP_EXPORT video::color randColor();

//! Returns a random direction vector. This vector is always normalized.
SP_EXPORT dim::vector3df randVector();

} // /namespace CollisionLibrary


} // /namespace math

} // /namespace sp


#endif



// ================================================================================
