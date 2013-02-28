//
// SoftPixel Engine - Audio Tests
//

#include <SoftPixelEngine.hpp>

using namespace sp;

#include "../common.hpp"

SP_TESTS_DECLARE

int main()
{
    SP_TESTS_INIT_NOAA("Audio")
    
    // Create sound device and load sample
    audio::SoundDevice* spListener = spDevice->createSoundDevice(audio::SOUNDDEVICE_OPENAL);
    
    // Get audio buffer
    const io::stringc MediaPath = "../Media/";
    audio::SAudioBufferPtr AudioBuffer = spListener->loadAudioPCMBuffer(
        MediaPath +
        "Test Sound (Cut Paperboard).wav"
        //"Test Sound (Plastic Container Bounce).wav"
        //"Test Sound (Biting into an Apple).wav"
    );
    
    audio::SAudioBuffer* b = AudioBuffer.get();
    
    // Setup render stuff
    spRenderer->setClearColor(255);
    
    const dim::size2di Res(spContext->getResolution());
    const s32 Vert = Res.Height/2;
    const s32 Size = 64;
    
    f32 From = 0.0f, To = 1.0f;
    
    SP_TESTS_MAIN_BEGIN
    {
        spRenderer->beginDrawing2D();
        
        f32 Range = static_cast<f32>(spControl->getCursorPosition().X) / Res.Width;
        
        if (spControl->mouseDown(io::MOUSE_LEFT))
            From = Range;
        else if (spControl->mouseDown(io::MOUSE_RIGHT))
            To = Range;
        
        tool::AudioPCMPlotter::plotAudioBuffer(
            *AudioBuffer.get(),
            dim::rect2di(0, Vert - Size, Res.Width, Vert + Size),
            video::color(64, 64, 255), From, From + To
        );
        
        spRenderer->endDrawing2D();
    }
    SP_TESTS_MAIN_END
}
