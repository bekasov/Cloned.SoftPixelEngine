/*
 * OpenAL sound device file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "SoundSystem/OpenAL/spOpenALSoundDevice.hpp"

#ifdef SP_COMPILE_WITH_OPENAL


#include "SoundSystem/OpenAL/spOpenALExtensions.hpp"
#include "SoundSystem/OpenAL/spOpenALSoundEffect.hpp"
#include "SoundSystem/OpenAL/spOpenALBufferObject.hpp"

#include <boost/foreach.hpp>


namespace sp
{
namespace audio
{


const f32 OpenALSoundDevice::DEFAULT_SOUND_SPEED = 343.3f;

OpenALSoundDevice::AudioBufferType OpenALSoundDevice::AudioBufferMap_;

OpenALSoundDevice::OpenALSoundDevice() :
    SoundDevice     (SOUNDDEVICE_OPENAL ),
    ALDevice_       (0                  ),
    ALContext_      (0                  ),
    ALEffectSlot_   (AL_EFFECT_NULL     ),
    HasExtensions_  (false              )
{
    /* Start OpenAL device */
    openALDevice();
    HasExtensions_ = loadExtensions();
    
    /* Create OpenAL effect slot */
    alGenAuxiliaryEffectSlots(1, &ALEffectSlot_);
    
    if (alGetError() != AL_NO_ERROR)
    {
        io::Log::warning("No OpenAL auxiliary effect slot available");
        ALEffectSlot_ = AL_EFFECT_NULL;
    }
    
    /* Initialize listener */
    setListenerOrientation(dim::matrix4f());
}
OpenALSoundDevice::~OpenALSoundDevice()
{
    /* Delete audio objects and buffers */
    MemoryManager::deleteList(SoundList_);
    MemoryManager::deleteList(SoundEffectList_);
    
    for (AudioBufferType::iterator it = OpenALSoundDevice::AudioBufferMap_.begin();
         it != OpenALSoundDevice::AudioBufferMap_.end(); ++it)
    {
        MemoryManager::deleteMemory(it->second);
    }
    
    /* Create OpenAL effect slot */
    if (ALEffectSlot_)
        alDeleteAuxiliaryEffectSlots(1, &ALEffectSlot_);
    
    /* Stop OpenAL device */
    closeALDevice();
}

io::stringc OpenALSoundDevice::getInterface() const
{
    return "OpenAL " + io::stringc(alGetString(AL_VERSION));
}

Sound* OpenALSoundDevice::createSound()
{
    OpenALSound* NewSound = new OpenALSound();
    SoundList_.push_back(NewSound);
    return NewSound;
}

SoundEffect* OpenALSoundDevice::createSoundEffect()
{
    OpenALSoundEffect* NewSoundEffect = new OpenALSoundEffect();
    SoundEffectList_.push_back(NewSoundEffect);
    return NewSoundEffect;
}

void OpenALSoundDevice::updateSounds()
{
    // do nothing -> sounds will be updated by the OpenAL implementation
}

void OpenALSoundDevice::setListenerPosition(const dim::vector3df &Position)
{
    alListenerfv(AL_POSITION, &Position.X);
}
void OpenALSoundDevice::setListenerVelocity(const dim::vector3df &Velocity)
{
    alListenerfv(AL_VELOCITY, &Velocity.X);
}
void OpenALSoundDevice::setListenerOrientation(const dim::matrix4f &Orientation)
{
    f32 Mat[6];
    
    *((dim::vector3df*)&Mat[0]) = Orientation * dim::vector3df(0, 0, -1);
    *((dim::vector3df*)&Mat[3]) = Orientation * dim::vector3df(0, 1,  0);
    
    alListenerfv(AL_ORIENTATION, Mat);
}

void OpenALSoundDevice::setListenerSpeed(f32 Speed)
{
    SoundDevice::setListenerSpeed(Speed);
    //alSpeedOfSound(OpenALSoundDevice::DEFAULT_SOUND_SPEED * Speed);
    
    foreach (Sound* Obj, SoundList_)
        Obj->setSpeed(Speed);
}

void OpenALSoundDevice::setEffectSlot(SoundEffect* Sfx)
{
    SoundDevice::setEffectSlot(Sfx);
    
    /* Bind or unbind effect from effect slot */
    if (ALEffectSlot_)
    {
        alAuxiliaryEffectSloti(
            ALEffectSlot_, AL_EFFECTSLOT_EFFECT,
            Sfx ? static_cast<OpenALSoundEffect*>(Sfx)->ALEffect_ : AL_EFFECT_NULL
        );
        
        if (alGetError() != AL_NO_ERROR)
            io::Log::error("Could not setup effect slot");
    }
}


/*
 * ======= Private: =======
 */

bool OpenALSoundDevice::openALDevice()
{
    /* Initialize OpenAL */
    ALDevice_ = alcOpenDevice(0);
    
    if (!ALDevice_)
    {
        io::Log::error("Could not create OpenAL device");
        return false;
    }
    
    /* Create OpenAL context */
    ALContext_ = alcCreateContext(ALDevice_, 0);
    
    if (!ALContext_)
    {
        io::Log::error("Could not create OpenAL context");
        return false;
    }
    
    /* Activate OpenAL context */
    if (alcMakeContextCurrent(ALContext_) == ALC_FALSE)
    {
        io::Log::error("Could not activate OpenAL context");
        return false;
    }
    
    return true;
}

void OpenALSoundDevice::closeALDevice()
{
    /* Release OpenAL context */
    alcMakeContextCurrent(0);
    alcDestroyContext(ALContext_);
    ALContext_ = 0;
    
    /* UnInitialize OpenAL */
    alcCloseDevice(ALDevice_);
    ALDevice_ = 0;
}

bool OpenALSoundDevice::loadExtensions()
{
    #define LOADOPENALPROC(o, t, n)                                                         \
        o = (t)alGetProcAddress(n);                                                         \
        if (!o)                                                                             \
        {                                                                                   \
            io::Log::error("Could not load OpenAL function \"" + io::stringc(n) + "\"");    \
            return false;                                                                   \
        }
    
    if (!alcIsExtensionPresent(ALDevice_, (ALCchar*)ALC_EXT_EFX_NAME))
    {
        io::Log::message("No OpenAL extensions supported");
        return false;
    }
    
    /* Effect objects */
    
    LOADOPENALPROC(alGenEffects,    LPALGENEFFECTS,     "alGenEffects"      )
    LOADOPENALPROC(alDeleteEffects, LPALDELETEEFFECTS,  "alDeleteEffects"   )
    LOADOPENALPROC(alIsEffect,      LPALISEFFECT,       "alIsEffect"        )
    LOADOPENALPROC(alEffecti,       LPALEFFECTI,        "alEffecti"         )
    LOADOPENALPROC(alEffectiv,      LPALEFFECTIV,       "alEffectiv"        )
    LOADOPENALPROC(alEffectf,       LPALEFFECTF,        "alEffectf"         )
    LOADOPENALPROC(alEffectfv,      LPALEFFECTFV,       "alEffectfv"        )
    LOADOPENALPROC(alGetEffecti,    LPALGETEFFECTI,     "alGetEffecti"      )
    LOADOPENALPROC(alGetEffectiv,   LPALGETEFFECTIV,    "alGetEffectiv"     )
    LOADOPENALPROC(alGetEffectf,    LPALGETEFFECTF,     "alGetEffectf"      )
    LOADOPENALPROC(alGetEffectfv,   LPALGETEFFECTFV,    "alGetEffectfv"     )
    
    /* Filter objects */
    
    LOADOPENALPROC(alGenFilters,    LPALGENFILTERS,     "alGenFilters"      )
    LOADOPENALPROC(alDeleteFilters, LPALDELETEFILTERS,  "alDeleteFilters"   )
    LOADOPENALPROC(alIsFilter,      LPALISFILTER,       "alIsFilter"        )
    LOADOPENALPROC(alFilteri,       LPALFILTERI,        "alFilteri"         )
    LOADOPENALPROC(alFilteriv,      LPALFILTERIV,       "alFilteriv"        )
    LOADOPENALPROC(alFilterf,       LPALFILTERF,        "alFilterf"         )
    LOADOPENALPROC(alFilterfv,      LPALFILTERFV,       "alFilterfv"        )
    LOADOPENALPROC(alGetFilteri,    LPALGETFILTERI,     "alGetFilteri"      )
    LOADOPENALPROC(alGetFilteriv,   LPALGETFILTERIV,    "alGetFilteriv"     )
    LOADOPENALPROC(alGetFilterf,    LPALGETFILTERF,     "alGetFilterf"      )
    LOADOPENALPROC(alGetFilterfv,   LPALGETFILTERFV,    "alGetFilterfv"     )
    
    /* Auxiliary slot objects */
    
    LOADOPENALPROC(alGenAuxiliaryEffectSlots,       LPALGENAUXILIARYEFFECTSLOTS,    "alGenAuxiliaryEffectSlots"     )
    LOADOPENALPROC(alDeleteAuxiliaryEffectSlots,    LPALDELETEAUXILIARYEFFECTSLOTS, "alDeleteAuxiliaryEffectSlots"  )
    LOADOPENALPROC(alIsAuxiliaryEffectSlot,         LPALISAUXILIARYEFFECTSLOT,      "alIsAuxiliaryEffectSlot"       )
    LOADOPENALPROC(alAuxiliaryEffectSloti,          LPALAUXILIARYEFFECTSLOTI,       "alAuxiliaryEffectSloti"        )
    LOADOPENALPROC(alAuxiliaryEffectSlotiv,         LPALAUXILIARYEFFECTSLOTIV,      "alAuxiliaryEffectSlotiv"       )
    LOADOPENALPROC(alAuxiliaryEffectSlotf,          LPALAUXILIARYEFFECTSLOTF,       "alAuxiliaryEffectSlotf"        )
    LOADOPENALPROC(alAuxiliaryEffectSlotfv,         LPALAUXILIARYEFFECTSLOTFV,      "alAuxiliaryEffectSlotfv"       )
    LOADOPENALPROC(alGetAuxiliaryEffectSloti,       LPALGETAUXILIARYEFFECTSLOTI,    "alGetAuxiliaryEffectSloti"     )
    LOADOPENALPROC(alGetAuxiliaryEffectSlotiv,      LPALGETAUXILIARYEFFECTSLOTIV,   "alGetAuxiliaryEffectSlotiv"    )
    LOADOPENALPROC(alGetAuxiliaryEffectSlotf,       LPALGETAUXILIARYEFFECTSLOTF,    "alGetAuxiliaryEffectSlotf"     )
    LOADOPENALPROC(alGetAuxiliaryEffectSlotfv,      LPALGETAUXILIARYEFFECTSLOTFV,   "alGetAuxiliaryEffectSlotfv"    )
    
    return true;
    
    #undef LOADOPENALPROC
}

ALBufferObject* OpenALSoundDevice::createSoundBuffer(const io::stringc &Filename)
{
    /* Search for an existing instance of the wanted buffer object */
    if (!OpenALSoundDevice::AudioBufferMap_.empty())
    {
        AudioBufferType::iterator it = OpenALSoundDevice::AudioBufferMap_.find(Filename.str());
        
        if (it != OpenALSoundDevice::AudioBufferMap_.end())
        {
            ALBufferObject* BufferObj = it->second;
            BufferObj->grab();
            return BufferObj;
        }
    }
    
    /* Load sound PCM buffer */
    SAudioBufferPtr AudioBuffer = loadAudioPCMBuffer(Filename);
    
    if (!AudioBuffer)
    {
        io::Log::error("Loading sound failed");
        return 0;
    }
    
    /* Get channel format */
    ALenum ALChannelFormat = 0;
    
    if (AudioBuffer->FormatFlags.BitsPerSample == 16)
    {
        if (AudioBuffer->FormatFlags.Channels == 1)
            ALChannelFormat = AL_FORMAT_MONO16;
        else
            ALChannelFormat = AL_FORMAT_STEREO16;
    }
    else
    {
        if (AudioBuffer->FormatFlags.Channels == 1)
            ALChannelFormat = AL_FORMAT_MONO8;
        else
            ALChannelFormat = AL_FORMAT_STEREO8;
    }
    
    /* Create a new instance */
    ALBufferObject* BufferObj = MemoryManager::createMemory<ALBufferObject>("OpenALSoundDevice::ALBufferObject");
    
    OpenALSoundDevice::AudioBufferMap_[Filename.str()] = BufferObj;
    
    /* Upload PCM buffer to the OpenAL buffer object */
    BufferObj->updateBuffer(
        Filename,
        ALChannelFormat,
        AudioBuffer->BufferPCM,
        AudioBuffer->BufferSize,
        AudioBuffer->FormatFlags.SamplesPerSec
    );
    
    BufferObj->grab();
    
    return BufferObj;
}

void OpenALSoundDevice::dropSoundBuffer(ALBufferObject* &BufferObj)
{
    /* Decrease reference counter */
    if (BufferObj && BufferObj->drop())
    {
        /* Delete sound buffer */
        if (!OpenALSoundDevice::AudioBufferMap_.empty())
        {
            AudioBufferType::iterator it = OpenALSoundDevice::AudioBufferMap_.find(BufferObj->getFilename().str());
            
            if (it != OpenALSoundDevice::AudioBufferMap_.end())
                OpenALSoundDevice::AudioBufferMap_.erase(it);
        }
        
        MemoryManager::deleteMemory(BufferObj);
    }
}


} // /namespace audio

} // /namespace sp


#endif



// ================================================================================
