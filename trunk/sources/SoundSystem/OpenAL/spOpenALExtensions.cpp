/*
 * OpenAL extensions file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "SoundSystem/OpenAL/spOpenALExtensions.hpp"

#ifdef SP_COMPILE_WITH_OPENAL


namespace sp
{


/* Effect objects */

LPALGENEFFECTS      alGenEffects    = 0;
LPALDELETEEFFECTS   alDeleteEffects = 0;
LPALISEFFECT        alIsEffect      = 0;
LPALEFFECTI         alEffecti       = 0;
LPALEFFECTIV        alEffectiv      = 0;
LPALEFFECTF         alEffectf       = 0;
LPALEFFECTFV        alEffectfv      = 0;
LPALGETEFFECTI      alGetEffecti    = 0;
LPALGETEFFECTIV     alGetEffectiv   = 0;
LPALGETEFFECTF      alGetEffectf    = 0;
LPALGETEFFECTFV     alGetEffectfv   = 0;

/* Filter objects */

LPALGENFILTERS      alGenFilters    = 0;
LPALDELETEFILTERS   alDeleteFilters = 0;
LPALISFILTER        alIsFilter      = 0;
LPALFILTERI         alFilteri       = 0;
LPALFILTERIV        alFilteriv      = 0;
LPALFILTERF         alFilterf       = 0;
LPALFILTERFV        alFilterfv      = 0;
LPALGETFILTERI      alGetFilteri    = 0;
LPALGETFILTERIV     alGetFilteriv   = 0;
LPALGETFILTERF      alGetFilterf    = 0;
LPALGETFILTERFV     alGetFilterfv   = 0;

/* Auxiliary slot objects */

LPALGENAUXILIARYEFFECTSLOTS     alGenAuxiliaryEffectSlots       = 0;
LPALDELETEAUXILIARYEFFECTSLOTS  alDeleteAuxiliaryEffectSlots    = 0;
LPALISAUXILIARYEFFECTSLOT       alIsAuxiliaryEffectSlot         = 0;
LPALAUXILIARYEFFECTSLOTI        alAuxiliaryEffectSloti          = 0;
LPALAUXILIARYEFFECTSLOTIV       alAuxiliaryEffectSlotiv         = 0;
LPALAUXILIARYEFFECTSLOTF        alAuxiliaryEffectSlotf          = 0;
LPALAUXILIARYEFFECTSLOTFV       alAuxiliaryEffectSlotfv         = 0;
LPALGETAUXILIARYEFFECTSLOTI     alGetAuxiliaryEffectSloti       = 0;
LPALGETAUXILIARYEFFECTSLOTIV    alGetAuxiliaryEffectSlotiv      = 0;
LPALGETAUXILIARYEFFECTSLOTF     alGetAuxiliaryEffectSlotf       = 0;
LPALGETAUXILIARYEFFECTSLOTFV    alGetAuxiliaryEffectSlotfv      = 0;


} // /namespace sp


#endif


// ================================================================================
