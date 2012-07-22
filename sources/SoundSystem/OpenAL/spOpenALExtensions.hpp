/*
 * OpenAL extensions header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_AUDIO_OPENAL_EXTENSIONS_H__
#define __SP_AUDIO_OPENAL_EXTENSIONS_H__


#include "Base/spStandard.hpp"

#ifdef SP_COMPILE_WITH_OPENAL


#include <AL/al.h>
#include <AL/alc.h>
#include <AL/efx.h>
#include <AL/efx-creative.h>


namespace sp
{


/* Effect objects */

extern LPALGENEFFECTS       alGenEffects;
extern LPALDELETEEFFECTS    alDeleteEffects;
extern LPALISEFFECT         alIsEffect;
extern LPALEFFECTI          alEffecti;
extern LPALEFFECTIV         alEffectiv;
extern LPALEFFECTF          alEffectf;
extern LPALEFFECTFV         alEffectfv;
extern LPALGETEFFECTI       alGetEffecti;
extern LPALGETEFFECTIV      alGetEffectiv;
extern LPALGETEFFECTF       alGetEffectf;
extern LPALGETEFFECTFV      alGetEffectfv;

/* Filter objects */

extern LPALGENFILTERS       alGenFilters;
extern LPALDELETEFILTERS    alDeleteFilters;
extern LPALISFILTER         alIsFilter;
extern LPALFILTERI          alFilteri;
extern LPALFILTERIV         alFilteriv;
extern LPALFILTERF          alFilterf;
extern LPALFILTERFV         alFilterfv;
extern LPALGETFILTERI       alGetFilteri;
extern LPALGETFILTERIV      alGetFilteriv;
extern LPALGETFILTERF       alGetFilterf;
extern LPALGETFILTERFV      alGetFilterfv;

/* Auxiliary slot objects */

extern LPALGENAUXILIARYEFFECTSLOTS      alGenAuxiliaryEffectSlots;
extern LPALDELETEAUXILIARYEFFECTSLOTS   alDeleteAuxiliaryEffectSlots;
extern LPALISAUXILIARYEFFECTSLOT        alIsAuxiliaryEffectSlot;
extern LPALAUXILIARYEFFECTSLOTI         alAuxiliaryEffectSloti;
extern LPALAUXILIARYEFFECTSLOTIV        alAuxiliaryEffectSlotiv;
extern LPALAUXILIARYEFFECTSLOTF         alAuxiliaryEffectSlotf;
extern LPALAUXILIARYEFFECTSLOTFV        alAuxiliaryEffectSlotfv;
extern LPALGETAUXILIARYEFFECTSLOTI      alGetAuxiliaryEffectSloti;
extern LPALGETAUXILIARYEFFECTSLOTIV     alGetAuxiliaryEffectSlotiv;
extern LPALGETAUXILIARYEFFECTSLOTF      alGetAuxiliaryEffectSlotf;
extern LPALGETAUXILIARYEFFECTSLOTFV     alGetAuxiliaryEffectSlotfv;


} // /namespace sp


#endif

#endif



// ================================================================================
