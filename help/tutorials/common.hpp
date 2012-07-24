//
// SoftPixel Engine common file
//

#include <SoftPixelEngine.hpp>
#include <iostream>


enum EChooseRendererFlags
{
    CHOOSE_RENDERER             = 0x00000001,
    CHOOSE_SCREENSIZE           = 0x00000002,
    CHOOSE_COLORDEPTH           = 0x00000004,
    CHOOSE_FULLSCREEN           = 0x00000008,
    CHOOSE_VSYNC                = 0x00000010,
    CHOOSE_ANTIALIAS            = 0x00000020,
    
    CHOOSE_DISABLE_OPENGL       = 0x00010000,
    CHOOSE_DISABLE_DIRECT3D9    = 0x00020000,
    CHOOSE_DISABLE_DIRECT3D11   = 0x00040000,
    CHOOSE_DISABLE_DUMMY        = 0x00100000,
};

struct SChooseRendererData
{
    SChooseRendererData()
    {
        Driver      = sp::video::RENDERER_OPENGL;
        ScreenSize  = sp::dim::size2di(640, 480);
        ColorDepth  = 32;
        FullScreen  = false;
        Vsync       = true;
        AntiAlias   = false;
        MultiSample = 2;
    }
    ~SChooseRendererData()
    {
    }
    
    /* Members */
    sp::video::ERenderSystems Driver;
    sp::dim::size2di ScreenSize;
    sp::s32 ColorDepth;
    bool FullScreen;
    bool Vsync;
    bool AntiAlias;
    sp::s32 MultiSample;
};


/*
 * Detailed functions
 */

bool CheckOptionState(bool Enabled, sp::c8 TypeName, sp::c8 &NumberChar)
{
    bool Result = false;
    
    if (Enabled)
    {
        if (TypeName == NumberChar)
            Result = true;
        ++NumberChar;
    }
    
    return Result;
}

SChooseRendererData ChooseRenderer(sp::s32 Flags)
{
    SChooseRendererData RendererData;
    
    sp::c8 TypeName;
    
    if (Flags & CHOOSE_RENDERER)
    {
        TypeName = 0;
        sp::c8 NumberChar = 'a';
        
        sp::io::Log::message("====================");
        sp::io::Log::message("Choose the renderer:");
        sp::io::Log::message("====================");
        
		#ifdef SP_COMPILE_WITH_OPENGL
        if (!(Flags & CHOOSE_DISABLE_OPENGL))
            sp::io::Log::message(sp::io::stringc(NumberChar++) + ".) OpenGL");
		#endif
		#ifdef SP_COMPILE_WITH_DIRECT3D9
        if (!(Flags & CHOOSE_DISABLE_DIRECT3D9))
            sp::io::Log::message(sp::io::stringc(NumberChar++) + ".) Direct3D9");
		#endif
		#ifdef SP_COMPILE_WITH_DIRECT3D11
        if (!(Flags & CHOOSE_DISABLE_DIRECT3D11))
            sp::io::Log::message(sp::io::stringc(NumberChar++) + ".) Direct3D11");
		#endif
        if (!(Flags & CHOOSE_DISABLE_DUMMY))
            sp::io::Log::message(sp::io::stringc(NumberChar++) + ".) Dummy");
        
        sp::io::Log::message("====================");
        
        if (NumberChar == 'a')
        {
            sp::io::Log::warning("No renderer enabled. Using dummy");
            RendererData.Driver = sp::video::RENDERER_DUMMY;
        }
        else
        {
            while (TypeName < 'a' || TypeName >= NumberChar)
                std::cin >> TypeName;
            
            NumberChar = 'a';
            
            #ifdef SP_COMPILE_WITH_OPENGL
            if (CheckOptionState(!(Flags & CHOOSE_DISABLE_OPENGL), TypeName, NumberChar))
                RendererData.Driver = sp::video::RENDERER_OPENGL;
            #endif
            #ifdef SP_COMPILE_WITH_DIRECT3D9
            if (CheckOptionState(!(Flags & CHOOSE_DISABLE_DIRECT3D9), TypeName, NumberChar))
                RendererData.Driver = sp::video::RENDERER_DIRECT3D9;
            #endif
            #ifdef SP_COMPILE_WITH_DIRECT3D11
            if (CheckOptionState(!(Flags & CHOOSE_DISABLE_DIRECT3D11), TypeName, NumberChar))
                RendererData.Driver = sp::video::RENDERER_DIRECT3D11;
            #endif
            if (CheckOptionState(!(Flags & CHOOSE_DISABLE_DUMMY), TypeName, NumberChar))
                RendererData.Driver = sp::video::RENDERER_DUMMY;
        }
    }
    
    if (Flags & CHOOSE_SCREENSIZE)
    {
        TypeName = 0;
        
        sp::io::Log::message("=======================");
        sp::io::Log::message("Choose the screen size:");
        sp::io::Log::message("=======================");
        sp::io::Log::message("a.) 320x240");
        sp::io::Log::message("b.) 352x288");
        sp::io::Log::message("c.) 576x480");
        sp::io::Log::message("d.) 640x480");
        sp::io::Log::message("e.) 800x600");
        sp::io::Log::message("f.) 1024x768");
        sp::io::Log::message("g.) 1280x768");
        sp::io::Log::message("h.) 1280x1024");
        sp::io::Log::message("=======================");
        
        while (TypeName < 'a' || TypeName > 'h')
            std::cin >> TypeName;
        
        switch (TypeName)
        {
            case 'a':
                RendererData.ScreenSize = sp::dim::size2di(320, 240); break;
            case 'b':
                RendererData.ScreenSize = sp::dim::size2di(352, 288); break;
            case 'c':
                RendererData.ScreenSize = sp::dim::size2di(576, 480); break;
            case 'd':
                RendererData.ScreenSize = sp::dim::size2di(640, 480); break;
            case 'e':
                RendererData.ScreenSize = sp::dim::size2di(800, 600); break;
            case 'f':
                RendererData.ScreenSize = sp::dim::size2di(1024, 768); break;
            case 'g':
                RendererData.ScreenSize = sp::dim::size2di(1280, 768); break;
            case 'h':
                RendererData.ScreenSize = sp::dim::size2di(1280, 1024); break;
        }
    }
    
    if (Flags & CHOOSE_COLORDEPTH)
    {
        TypeName = 0;
        
        sp::io::Log::message("=======================");
        sp::io::Log::message("Choose the color depth:");
        sp::io::Log::message("=======================");
        sp::io::Log::message("a.) 16 bits");
        sp::io::Log::message("b.) 24 bits");
        sp::io::Log::message("c.) 32 bits");
        sp::io::Log::message("=======================");
        
        while (TypeName < 'a' || TypeName > 'c')
            std::cin >> TypeName;
        
        switch (TypeName)
        {
            case 'a':
                RendererData.ColorDepth = 16; break;
            case 'b':
                RendererData.ColorDepth = 24; break;
            case 'c':
                RendererData.ColorDepth = 32; break;
        }
    }
    
    if (Flags & CHOOSE_FULLSCREEN)
    {
        TypeName = 0;
        
        sp::io::Log::message("=======================");
        sp::io::Log::message("Choose the screen mode:");
        sp::io::Log::message("=======================");
        sp::io::Log::message("a.) windowed screen");
        sp::io::Log::message("b.) full screen");
        sp::io::Log::message("=======================");
        
        while (TypeName < 'a' || TypeName > 'b')
            std::cin >> TypeName;
        
        switch (TypeName)
        {
            case 'a':
                RendererData.FullScreen = false; break;
            case 'b':
                RendererData.FullScreen = true; break;
        }
    }
    
    if (Flags & CHOOSE_VSYNC)
    {
        TypeName = 0;
        
        sp::io::Log::message("======================");
        sp::io::Log::message("Choose the vsync mode:");
        sp::io::Log::message("======================");
        sp::io::Log::message("a.) vsync on");
        sp::io::Log::message("b.) vsync off");
        sp::io::Log::message("======================");
        
        while (TypeName < 'a' || TypeName > 'b')
            std::cin >> TypeName;
        
        switch (TypeName)
        {
            case 'a':
                RendererData.Vsync = true; break;
            case 'b':
                RendererData.Vsync = false; break;
        }
    }
    
    if (Flags & CHOOSE_ANTIALIAS)
    {
        TypeName = 0;
        
        sp::io::Log::message("===========================");
        sp::io::Log::message("Choose the anti-alias mode:");
        sp::io::Log::message("===========================");
        sp::io::Log::message("a.) no anti-alising");
        sp::io::Log::message("b.) 2x multi-sampler");
        sp::io::Log::message("c.) 4x multi-sampler");
        sp::io::Log::message("d.) 8x multi-sampler");
        sp::io::Log::message("e.) 16x multi-sampler");
        sp::io::Log::message("===========================");
        
        while (TypeName < 'a' || TypeName > 'e')
            std::cin >> TypeName;
        
        switch (TypeName)
        {
            case 'a':
                RendererData.AntiAlias      = false; break;
            case 'b':
                RendererData.AntiAlias      = true;
                RendererData.MultiSample    = 2;
                break;
            case 'c':
                RendererData.AntiAlias      = true;
                RendererData.MultiSample    = 4;
                break;
            case 'd':
                RendererData.AntiAlias      = true;
                RendererData.MultiSample    = 8;
                break;
            case 'e':
                RendererData.AntiAlias      = true;
                RendererData.MultiSample    = 16;
                break;
        }
    }
    
    sp::io::Log::clearConsole();
    
    return RendererData;
}


/*
 * Simple functions
 */

sp::video::ERenderSystems ChooseRenderer()
{
    return ChooseRenderer(CHOOSE_RENDERER).Driver;
}

sp::audio::ESoundDevices ChooseSoundDevice()
{
    sp::audio::ESoundDevices SoundDevice = sp::audio::SOUNDDEVICE_AUTODETECT;
    
    sp::c8 TypeName = 0;
    
    sp::io::Log::message("========================");
    sp::io::Log::message("Choose the sound device:");
    sp::io::Log::message("========================");
    sp::io::Log::message("a.) WinMM");
    sp::io::Log::message("b.) Dummy");
    sp::io::Log::message("========================");
    
    while (TypeName < 'a' || TypeName > 'b')
        std::cin >> TypeName;
    
    switch (TypeName)
    {
        case 'a':
            SoundDevice = sp::audio::SOUNDDEVICE_WINMM; break;
        case 'b':
            SoundDevice = sp::audio::SOUNDDEVICE_DUMMY; break;
    }
    
    sp::io::Log::clearConsole();
    
    return SoundDevice;
}


// ============================
