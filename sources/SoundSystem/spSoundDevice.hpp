/*
 * Sound device header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_AUDIO_SOUNDDEVICE_H__
#define __SP_AUDIO_SOUNDDEVICE_H__


#include "Base/spStandard.hpp"
#include "Base/spInputOutput.hpp"
#include "SoundSystem/spSound.hpp"
#include "SoundSystem/spSoundEffect.hpp"

#include <list>
#include <stdio.h>


namespace sp
{

class SoftPixelDevice;

#if defined(SP_PLATFORM_LINUX)
class SoftPixelDeviceLinux;
#endif

namespace audio
{


/*
 * Enumerations
 */

//! Supported audio devices
enum ESoundDevices
{
    SOUNDDEVICE_AUTODETECT, //!< This is not a sound device but the engine will choose the best suitable device for the operating-system.
    SOUNDDEVICE_OPENAL,     //!< OpenAL sound device.
    SOUNDDEVICE_XAUDIO2,    //!< DirectX Xaudio2 sound system for Windows and XBox.
    SOUNDDEVICE_OPENSLES,   //!< OpenSL|ES for the mobile Android platform.
    SOUNDDEVICE_WINMM,      //!< Windows Multi Media sound system.
    SOUNDDEVICE_DUMMY,      //!< "Dummy" renderer. Just for debugging or for no sound support.
};

//! Note names for beep melodies.
enum ENoteNames
{
    NOTE_C          = 0,
    NOTE_H_SHARP    = 0,
    NOTE_C_SHARP    = 1,
    NOTE_D_FLAT     = 1,
    NOTE_D          = 2,
    NOTE_D_SHARP    = 3,
    NOTE_E_FLAT     = 3,
    NOTE_E          = 4,
    NOTE_F_FLAT     = 4,
    NOTE_F          = 5,
    NOTE_E_SHARP    = 5,
    NOTE_F_SHARP    = 6,
    NOTE_G_FLAT     = 6,
    NOTE_G          = 7,
    NOTE_G_SHARP    = 8,
    NOTE_A_FLAT     = 8,
    NOTE_A          = 9,
    NOTE_A_SHARP    = 10,
    NOTE_H_FLAT     = 10,
    NOTE_H          = 11,
    NOTE_C_FLAT     = 11,
};


class SP_EXPORT SoundDevice
{
    
    public:
        
        virtual ~SoundDevice();
        
        /* === Functions === */
        
        //! Returns the version's name of the sound system.
        virtual io::stringc getVersion() const;
        
        //! Returns the interface name of the sound system (e.g. "WinMM", "OpenSL|ES" or "OpenAL 1.1").
        virtual io::stringc getInterface() const = 0;
        
        //! Prints the console header for this sound device.
        void printConsoleHeader();
        
        //! Updates all sounds
        virtual void updateSounds();
        
        /**
        Creates a sound where no sound-file will be loaded.
        \return Pointer to the new Sound object.
        */
        virtual Sound* createSound() = 0;
        
        /**
        Loads a sound file from the disk.
        \param Filename: Filename or path of the sound file which is to be loaded.
        \param BufferCount: Count of internal sound handles for multiple playback.
        If 1 you will always hear only one sound when it is played. Use more then one if the sound
        is to be played several times successively.
        */
        virtual Sound* loadSound(const io::stringc &Filename, u8 BufferCount = DEF_SOUND_BUFFERCOUNT);
        
        //! Releases and deletes the Sound object (not the sound file).
        virtual void deleteSound(Sound* &SoundObject);
        
        /**
        Creates a new sound effect. You can exchange the effects for each sound dynamically.
        \returns Pointer to the new SoundEffect object.
        */
        virtual SoundEffect* createSoundEffect();
        //! Deletes the given sound effect object.
        virtual void deleteSoundDevice(SoundEffect* &SoundEffectObject);
        
        /* === Listener control === */
        
        /**
        Sets the 3D listener position.
        \param Position: 3D position of the listener. Normally the camera position.
        */
        virtual void setListenerPosition(const dim::vector3df &Position);
        inline dim::vector3df getListenerPosition() const
        {
            return ListenerPosition_;
        }
        
        /**
        Sets the 3D listener velocity. Used for the doppler effect (not supported yet, implemented for futured sound systems).
        \param Velocity: Velocity or speed of the movable 3D listener.
        */
        virtual void setListenerVelocity(const dim::vector3df &Velocity);
        inline dim::vector3df getListenerVelocity() const
        {
            return ListenerVelocity_;
        }
        
        /**
        Sets the 3D listener orientation.
        \param Orientation: Transformation matrix which describes the 3D listener's rotation.
        Use the "getRotationMatrix" function of the active Camera.
        */
        virtual void setListenerOrientation(const dim::matrix4f &Orientation);
        inline dim::matrix4f getListenerOrientation() const
        {
            return ListenerOrientation_;
        }
        
        /**
        Sets the 3D listener hearing range.
        \param NearDist: Distance which must be achieved by the Camera to a 3D sound to hear the sound for
        "NearVol" (the maximal) volume.
        \param FarDist: Distance for the "FarVol" (the minimal) volume.
        \param NearVol: Maximal 3D sound volume.
        \param FarVol: Minimal 3D sound volume.
        */
        virtual void setListenerRange(const f32 NearDist, const f32 FarDist, const f32 NearVol, const f32 FarVol);
        virtual void getListenerRange(f32 &NearDist, f32 &FarDist, f32 &NearVol, f32 &FarVol) const;
        
        /* === Static functions === */
        
        static void setMelodySpeed(f32 Speed);
        
        /**
        Plays a simple melody using the "Beep" function on Windows and the "beep" command (if installed) on Linux.
        \param Note: Kind of note which is to be played.
        \param Octave: Octave number. By default 3 which is the one-line octave.
        \param Duration: Duration in milliseconds. The programs stops for this time!
        (Using "Sleep" function on Windows and "usleep" function on Linux)
        \param Delay: Waiting time after playing (in milliseconds as well).
        */
        static void playMelody(const ENoteNames Note, u8 Octave = 3, u32 Duration = 250, u32 Delay = 0);
        
        /**
        Plays an array of melodies. This function is using the first "playMelody" function.
        \param MelodyCmdStr: String with the note commands. The note's name must be in upper case letter (e.g. 'C', 'D', 'DIS', 'FIS' etc).
        To change the octave you have to write 'O' (for octave) and the number (e.g. "O3" is the default octave).
        Each command needs to be finished with a semicolon. The time is to be set with 'T' and the number (e.g. "T250" for 250 milliseconds).
        The delay after playing a melody is to be set with 'S' (for sleeping time) and the duration (e.g. "S100" for 100 milliseconds).
        Here is an example command string which plays the beginning from the "Super Mario Overworld Theme":
        \code
        audio::setMelodySpeed(2);
        audio::playMelody("O3;T250;S10;E;S250;E;E;S10;C;S250;E;S750;G;O2;S0;G;");
        \endcode
        */
        static void playMelody(io::stringc MelodyCmdStr);
        
    protected:
        
        friend class sp::SoftPixelDevice;
        
        #if defined(SP_PLATFORM_LINUX)
        friend class sp::SoftPixelDeviceLinux;
        #endif
        
        /* === Functions === */
        
        SoundDevice(const ESoundDevices DeviceType);
        
        virtual void updateSoundVolumetic(Sound* CurSound);
        
        /* === Members === */
        
        ESoundDevices DeviceType_;
        
        std::list<Sound*> SoundList_;
        std::list<SoundEffect*> SoundEffectList_;
        
        dim::vector3df ListenerPosition_;
        dim::vector3df ListenerVelocity_;
        dim::matrix4f ListenerOrientation_;
        
        f32 NearVol_, FarVol_, MiddleVol_;
        f32 NearDist_, FarDist_, MiddleDist_;
        
        static f32 MelodySpeed_;
        
};


} // /namespace audio

} // /namespace sp


#endif



// ================================================================================
