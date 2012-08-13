/*
 * Sound device file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "SoundSystem/spSoundDevice.hpp"
#include "Base/spMemoryManagement.hpp"
#include "Base/spMath.hpp"

#if defined(SP_PLATFORM_WINDOWS)
#   include <windows.h>
#endif

#include <boost/foreach.hpp>


namespace sp
{
namespace audio
{


f32 SoundDevice::MelodySpeed_ = 1.0f;

SoundDevice::SoundDevice(const ESoundDevices DeviceType) :
    DeviceType_(DeviceType)
{
    setListenerRange(0.0f, 25.0, 1.0f, 0.0f);
}
SoundDevice::~SoundDevice()
{
    MemoryManager::deleteList(SoundList_);
    MemoryManager::deleteList(SoundEffectList_);
}

io::stringc SoundDevice::getVersion() const
{
    return "SoftSoundSystem - v.4.0";
}

void SoundDevice::printConsoleHeader()
{
    io::Log::message(getVersion(), 0);
    io::Log::message("Copyright (c) 2008 - Lukas Hermanns", 0);
    io::Log::message("Using device: " + getInterface(), 0);
    io::Log::message("", 0);
}

Sound* SoundDevice::loadSound(const io::stringc &Filename, u8 BufferCount)
{
    Sound* NewSound = createSound();
    NewSound->reload(Filename, BufferCount);
    return NewSound;
}

void SoundDevice::deleteSound(Sound* &SoundObject)
{
    MemoryManager::removeElement(SoundList_, SoundObject, true);
}

SoundEffect* SoundDevice::createSoundEffect()
{
    SoundEffect* NewSoundEffect = new SoundEffect();
    SoundEffectList_.push_back(NewSoundEffect);
    return NewSoundEffect;
}
void SoundDevice::deleteSoundDevice(SoundEffect* &SoundEffectObject)
{
    MemoryManager::removeElement(SoundEffectList_, SoundEffectObject, true);
}

void SoundDevice::setListenerPosition(const dim::vector3df &Position)
{
    ListenerPosition_ = Position;
}
void SoundDevice::setListenerVelocity(const dim::vector3df &Velocity)
{
    ListenerVelocity_ = Velocity;
}
void SoundDevice::setListenerOrientation(const dim::matrix4f &Orientation)
{
    ListenerOrientation_ = Orientation;
    ListenerOrientation_.setInverse();
}

void SoundDevice::setListenerRange(const f32 NearDist, const f32 FarDist, const f32 NearVol, const f32 FarVol)
{
    NearDist_   = NearDist;
    FarDist_    = FarDist;
    MiddleDist_ = FarDist_ - NearDist_;
    
    NearVol_    = NearVol;
    FarVol_     = FarVol;
    MiddleVol_  = NearVol_ - FarVol_;
}
void SoundDevice::getListenerRange(f32 &NearDist, f32 &FarDist, f32 &NearVol, f32 &FarVol) const
{
    NearDist    = NearDist_;
    FarDist     = FarDist_;
    
    NearVol     = NearVol_;
    FarVol      = FarVol_;
}


/*
 * ======= Private: =======
 */

void SoundDevice::updateSounds()
{
    foreach (audio::Sound* Sound, SoundList_)
    {
        if (Sound->getLoop() && Sound->finish())
            Sound->play();
        if (Sound->getVolumetric())
            updateSoundVolumetic(Sound);
    }
}

void SoundDevice::updateSoundVolumetic(Sound* CurSound)
{
    const dim::vector3df SoundPos(CurSound->getPosition());
    const f32 Distance = math::getDistance(ListenerPosition_, SoundPos);
    
    f32 Volume = 0.0f;
    
    #if 1
    NearDist_ = 0.0f;
    FarDist_ = MiddleDist_ = CurSound->getVolumetricRadius();
    #endif
    
    if (Distance <= NearDist_)
        Volume = NearVol_;
    else if (Distance <= FarDist_)
        Volume = FarVol_ + ( 1.0f - math::Abs<f32>(Distance - NearDist_) / MiddleDist_ ) * MiddleVol_;
    else
        Volume = FarVol_;
    
    dim::vector3df Dir(SoundPos - ListenerPosition_);
    Dir.normalize();
    Dir = ListenerOrientation_ * Dir;
    
    const f32 OrigVolume = CurSound->Volume_;
    CurSound->Volume_ = Volume * OrigVolume;
    CurSound->setBalance(Dir.X);
    CurSound->Volume_ = OrigVolume;
}


/*
 * Static functions
 */

void SoundDevice::setMelodySpeed(f32 Speed)
{
    MelodySpeed_ = 1.0f / Speed;
}

void SoundDevice::playMelody(const ENoteNames Note, u8 Octave, u32 Duration, u32 Delay)
{
    #if defined(SP_PLATFORM_WINDOWS) || defined(SP_PLATFORM_LINUX)
    
    static const f32 FreqFactor = 1.0595f;
    static const s32 FreqBase   = 65;
    
    const s32 Frequence = static_cast<s32>( pow(FreqFactor, Note + Octave * (NOTE_H + 1)) * FreqBase );
    
    Duration    = static_cast<s32>(MelodySpeed_ * Duration);
    Delay       = static_cast<s32>(MelodySpeed_ * Delay);
    
    #   if defined(SP_PLATFORM_WINDOWS)
    
    Beep(Frequence, Duration);
    
    if (Delay)
        Sleep(Delay);
    
    #   elif defined(SP_PLATFORM_LINUX)
    
    system(("beep -f " + io::stringc(Frequence) + " -l " + io::stringc(Duration)).c_str());
    
    if (Delay)
        usleep(Delay * 1000);
    
    #   endif
    
    #endif
}

void SoundDevice::playMelody(io::stringc MelodyCmdStr)
{
    /* Temporary variables */
    MelodyCmdStr = MelodyCmdStr.upper();
    io::stringc str;
    
    s32 Note = -1;
    u8 Octave = 3;
    u32 Duration = 250, Delay = 0;
    s32 pos1 = 0, pos2 = 0;
    
    /* Loop through the string for each ';' */
    while ( ( pos1 = MelodyCmdStr.find(";", pos2) ) != -1 )
    {
        str = MelodyCmdStr.section(pos2, pos1);
        
        switch (str[0])
        {
            case 'O': // Octave
                Octave = str.right(str.size() - 1).val<u8>(); break;
            case 'T': // Duration time
                Duration = str.right(str.size() - 1).val<s32>(); break;
            case 'S': // Sleep time
                Delay = str.right(str.size() - 1).val<s32>(); break;
                
            case 'C':
            {
                if (str == "C#")        Note = NOTE_C_SHARP;
                else if (str == "CB")   Note = NOTE_C_FLAT;
                else                    Note = NOTE_C;
            }
            break;
            
            case 'D':
            {
                if (str == "D#")        Note = NOTE_D_SHARP;
                else if (str == "DB")   Note = NOTE_D_FLAT;
                else                    Note = NOTE_D;
            }
            break;
            
            case 'E':
            {
                if (str == "E#")        Note = NOTE_E_SHARP;
                else if (str == "EB")   Note = NOTE_E_FLAT;
                else                    Note = NOTE_E;
            }
            break;
            
            case 'F':
            {
                if (str == "F#")        Note = NOTE_F_SHARP;
                else if (str == "FB")   Note = NOTE_F_FLAT;
                else                    Note = NOTE_F;
            }
            break;
            
            case 'G':
            {
                if (str == "G#")        Note = NOTE_G_SHARP;
                else if (str == "GB")   Note = NOTE_G_FLAT;
                else                    Note = NOTE_G;
            }
            break;
            
            case 'A':
            {
                if (str == "A#")        Note = NOTE_A_SHARP;
                else if (str == "AB")   Note = NOTE_A_FLAT;
                else                    Note = NOTE_A;
            }
            break;
            
            case 'H':
            {
                if (str == "H#")        Note = NOTE_H_SHARP;
                else if (str == "HB")   Note = NOTE_H_FLAT;
                else                    Note = NOTE_H;
            }
            break;
        }
        
        if (Note != -1)
        {
            playMelody(static_cast<ENoteNames>(Note), Octave, Duration, Delay);
            Note = -1;
        }
        
        pos2 = pos1 + 1;
    }
}


} // /namespace audio

} // /namespace sp



// ================================================================================
