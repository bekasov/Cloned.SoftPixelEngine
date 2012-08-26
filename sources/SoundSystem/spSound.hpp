/*
 * Sound interface header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_AUDIO_SOUND_H__
#define __SP_AUDIO_SOUND_H__


#include "Base/spStandard.hpp"
#include "Base/spDimensionVector3D.hpp"
#include "Base/spInputOutputString.hpp"
#include "SoundSystem/spSoundEffect.hpp"


namespace sp
{
namespace audio
{


/*
 * Macros
 */

static const s32 DEF_SOUND_BUFFERCOUNT = 1;


/**
Sound base class. Sounds can have several buffers (in OpenAL called "sources" which share the same buffer).
By default each sound has only one buffer. Use several buffers when you want to play the same sound several times in a row.
\ingroup group_audio
*/
class SP_EXPORT Sound
{
    
    public:
        
        virtual ~Sound();
        
        /* === Functions === */
        
        virtual bool reload(const io::stringc &Filename, u8 BufferCount = DEF_SOUND_BUFFERCOUNT) = 0;
        virtual void close() = 0;
        
        //! Starts playing the sound. The sound will be played from the beginning.
        virtual void play();
        
        //! Pauses (if "Paused" is true) or resumes (if "Paused" if false) the sound.
        virtual void pause(bool Paused = true);
        
        //! Stops playing the sound.
        virtual void stop();
        
        /**
        Sets the seek or status of the sound. To use this function "play" needs not to be called.
        \param Seek: Specifies the status. Must lie between 0.0 and 1.0.
        */
        virtual void setSeek(f32 Seek);
        virtual f32 getSeek() const;
        
        /**
        Sets the volume of the sound.
        \param Volume: Specifies the volume for the sound. Must lie between 0.0 (quiet) and 1.0 (loud).
        By default 1.0.
        */
        virtual void setVolume(f32 Volume);
        
        /**
        Sets the speed or frequency of the sound.
        \param Speed: Specifies the speed. Must lie between 0.0 (slow, where with 0.0 the sound stops to play
        and 2.0 (fast). By default 1.0 (normal). Backwards playing is not possible!
        */
        virtual void setSpeed(f32 Speed);
        
        /**
        Sets the balance of the sound.
        \param Balance: Specifies the balance between two sound-output devices (the sound boxes).
        Must lie between -1.0 (left) and 1.0 (right). By default 0.0 (in the middle, both boxes have the
        same volume).
        */
        virtual void setBalance(f32 Balance);
        
        //! Enables or disables loop playback mode. If true the sound will played for ever. By default false.
        virtual void setLoop(bool Enable);
        
        //! Returns the length of the sound (in seconds).
        virtual f32 getLength() const = 0;
        
        //! Returns true if the object is a valid sound. Otherwise no sound file has been loaded or could not load.
        virtual bool valid() const = 0;
        
        //! Sets the 3D global position of that sound. Only for volumetric sounds.
        virtual void setPosition(const dim::vector3df &Position);
        //! Sets the 3D global velocity of that sound. Only for volumetric sound and sound systems which support the 'doppler' effect.
        virtual void setVelocity(const dim::vector3df &Veclotiy);
        
        virtual void setVolumetric(bool isVolumetric);
        virtual void setVolumetricRadius(f32 Radius);
        
        //! Returns true if this sound is currenlty playing.
        virtual bool playing() const;
        //! Returns true if the sound has reached the end.
        virtual bool finish() const;
        
        //! Sets the new sound effect.
        virtual void setSoundEffect(SoundEffect* SoundEffectObject);
        
        /**
        Saves the record sound as a wave (WAV) file to the disk. A sound needs to be recorded using
        the "beginRecording" and "endRecording" before this function can be used.
        \param Filename: Filename or path where the sound file is to be saved.
        \return True if the sound could be saved.
        */
        virtual bool saveRecord(const io::stringc &Filename);
        
        //! Begins to record a sound from an sound-input device (microphone).
        virtual void beginRecording();
        
        //! Finish the recording sound. After recording a sound it can be saved as a wave (WAV) file.
        virtual void endRecording();
        
        /* === Inline functions === */
        
        //! Returns the count of buffers. By default 1.
        inline u8 getBufferCount() const
        {
            return BufferCount_;
        }
        //! Returns the sound's filename.
        inline io::stringc getFilename() const
        {
            return Filename_;
        }
        
        /**
        Returns true if the sound is currently paused.
        \note Can only be true if the sound was not stoped until it was played.
        */
        inline bool paused() const
        {
            return isPaused_;
        }
        //! Returns true if the sound is currently being recorded.
        inline bool recording() const
        {
            return isRecording_;
        }
        
        //! Returns true if the sound was played in loop mode.
        inline bool getLoop() const
        {
            return isLoop_;
        }
        
        //! Returns true if the sound is volumetric. By default false.
        inline bool getVolumetric() const
        {
            return isVolumetric_;
        }
        //! Returns the volumetric radius. By default 100.0.
        inline f32 getVolumetricRadius() const
        {
            return Radius_;
        }
        
        //! Returns the global position.
        inline dim::vector3df getPosition() const
        {
            return Position_;
        }
        //! Returns the global velocity.
        inline dim::vector3df getVeclotiy() const
        {
            return Veclotiy_;
        }
        
        //! Returns the speed. By default 1.0.
        inline f32 getSpeed() const
        {
            return Speed_;
        }
        //! Returns the volume. By default 1.0.
        inline f32 getVolume() const
        {
            return Volume_;
        }
        /**
        Returns the balance. 0.0 means the sound is centered, -1.0 means the sound is on the
        left and +1.0 means the sound is on the right. By default 0.0.
        */
        inline f32 getBalance() const
        {
            return Balance_;
        }
        
        //! Returns a pointer to the current SoundEffect object.
        inline SoundEffect* getSoundEffect() const
        {
            return Effect_;
        }
        
    protected:
        
        friend class SoundDevice;
        
        Sound();
        
        /* Functions */
        
        bool checkFile(const io::stringc &Filename) const;
        
        /* Members */
        
        io::stringc Filename_;
        u8 BufferCount_;
        
        f32 Radius_, Volume_, Balance_, Speed_, Seek_;
        bool isPaused_, isLoop_, isPlaying_, isVolumetric_, isRecording_;
        
        dim::vector3df Position_, Veclotiy_;
        bool hasSeekChanged_;
        
        SoundEffect* Effect_;
        
};


} // /namespace audio

} // /namespace sp


#endif



// ================================================================================
