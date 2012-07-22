/*
 * Internal declarations header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_INTERNALDECLARATIONS_H__
#define __SP_INTERNALDECLARATIONS_H__


#include "Base/spStandard.hpp"


namespace sp
{


static const s32 MAX_COUNT_OF_KEYCODES  = 256;
static const s32 MAX_COUNT_OF_MOUSEKEYS = 3;

/* Internal global variables */

extern bool __isKey[MAX_COUNT_OF_KEYCODES], __wasKey[MAX_COUNT_OF_KEYCODES], __hitKey[MAX_COUNT_OF_KEYCODES];
extern bool __isMouseKey[MAX_COUNT_OF_MOUSEKEYS], __wasMouseKey[MAX_COUNT_OF_MOUSEKEYS], __hitMouseKey[MAX_COUNT_OF_MOUSEKEYS];

extern bool __isLighting, __isFog, __isTexturing;

extern f32 __GeneralRunTime;


} // /namespace sp


#endif



// ================================================================================
