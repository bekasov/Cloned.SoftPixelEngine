/*
 * OpenSL|ES sound device file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "SoundSystem/OpenSLES/spOpenSLESSoundDevice.hpp"

#if defined(SP_COMPILE_WITH_OPENSLES)


namespace sp
{
namespace audio
{


OpenSLESSoundDevice::OpenSLESSoundDevice() :
    SoundDevice     (SOUNDDEVICE_OPENSLES   ),
    EngineObject_   (0                      ),
    OutputMixer_    (0                      )
{
    if (createSoundEngine())
        createOutputMixer();
}
OpenSLESSoundDevice::~OpenSLESSoundDevice()
{
    /* Delete sound list before releasing sound device */
    MemoryManager::deleteList(SoundList_);
    
    /* Release sound device */
    OpenSLESSoundDevice::releaseObject(OutputMixer_);
    OpenSLESSoundDevice::releaseObject(EngineObject_);
}

io::stringc OpenSLESSoundDevice::getInterface() const
{
    return "OpenSL|ES 1.0.1";
}

Sound* OpenSLESSoundDevice::createSound()
{
    OpenSLESSound* NewSound = new OpenSLESSound();
    SoundList_.push_back(NewSound);
    return NewSound;
}


/*
 * ======= Private: =======
 */

bool OpenSLESSoundDevice::createSoundEngine()
{
    /* Create OpenSL|ES sound engine */
    if (!OpenSLESSoundDevice::checkForError(slCreateEngine(&EngineObject_, 0, 0, 0, 0, 0), "Creating OpenSL|ES sound engine failed"))
        return false;
    
    /* Realize the object */
    if (!OpenSLESSoundDevice::objectRealize(EngineObject_, "OpenSL|ES sound engine"))
        return false;
    
    /* Get the engine interface to create other objects */
    if (!OpenSLESSoundDevice::objectGetInterface(EngineObject_, SL_IID_ENGINE, &Engine_, "OpenSL|ES sound engine"))
        return false;
    
    return true;
}

bool OpenSLESSoundDevice::createOutputMixer()
{
    /* Create output mixer */
    const SLInterfaceID IDList[1] = { SL_IID_ENVIRONMENTALREVERB };
    const SLboolean RequiredArgs[1] = { SL_BOOLEAN_FALSE };
    
    if (!OpenSLESSoundDevice::checkForError((*Engine_)->CreateOutputMix(Engine_, &OutputMixer_, 1, IDList, RequiredArgs), "Creating output mixer failed"))
        return false;
    
    /* Realize the object */
    if (!OpenSLESSoundDevice::objectRealize(OutputMixer_, "output mixer"))
        return false;
    
    /* Get the environmental reverb interface */
    if (!OpenSLESSoundDevice::objectGetInterface(OutputMixer_, SL_IID_ENVIRONMENTALREVERB, &EnvReverbInterface_, "output mixer"))
        return false;
    
    /* Configure the reverb interface */
    SLEnvironmentalReverbSettings ReverbSettings = SL_I3DL2_ENVIRONMENT_PRESET_STONECORRIDOR;
    
    if (!OpenSLESSoundDevice::checkForError((*EnvReverbInterface_)->SetEnvironmentalReverbProperties(EnvReverbInterface_, &ReverbSettings)))
        return false;
    
    return true;
}

bool OpenSLESSoundDevice::checkForError(const SLresult Result, const io::stringc &ErrorMessage)
{
    if (Result != SL_RESULT_SUCCESS)
    {
        io::Log::error(ErrorMessage);
        io::Log::upperTab();
        
        switch (Result)
        {
            case SL_RESULT_PARAMETER_INVALID:
                io::Log::error("Invalid parameter"); break;
            case SL_RESULT_FEATURE_UNSUPPORTED:
                io::Log::error("Feature unsupported"); break;
            case SL_RESULT_PRECONDITIONS_VIOLATED:
                io::Log::error("Preconditions violated"); break;
            case SL_RESULT_RESOURCE_ERROR:
                io::Log::error("Resource error"); break;
            case SL_RESULT_MEMORY_FAILURE:
                io::Log::error("Memory failure"); break;
            case SL_RESULT_IO_ERROR:
                io::Log::error("Input/output error"); break;
            case SL_RESULT_CONTENT_CORRUPTED:
                io::Log::error("Content corrupted"); break;
            case SL_RESULT_CONTENT_NOT_FOUND:
                io::Log::error("Content not found"); break;
            case SL_RESULT_PERMISSION_DENIED:
                io::Log::error("Permission denied"); break;
            case SL_RESULT_CONTROL_LOST:
                io::Log::error("Control lost"); break;
        }
        
        io::Log::lowerTab();
        
        return false;
    }
    
    return true;
}

bool OpenSLESSoundDevice::objectRealize(SLObjectItf &Object, const io::stringc &ObjectName)
{
    return OpenSLESSoundDevice::checkForError(
        (*Object)->Realize(Object, SL_BOOLEAN_FALSE), "Realizing " + ObjectName + " failed"
    );
}
bool OpenSLESSoundDevice::objectGetInterface(
    SLObjectItf &Object, const SLInterfaceID ID, void* Interface, const io::stringc &ObjectName)
{
    return OpenSLESSoundDevice::checkForError(
        (*Object)->GetInterface(Object, ID, Interface), "Getting interface from " + ObjectName + " failed"
    );
}

void OpenSLESSoundDevice::releaseObject(SLObjectItf &Object)
{
    if (Object)
    {
        (*Object)->Destroy(Object);
        Object = 0;
    }
}


} // /namespace audio

} // /namespace sp


#endif



// ================================================================================
