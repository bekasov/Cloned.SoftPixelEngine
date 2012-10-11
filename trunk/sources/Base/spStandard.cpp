/*
 * Standard definitions file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "Base/spStandard.hpp"
#include "Base/spInternalDeclarations.hpp"
#include "Base/spSharedObjects.hpp"


namespace sp
{


/*
 * Global members
 */

SSharedObjects gSharedObjects;

bool __isKey    [MAX_COUNT_OF_KEYCODES] = { 0 };
bool __wasKey   [MAX_COUNT_OF_KEYCODES] = { 0 };
bool __hitKey   [MAX_COUNT_OF_KEYCODES] = { 0 };

bool __isMouseKey   [MAX_COUNT_OF_MOUSEKEYS] = { 0 };
bool __wasMouseKey  [MAX_COUNT_OF_MOUSEKEYS] = { 0 };
bool __hitMouseKey  [MAX_COUNT_OF_MOUSEKEYS] = { 0 };
bool __dbclkMouseKey[MAX_COUNT_OF_MOUSEKEYS] = { 0 };

bool __isLighting = false, __isFog = false, __isTexturing = true;

f32 __GeneralRunTime = 1.0f;


} // /namespace sp



// ================================================================================
