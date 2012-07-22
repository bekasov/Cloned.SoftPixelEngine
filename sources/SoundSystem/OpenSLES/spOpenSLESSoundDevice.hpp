/*
 * OpenSL|ES sound device header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_AUDIO_SOUNDDEVICE_OPENSLES_H__
#define __SP_AUDIO_SOUNDDEVICE_OPENSLES_H__


#include "Base/spStandard.hpp"

#if defined(SP_COMPILE_WITH_OPENSLES)


#include "SoundSystem/spSoundDevice.hpp"
#include "SoundSystem/OpenSLES/spOpenSLESSound.hpp"


namespace sp
{
namespace audio
{


class SP_EXPORT OpenSLESSoundDevice : public SoundDevice
{
        
    public:
            
        OpenSLESSoundDevice();
        ~OpenSLESSoundDevice();
        
        /* Functions */
        
        io::stringc getInterface() const;
        
        Sound* createSound();
        
    private:
        
        friend class OpenSLESSound;
        
        /* Functions */
        
        bool createSoundEngine();
        bool createOutputMixer();
        
        static bool checkForError(
            const SLresult Result, const io::stringc &ErrorMessage = "OpenSL|ES audio error"
        );
        
        static bool objectRealize(
            SLObjectItf &Object, const io::stringc &ObjectName = "OpenSL|ES object"
        );
        static bool objectGetInterface(
            SLObjectItf &Object, const SLInterfaceID ID, void* Interface, const io::stringc &ObjectName = "OpenSL|ES object"
        );
        
        static void releaseObject(SLObjectItf &Object);
        
        /* Members */
        
        SLEngineItf Engine_;
        SLObjectItf EngineObject_;
        
        SLObjectItf OutputMixer_;
        SLEnvironmentalReverbItf EnvReverbInterface_;
        
};


} // /namespace audio

} // /namespace sp


#endif

#endif



// ================================================================================
