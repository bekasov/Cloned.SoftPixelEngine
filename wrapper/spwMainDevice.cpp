/*
 * Wrapper main device file
 * 
 * This file is part of the "SoftPixel Engine Wrapper" (Copyright (c) 2008 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "spwStandard.hpp"


/*
 * ======= All wrapper functions =======
 */

SPW_PROC(void) extHideWindow(int Handle)
{
    #if defined(SP_PLATFORM_WINDOWS)
    ShowWindow(CASTTO(HWND, Handle), SW_HIDE);
    #endif
}

SPW_PROC(STR) spwWrapperVersion_ex()
{
    return "SoftPixel Engine Wrapper - v.2.2 beta";
}

SPW_PROC(void) spwUseOpenGL_ex()
{
    g_DriverType = video::RENDERER_OPENGL;
}
SPW_PROC(void) spwUseDirect3D9_ex()
{
    g_DriverType = video::RENDERER_DIRECT3D9;
}
SPW_PROC(void) spwUseDirect3D11_ex()
{
    g_DriverType = video::RENDERER_DIRECT3D11;
}
SPW_PROC(void) spwUseDummy_ex()
{
    g_DriverType = video::RENDERER_DUMMY;
}
SPW_PROC(void) spwMultiSamples_ex(int MultiSamples)
{
    g_DeviceFlags.isAntiAlias = (MultiSamples != 0);
    g_DeviceFlags.MultiSamples = MultiSamples;
}
SPW_PROC(void) spwVsync_ex(bool Enable)
{
    g_DeviceFlags.isVsync = Enable;
}

SPW_PROC(void) spwGraphics3D(int Width, int Height, int Depth = 32, int Mode = 0)
{
    /* Create SoftPixel device */
    g_Device = createGraphicsDevice(
        g_DriverType, dim::size2di(Width, Height), Depth, g_AppTitle, (Mode == 1), g_DeviceFlags
    );
    
    /* Basic members */
    g_RenderSystem      = g_Device->getRenderSystem();
    g_RenderContext     = g_Device->getRenderContext();
    g_InputControl      = g_Device->getInputControl();
    g_OSInformator      = g_Device->getOSInformator();
    
    g_SceneGraph        = g_Device->createSceneGraph();
    g_SoundDevice       = g_Device->createSoundDevice();
    g_CollisionDetector = g_Device->createCollisionGraph();
    g_PhysicsSystem     = g_Device->createPhysicsSimulator(physics::SIMULATOR_BULLET);
    
    g_StencilManager    = g_SceneGraph->getStencilManager();
    
    g_ScriptLoader      = new tool::ScriptLoader();
    
    /* Basic settings */
    g_Font = g_StdFont = g_RenderSystem->createFont("Blitz");
    g_RenderSystem->clearBuffers();
    
    g_ScreenSize.Width  = Width;
    g_ScreenSize.Height = Height;
    g_ColorDepth        = Depth;
    
    g_Device->setFrameRate(200);
    
    /* Initialization */
    io::Log::close();
    
    g_Device->updateEvent();
    
    InitScancodeList();
}
SPW_PROC(void) spwGraphics(int Width, int Height, int Depth = 32, int Mode = 0)
{
    spwGraphics3D(Width, Height, Depth, Mode);
}
SPW_PROC(void) spwEndGraphics()
{
    /* Delete each buffer object */
    MemoryManager::deleteList(g_MatrixList);
    MemoryManager::deleteList(g_BufferList);
    MemoryManager::deleteMemory(g_ScriptLoader);
    
    /* Delete the whole device */
    deleteDevice();
}
SPW_PROC(void) spwOpenDebugLog_ex(bool Enable, STR File)
{
    if (Enable)
        io::Log::open(File);
    else
        io::Log::close();
}

SPW_PROC(void) spwAppTitle(STR Title, STR EndMessage = "")
{
    g_AppTitle      = Title;
    g_EndMessage    = EndMessage;
    
    if (g_RenderContext)
        g_RenderContext->setWindowTitle(g_AppTitle);
}
SPW_PROC(int) spwGraphicsWidth()
{
    return (g_RenderSystem->getRenderTarget() ? g_RenderSystem->getRenderTarget()->getSize().Width : g_ScreenSize.Width);
}
SPW_PROC(int) spwGraphicsHeight()
{
    return (g_RenderSystem->getRenderTarget() ? g_RenderSystem->getRenderTarget()->getSize().Height : g_ScreenSize.Height);
}
SPW_PROC(int) spwGraphicsDepth()
{
    return g_ColorDepth;
}

SPW_PROC(int) spwRenderSystem_ex()
{
    return (int)g_RenderSystem->getRendererType();
}
SPW_PROC(STR) spwVDVersion_ex()
{
    return g_RenderSystem->getVersion().c_str();
}
SPW_PROC(STR) spwVDVendor_ex()
{
    return g_RenderSystem->getVendor().c_str();
}
SPW_PROC(STR) spwVDRenderer_ex()
{
    return g_RenderSystem->getRenderer().c_str();
}

SPW_PROC(void) spwMessageBox(STR Message, int Type = 0)
{
    switch (Type)
    {
        case 0:
            io::Log::message(Message, io::LOG_MSGBOX); break;
        case 1:
            io::Log::warning(Message, io::LOG_MSGBOX); break;
        case 2:
            io::Log::error(Message, io::LOG_MSGBOX); break;
    }
}
SPW_PROC(void) spwBeep_ex(int Frequency, int Duration)
{
    #if defined(SP_PLATFORM_WINDOWS)
    Beep(Frequency, Duration);
    #endif
}

SPW_PROC(void) spwFrameRate_ex(int FPS)
{
    g_Device->setFrameRate(FPS);
}
SPW_PROC(int) spwFPS_ex()
{
    return g_Timer.getFPS();
}

SPW_PROC(int) spwGetKey()
{
    for (s32 i = 0; i < 255; ++i)
    {
        if (g_InputControl->keyDown((io::EKeyCodes)g_ScancodeList[i]))
            return i;
    }
    return 0;
}
SPW_PROC(void) spwWaitKey()
{
    if (g_Device)
    {
        while (g_Device->updateEvent() && !spwGetKey());
    }
    #if defined(SP_PLATFORM_WINDOWS)
    else
    {
        while (1)
        {
            for (int i = 0x01; i <= 0xFE; ++i)
            {
                if (HIWORD(GetKeyState(i)) != 0)
                    return;
            }
        }
    }
    #endif
}
SPW_PROC(bool) spwKeyDown(int Scancode)
{
    //return g_InputControl->keyDown((io::EKeyCodes)g_ScancodeList[Scancode]);
    return g_InputControl->keyDownEx((io::EKeyCodes)g_ScancodeList[Scancode]);
}
SPW_PROC(bool) spwKeyHit(int Scancode)
{
    return g_InputControl->keyHit((io::EKeyCodes)g_ScancodeList[Scancode]);
}
SPW_PROC(bool) spwKeyReleased_ex(int Scancode)
{
    return g_InputControl->keyReleased((io::EKeyCodes)g_ScancodeList[Scancode]);
}

SPW_PROC(int) spwGetMouse()
{
    if (g_InputControl->mouseDown(io::MOUSE_LEFT))   return 1;
    if (g_InputControl->mouseDown(io::MOUSE_RIGHT))  return 2;
    if (g_InputControl->mouseDown(io::MOUSE_MIDDLE)) return 3;
    return 0;
}
SPW_PROC(bool) spwMouseDown(int Scancode)
{
    return (Scancode >= 1 && Scancode <= 3) ? g_InputControl->mouseDown(g_MouseKeyList[Scancode - 1]) : false;
}
SPW_PROC(bool) spwMouseHit(int Scancode)
{
    return (Scancode >= 1 && Scancode <= 3) ? g_InputControl->mouseHit(g_MouseKeyList[Scancode - 1]) : false;
}
SPW_PROC(bool) spwMouseReleased_ex(int Scancode)
{
    return (Scancode >= 1 && Scancode <= 3) ? g_InputControl->mouseReleased(g_MouseKeyList[Scancode - 1]) : false;
}
SPW_PROC(int) spwMouseX()
{
    return g_InputControl->getCursorPosition().X;
}
SPW_PROC(int) spwMouseY()
{
    return g_InputControl->getCursorPosition().Y;
}
SPW_PROC(int) spwMouseZ()
{
    return g_MouseZ += g_InputControl->getMouseWheel();
}
SPW_PROC(int) spwMouseXSpeed()
{
    return g_InputControl->getCursorSpeed().X;
}
SPW_PROC(int) spwMouseYSpeed()
{
    return g_InputControl->getCursorSpeed().Y;
}
SPW_PROC(int) spwMouseZSpeed()
{
    return g_InputControl->getMouseWheel();
}
SPW_PROC(void) spwShowPointer()
{
    g_InputControl->setCursorVisible(true);
}
SPW_PROC(void) spwHidePointer()
{
    g_InputControl->setCursorVisible(false);
}
SPW_PROC(void) spwMoveMouse(int X, int Y)
{
    g_InputControl->setCursorPosition(dim::point2di(X, Y));
}
SPW_PROC(void) spwWaitMouse()
{
    while (g_Device->updateEvent() && !spwGetMouse());
}
SPW_PROC(void) spwMouseWait()
{
    spwWaitMouse();
}

SPW_PROC(LP) spwCreateListener(LP Parent, float RollOff = 1.0, float DopplerScale = 1.0, float DistanceScale = 1.0)
{
    g_ListenerParent = NODE(Parent);
    return CAST(g_SoundDevice);
}
SPW_PROC(LP) spwLoadSound(STR File)
{
    return CAST(g_SoundDevice->loadSound(File, 1));
}
SPW_PROC(LP) spwLoad3DSound(STR File)
{
    audio::Sound* Obj = g_SoundDevice->loadSound(File);
    Obj->setVolumetric(true);
    return CAST(Obj);
}
SPW_PROC(void) spwFreeSound(LP Sound)
{
    audio::Sound* Obj = SOUND(Sound);
    g_SoundDevice->deleteSound(Obj);
}
SPW_PROC(void) spwLoopSound(LP Sound)
{
    SOUND(Sound)->setLoop(true);
    SOUND(Sound)->play();
}
SPW_PROC(void) spwPlaySound(LP Sound)
{
    SOUND(Sound)->setLoop(false);
    SOUND(Sound)->play();
}
SPW_PROC(void) spwSoundPitch(LP Sound, int Frequency)
{
    // !TODO
    //SOUND(Sound)->setSpeed(Frequency);
}
SPW_PROC(void) spwSoundPan(LP Sound, float Balance)
{
    SOUND(Sound)->setBalance(Balance);
}
SPW_PROC(void) spwSoundVolume(LP Sound, float Volume)
{
    SOUND(Sound)->setVolume(Volume);
}
SPW_PROC(void) spwEmitSound(LP Sound, LP Entity)
{
    // !TODO
}

SPW_PROC(void) spwSetMelodySpeed_ex(float Speed)
{
    audio::SoundDevice::setMelodySpeed(Speed);
}
SPW_PROC(void) spwPlayMelody_ex(STR CmdStr)
{
    audio::SoundDevice::playMelody(CmdStr);
}

SPW_PROC(void) spwMoveCameraFree(
    LP Camera = 0, float MoveSpeed = 0.25, float TurnSpeed = 0.25, float MaxTurnDegree = 90.0, bool UseArrowKeys = true)
{
    tool::Toolset::moveCameraFree(CAMERA(Camera), MoveSpeed, TurnSpeed, MaxTurnDegree, UseArrowKeys);
}
SPW_PROC(void) spwCombineModels(LP MeshA, LP MeshB, int Mode = 0, float Precision = 0.00001)
{
    tool::ModelCombiner::setPrecision(Precision);
    tool::ModelCombiner().combineModels(MESH(MeshA), MESH(MeshB), (tool::EModelCombinations)Mode);
}

SPW_PROC(bool) spwLoadScriptFile(STR File)
{
    return g_ScriptLoader->loadScriptFile(File);
}


/*
 * ======= Default callback functions =======
 */

void defCallbackUserMaterial(scene::Mesh* Obj, bool isBegin)
{
    SWrapTexture TexData;
    
    for (unsigned int s = 0, l; s < Obj->getMeshBufferCount(); ++s)
    {
        video::MeshBuffer* Surface = Obj->getMeshBuffer(s);
        for (l = 0; l < Surface->getTextureCount(); ++l)
        {
            TexData = g_TextureList[ CAST(Surface->getTexture(l)) ];
            
            Surface->setTextureMatrix(l, TexData.Matrix);
            Surface->setTextureEnv(l, TexData.BlendMode);
            Surface->setMappingGen(l, (TexData.isSphereMap ? video::MAPGEN_SPHERE_MAP : video::MAPGEN_DISABLE));
        }
    }
}


/*
 * ======= Static functions =======
 */

static void InitScancodeList()
{
    g_ScancodeList[1] = 0x1B;   // Escape
    g_ScancodeList[2] = 0x31;   // 1
    g_ScancodeList[3] = 0x32;   // 2
    g_ScancodeList[4] = 0x33;   // 3
    g_ScancodeList[5] = 0x34;   // 4
    g_ScancodeList[6] = 0x35;   // 5
    g_ScancodeList[7] = 0x36;   // 6
    g_ScancodeList[8] = 0x37;   // 7
    g_ScancodeList[9] = 0x38;   // 8
    g_ScancodeList[10] = 0x39;  // 9
    g_ScancodeList[11] = 0x30;  // 0
    //g_ScancodeList[12,0x];    // ί?\
    //g_ScancodeList[13,0x];    // ΄ιθ...
    g_ScancodeList[14] = 0x08;  // Backspace
    g_ScancodeList[15] = 0x09;  // Tab
    g_ScancodeList[16] = 0x51;  // Q
    g_ScancodeList[17] = 0x57;  // W
    g_ScancodeList[18] = 0x45;  // E
    g_ScancodeList[19] = 0x52;  // R
    g_ScancodeList[20] = 0x54;  // T
    g_ScancodeList[21] = 0x5A;  // Z
    g_ScancodeList[22] = 0x55;  // U
    g_ScancodeList[23] = 0x49;  // I
    g_ScancodeList[24] = 0x4F;  // O
    g_ScancodeList[25] = 0x50;  // P
    //g_ScancodeList[260x];     // ά
    g_ScancodeList[27] = 0xBB;  // +*~
    g_ScancodeList[28] = 0x0D;  // Enter
    g_ScancodeList[29] = 0xA2;  // Strg-left
    g_ScancodeList[30] = 0x41;  // A
    g_ScancodeList[31] = 0x53;  // S
    g_ScancodeList[32] = 0x44;  // D
    g_ScancodeList[33] = 0x46;  // F
    g_ScancodeList[34] = 0x47;  // G
    g_ScancodeList[35] = 0x48;  // H
    g_ScancodeList[36] = 0x4A;  // J
    g_ScancodeList[37] = 0x4B;  // K
    g_ScancodeList[38] = 0x4C;  // L
    //g_ScancodeList[390x];     // Φ
    //g_ScancodeList[400x];     // Δ
    g_ScancodeList[41] = 0xDC;  // ^°
    g_ScancodeList[42] = 0xA0;  // Shift-left
    //g_ScancodeList[43];       // '#
    g_ScancodeList[44] = 0x59;  // Y
    g_ScancodeList[45] = 0x58;  // X
    g_ScancodeList[46] = 0x43;  // C
    g_ScancodeList[47] = 0x56;  // V
    g_ScancodeList[48] = 0x42;  // B
    g_ScancodeList[49] = 0x4E;  // N
    g_ScancodeList[50] = 0x4D;  // M
    g_ScancodeList[51] = 0xBC;  // ;
    g_ScancodeList[52] = 0xBE;  // .:
    //g_ScancodeList[53];       // -_
    g_ScancodeList[54] = 0xA1;  // Shift-right
    g_ScancodeList[55] = 0x6A;  // *[Numpad]
    g_ScancodeList[56] = 0xA4;  // Alt-left
    g_ScancodeList[57] = 0x20;  // Space
    g_ScancodeList[58] = 0x14;  // Lock
    g_ScancodeList[59] = 0x70;  // F1
    g_ScancodeList[60] = 0x71;  // F2
    g_ScancodeList[61] = 0x72;  // F3
    g_ScancodeList[62] = 0x73;  // F4
    g_ScancodeList[63] = 0x74;  // F5
    g_ScancodeList[64] = 0x75;  // F6
    g_ScancodeList[65] = 0x76;  // F7
    g_ScancodeList[66] = 0x77;  // F8
    g_ScancodeList[67] = 0x78;  // F9
    g_ScancodeList[68] = 0x79;  // F10
    g_ScancodeList[69] =0x90 ;  // Numblock
    //g_ScancodeList[70];       // Roll
    g_ScancodeList[71] = 0x67;  // 7[Numblock]
    g_ScancodeList[72] = 0x68;  // 8[Numblock]
    g_ScancodeList[73] = 0x69;  // 9[Numblock]
    g_ScancodeList[74] = 0x6D;  // -[Numblock]
    g_ScancodeList[75] = 0x64;  // 4[Numblock]
    g_ScancodeList[76] = 0x65;  // 5[Numblock]
    g_ScancodeList[77] = 0x66;  // 6[Numblock]
    g_ScancodeList[78] = 0x6B;  // +[Numblock]
    g_ScancodeList[79] = 0x61;  // 1[Numblock]
    g_ScancodeList[80] = 0x62;  // 2[Numblock]
    g_ScancodeList[81] = 0x63;  // 3[Numblock]
    g_ScancodeList[82] = 0x60;  // 0[Numblock]
    //g_ScancodeList[83];       // [Numblock]
    //g_ScancodeList[86];       // < and >
    g_ScancodeList[87] = 0x7A;  // F11
    g_ScancodeList[88] = 0x7B;  // F12
    //g_ScancodeList[153];      // Next Track[Media]
    //g_ScancodeList[156];      // Return[Numblock]
    g_ScancodeList[157] = 0xA3; // Strg-right
    //g_ScancodeList[158];      //
    //g_ScancodeList[160];      // Mute[Media]
    //g_ScancodeList[161];      // Calculator [Windows] 
    //g_ScancodeList[162];      // Play/Pause [Media]
    //g_ScancodeList[164];      // Stop [Media]
    //g_ScancodeList[174];      // Volume - [Media]
    //g_ScancodeList[176];      // Volume + [Media]
    g_ScancodeList[178] = 0x24; // Home [Internet]
    g_ScancodeList[181] = 0x6F; // / [Numblock]
    //g_ScancodeList[183];      // Print
    g_ScancodeList[184] = 0x12; // Alt-right
    g_ScancodeList[197] = 0x90; // NumLock
    //g_ScancodeList[198];      // Pause
    //g_ScancodeList[199];      // Pos1
    g_ScancodeList[200] = 0x26; // Up
    g_ScancodeList[201] = 0x21; // Page up
    g_ScancodeList[203] = 0x25; // Left
    g_ScancodeList[205] = 0x27; // Right
    g_ScancodeList[207] = 0x23; // End
    g_ScancodeList[208] = 0x28; // Down
    g_ScancodeList[209] = 0x22; // Page down
    g_ScancodeList[210] = 0x2D; // Insert
    g_ScancodeList[211] = 0x2E; // Remove
    g_ScancodeList[219] = 0x5B; // Windows left
    g_ScancodeList[220] = 0x5C; // Windows right
}

void WrapperError(const io::stringc &Message)
{
    io::Log::message("Wrapper error: " + Message + "!", io::LOG_ERROR);
}

void WrapperError(const io::stringc &FunctionName, const io::stringc &Message)
{
    io::Log::message("Wrapper error (in function \"" + FunctionName + "\"): " + Message + "!", io::LOG_ERROR);
}



// ================================================================================
