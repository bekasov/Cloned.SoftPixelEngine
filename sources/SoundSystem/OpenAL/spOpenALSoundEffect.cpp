/*
 * OpenAL sound effect file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "SoundSystem/OpenAL/spOpenALSoundEffect.hpp"

#ifdef SP_COMPILE_WITH_OPENAL


#include "Base/spInputOutputLog.hpp"
#include "SoundSystem/OpenAL/spOpenALExtensions.hpp"


#if defined(SP_PLATFORM_WINDOWS)
#   include <AL/EFX-Util.h>
#else
#   include <AL/efx-presets.h>
#endif


namespace sp
{
namespace audio
{


/*
 * Internal members
 */

static const ALenum ALEffectTypeList[] =
{
    AL_EFFECT_EAXREVERB,
    AL_EFFECT_CHORUS,
    AL_EFFECT_DISTORTION,
    AL_EFFECT_ECHO,
    AL_EFFECT_FLANGER,
    AL_EFFECT_FREQUENCY_SHIFTER,
    AL_EFFECT_VOCAL_MORPHER,
    AL_EFFECT_PITCH_SHIFTER,
    AL_EFFECT_RING_MODULATOR,
    AL_EFFECT_AUTOWAH,
    AL_EFFECT_COMPRESSOR,
    AL_EFFECT_EQUALIZER,
};
/*
#if defined(SP_PLATFORM_WINDOWS)
static const EAXREVERBPROPERTIES ALReverbPropertyList[] =
#else
static const EFXEAXREVERBPROPERTIES ALReverbPropertyList[] =
#endif
{
    REVERB_PRESET_GENERIC,
    REVERB_PRESET_PADDEDCELL,
    REVERB_PRESET_ROOM,
    REVERB_PRESET_BATHROOM,
    REVERB_PRESET_LIVINGROOM,
    REVERB_PRESET_STONEROOM,
    REVERB_PRESET_AUDITORIUM,
    REVERB_PRESET_CONCERTHALL,
    REVERB_PRESET_CAVE,
    REVERB_PRESET_ARENA,
    REVERB_PRESET_HANGAR,
    REVERB_PRESET_CARPETTEDHALLWAY,
    REVERB_PRESET_HALLWAY,
    REVERB_PRESET_STONECORRIDOR,
    REVERB_PRESET_ALLEY,
    REVERB_PRESET_FOREST,
    REVERB_PRESET_CITY,
    REVERB_PRESET_MOUNTAINS,
    REVERB_PRESET_QUARRY,
    REVERB_PRESET_PLAIN,
    REVERB_PRESET_PARKINGLOT,
    REVERB_PRESET_SEWERPIPE,
    REVERB_PRESET_UNDERWATER,
    REVERB_PRESET_DRUGGED,
    REVERB_PRESET_DIZZY,
    REVERB_PRESET_PSYCHOTIC,
    
    REVERB_PRESET_CASTLE_SMALLROOM,
    REVERB_PRESET_CASTLE_SHORTPASSAGE,
    REVERB_PRESET_CASTLE_MEDIUMROOM,
    REVERB_PRESET_CASTLE_LONGPASSAGE,
    REVERB_PRESET_CASTLE_LARGEROOM,
    REVERB_PRESET_CASTLE_HALL,
    REVERB_PRESET_CASTLE_CUPBOARD,
    REVERB_PRESET_CASTLE_COURTYARD,
    REVERB_PRESET_CASTLE_ALCOVE,
    
    REVERB_PRESET_FACTORY_ALCOVE,
    REVERB_PRESET_FACTORY_SHORTPASSAGE,
    REVERB_PRESET_FACTORY_MEDIUMROOM,
    REVERB_PRESET_FACTORY_LONGPASSAGE,
    REVERB_PRESET_FACTORY_LARGEROOM,
    REVERB_PRESET_FACTORY_HALL,
    REVERB_PRESET_FACTORY_CUPBOARD,
    REVERB_PRESET_FACTORY_COURTYARD,
    REVERB_PRESET_FACTORY_SMALLROOM,
    
    REVERB_PRESET_ICEPALACE_ALCOVE,
    REVERB_PRESET_ICEPALACE_SHORTPASSAGE,
    REVERB_PRESET_ICEPALACE_MEDIUMROOM,
    REVERB_PRESET_ICEPALACE_LONGPASSAGE,
    REVERB_PRESET_ICEPALACE_LARGEROOM,
    REVERB_PRESET_ICEPALACE_HALL,
    REVERB_PRESET_ICEPALACE_CUPBOARD,
    REVERB_PRESET_ICEPALACE_COURTYARD,
    REVERB_PRESET_ICEPALACE_SMALLROOM,
    
    REVERB_PRESET_SPACESTATION_ALCOVE,
    REVERB_PRESET_SPACESTATION_MEDIUMROOM,
    REVERB_PRESET_SPACESTATION_SHORTPASSAGE,
    REVERB_PRESET_SPACESTATION_LONGPASSAGE,
    REVERB_PRESET_SPACESTATION_LARGEROOM,
    REVERB_PRESET_SPACESTATION_HALL,
    REVERB_PRESET_SPACESTATION_CUPBOARD,
    REVERB_PRESET_SPACESTATION_SMALLROOM,
    
    REVERB_PRESET_WOODEN_ALCOVE,
    REVERB_PRESET_WOODEN_SHORTPASSAGE,
    REVERB_PRESET_WOODEN_MEDIUMROOM,
    REVERB_PRESET_WOODEN_LONGPASSAGE,
    REVERB_PRESET_WOODEN_LARGEROOM,
    REVERB_PRESET_WOODEN_HALL,
    REVERB_PRESET_WOODEN_CUPBOARD,
    REVERB_PRESET_WOODEN_SMALLROOM,
    REVERB_PRESET_WOODEN_COURTYARD,
    
    REVERB_PRESET_SPORT_EMPTYSTADIUM,
    REVERB_PRESET_SPORT_SQUASHCOURT,
    REVERB_PRESET_SPORT_SMALLSWIMMINGPOOL,
    REVERB_PRESET_SPORT_LARGESWIMMINGPOOL,
    REVERB_PRESET_SPORT_GYMNASIUM,
    REVERB_PRESET_SPORT_FULLSTADIUM,
    REVERB_PRESET_SPORT_STADIUMTANNOY,
    
    REVERB_PRESET_PREFAB_WORKSHOP,
    REVERB_PRESET_PREFAB_SCHOOLROOM,
    REVERB_PRESET_PREFAB_PRACTISEROOM,
    REVERB_PRESET_PREFAB_OUTHOUSE,
    REVERB_PRESET_PREFAB_CARAVAN,
    
    REVERB_PRESET_DOME_TOMB,
    REVERB_PRESET_PIPE_SMALL,
    REVERB_PRESET_DOME_SAINTPAULS,
    REVERB_PRESET_PIPE_LONGTHIN,
    REVERB_PRESET_PIPE_LARGE,
    REVERB_PRESET_PIPE_RESONANT,
    
    REVERB_PRESET_OUTDOORS_BACKYARD,
    REVERB_PRESET_OUTDOORS_ROLLINGPLAINS,
    REVERB_PRESET_OUTDOORS_DEEPCANYON,
    REVERB_PRESET_OUTDOORS_CREEK,
    REVERB_PRESET_OUTDOORS_VALLEY,
    
    REVERB_PRESET_MOOD_HEAVEN,
    REVERB_PRESET_MOOD_HELL,
    REVERB_PRESET_MOOD_MEMORY,
    
    REVERB_PRESET_DRIVING_COMMENTATOR,
    REVERB_PRESET_DRIVING_PITGARAGE,
    REVERB_PRESET_DRIVING_INCAR_RACER,
    REVERB_PRESET_DRIVING_INCAR_SPORTS,
    REVERB_PRESET_DRIVING_INCAR_LUXURY,
    REVERB_PRESET_DRIVING_FULLGRANDSTAND,
    REVERB_PRESET_DRIVING_EMPTYGRANDSTAND,
    REVERB_PRESET_DRIVING_TUNNEL,
    
    REVERB_PRESET_CITY_STREETS,
    REVERB_PRESET_CITY_SUBWAY,
    REVERB_PRESET_CITY_MUSEUM,
    REVERB_PRESET_CITY_LIBRARY,
    REVERB_PRESET_CITY_UNDERPASS,
    REVERB_PRESET_CITY_ABANDONED,
    
    REVERB_PRESET_DUSTYROOM,
    REVERB_PRESET_CHAPEL,
    REVERB_PRESET_SMALLWATERROOM,
};
*/

/*
 * OpenALSoundEffect class functions
 */

OpenALSoundEffect::OpenALSoundEffect() :
    SoundEffect (   ),
    ALEffect_   (0  )
{
    /* Check for extension support */
    if (!alGenAuxiliaryEffectSlots || !alGenEffects)
    {
        io::Log::error("OpenAL EFX extensions are not supported");
        return;
    }
    
    /* Create OpenAL effect object */
    alGenEffects(1, &ALEffect_);
    
    if (alGetError() != AL_NO_ERROR)
    {
        io::Log::error("Could not create OpenAL effect object");
        ALEffect_ = 0;
    }
    
    /* Setup default effect type */
    setType(SOUNDEFFECT_REVERBERATION);
}
OpenALSoundEffect::~OpenALSoundEffect()
{
    /* Delete OpenAL effect objects */
    if (ALEffect_)
        alDeleteEffects(1, &ALEffect_);
}

void OpenALSoundEffect::setType(const ESoundEffectTypes Type)
{
    SoundEffect::setType(Type);
    
    /* Setup effect type */
    alEffecti(ALEffect_, AL_EFFECT_TYPE, ALEffectTypeList[Type_]);
}

void OpenALSoundEffect::setupEffectPreset(const ESoundEffectPresets Preset)
{
    switch (Type_)
    {
        /*case SOUNDEFFECT_REVERBERATION:
        {

            EAXREVERBPROPERTIES ReverbProperties = ALReverbPropertyList[Preset];
            EFXEAXREVERBPROPERTIES EffectSettings;
            
            ConvertReverbParameters(&ReverbProperties, &EffectSettings);
            

            alEffectf   (ALEffect_, AL_EAXREVERB_DENSITY,               EffectSettings.flDensity            );
            alEffectf   (ALEffect_, AL_EAXREVERB_DIFFUSION,             EffectSettings.flDiffusion          );
            alEffectf   (ALEffect_, AL_EAXREVERB_GAIN,                  EffectSettings.flGain               );
            alEffectf   (ALEffect_, AL_EAXREVERB_GAINHF,                EffectSettings.flGainHF             );
            alEffectf   (ALEffect_, AL_EAXREVERB_GAINLF,                EffectSettings.flGainLF             );
            alEffectf   (ALEffect_, AL_EAXREVERB_DECAY_TIME,            EffectSettings.flDecayTime          );
            alEffectf   (ALEffect_, AL_EAXREVERB_DECAY_HFRATIO,         EffectSettings.flDecayHFRatio       );
            alEffectf   (ALEffect_, AL_EAXREVERB_DECAY_LFRATIO,         EffectSettings.flDecayLFRatio       );
            alEffectf   (ALEffect_, AL_EAXREVERB_REFLECTIONS_GAIN,      EffectSettings.flReflectionsGain    );
            alEffectf   (ALEffect_, AL_EAXREVERB_REFLECTIONS_DELAY,     EffectSettings.flReflectionsDelay   );
            alEffectfv  (ALEffect_, AL_EAXREVERB_REFLECTIONS_PAN,       EffectSettings.flReflectionsPan     );
            alEffectf   (ALEffect_, AL_EAXREVERB_LATE_REVERB_GAIN,      EffectSettings.flLateReverbGain     );
            alEffectf   (ALEffect_, AL_EAXREVERB_LATE_REVERB_DELAY,     EffectSettings.flLateReverbDelay    );
            alEffectfv  (ALEffect_, AL_EAXREVERB_LATE_REVERB_PAN,       EffectSettings.flLateReverbPan      );
            alEffectf   (ALEffect_, AL_EAXREVERB_ECHO_TIME,             EffectSettings.flEchoTime           );
            alEffectf   (ALEffect_, AL_EAXREVERB_ECHO_DEPTH,            EffectSettings.flEchoDepth          );
            alEffectf   (ALEffect_, AL_EAXREVERB_MODULATION_TIME,       EffectSettings.flModulationTime     );
            alEffectf   (ALEffect_, AL_EAXREVERB_MODULATION_DEPTH,      EffectSettings.flModulationDepth    );
            alEffectf   (ALEffect_, AL_EAXREVERB_AIR_ABSORPTION_GAINHF, EffectSettings.flAirAbsorptionGainHF);
            alEffectf   (ALEffect_, AL_EAXREVERB_HFREFERENCE,           EffectSettings.flHFReference        );
            alEffectf   (ALEffect_, AL_EAXREVERB_LFREFERENCE,           EffectSettings.flLFReference        );
            alEffectf   (ALEffect_, AL_EAXREVERB_ROOM_ROLLOFF_FACTOR,   EffectSettings.flRoomRolloffFactor  );
            alEffecti   (ALEffect_, AL_EAXREVERB_DECAY_HFLIMIT,         EffectSettings.iDecayHFLimit        );
        }
        break;
        */
        default:
            io::Log::warning("Specified sound effect is currently not supported for OpenAL sound system");
            return;
    }
    
    /* Bind effect to effect slot */
    if (alGetError() != AL_NO_ERROR)
        io::Log::error("Could not setup effect properties");
}


} // /namespace audio

} // /namespace sp


#endif



// ================================================================================
