/*
 * SoftPixel Device Android file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "Platform/spSoftPixelDeviceAndroid.hpp"
#include "Base/spInternalDeclarations.hpp"

#if defined(SP_PLATFORM_ANDROID)


#include "Base/spSharedObjects.hpp"

//#define __DEBUG_JNI__
#ifdef __DEBUG_JNI__
#   include "SoftPixelEngine.hpp"
#endif


sp::s32 __spAndroidInputEvent = 0;

static void spAndroidHandleCommand(android_app* App, int32_t Cmd);
static int32_t spAndroidHandleInput(android_app* App, AInputEvent* Event);

namespace sp
{


/*
 * Internal member
 */

extern SoftPixelDevice*             __spDevice;
extern video::RenderSystem*         __spVideoDriver;
extern video::RenderContext*        __spRenderContext;
extern scene::SceneGraph*           __spSceneManager;
extern io::InputControl*            __spInputControl;
extern io::OSInformator*            __spOSInformator;
extern gui::GUIManager*             __spGUIManager;


/*
 * SoftPixel Engine device class
 */

PFNSAVESTATECALLBACKPROC SoftPixelDeviceAndroid::SaveStateRead_ = 0;
PFNSAVESTATECALLBACKPROC SoftPixelDeviceAndroid::SaveStateWrite_ = 0;

SoftPixelDeviceAndroid::SoftPixelDeviceAndroid(
    android_app* App, const video::ERenderSystems RendererType, const io::stringc &Title, bool isFullscreen) :
    SoftPixelDevice(
        RendererType, 0, 32, Title, isFullscreen, SDeviceFlags()
    ),
    App_                (App),
    SensorManager_      (0  ),
    SensorEventQueue_   (0  ),
    GyroscopeSensor_    (0  ),
    AccelerometerSensor_(0  ),
    LightSensor_        (0  )
{
    /* Make sure glue is not stripped */
    //app_dummy();
    
    /* General settings */
    isActive_       = false;
    isInitWindow_   = false;
    isTermWindow_   = false;
    
    /* Check for valid app value */
    if (!App_)
    {
        io::Log::error("Android app object is invalid");
        return;
    }
    
    /* Setup app status */
    App_->userData      = this;
    App_->onAppCmd      = spAndroidHandleCommand;
    App_->onInputEvent  = spAndroidHandleInput;
    
    /* Setup asset manager */
    io::FileAsset::AssetManager_ = App_->activity->assetManager;
    
    /* Create hardware sensor objects */
    SensorManager_          = ASensorManager_getInstance();
    AccelerometerSensor_    = ASensorManager_getDefaultSensor(SensorManager_, ASENSOR_TYPE_ACCELEROMETER);
    GyroscopeSensor_        = ASensorManager_getDefaultSensor(SensorManager_, ASENSOR_TYPE_MAGNETIC_FIELD);
    //LightSensor_            = ASensorManager_getDefaultSensor(SensorManager_, ASENSOR_TYPE_LIGHT);
    SensorEventQueue_       = ASensorManager_createEventQueue(SensorManager_, App_->looper, LOOPER_ID_USER, 0, 0);
    
    /* Check for saved state */
    if (App_->savedState && SoftPixelDeviceAndroid::SaveStateRead_)
        SoftPixelDeviceAndroid::SaveStateRead_(App_->savedState, App_->savedStateSize);
    
    /* Wait until the command-handler-thread got the window-init command */
    updateEvents();
}
SoftPixelDeviceAndroid::~SoftPixelDeviceAndroid()
{
    deleteDevice();
}

void SoftPixelDeviceAndroid::processEvent(int32_t Cmd)
{
    switch (Cmd)
    {
        case APP_CMD_INIT_WINDOW:
        {
            sp::io::Log::message("DEB: APP_CMD_INIT_WINDOW");
            
            if (App_->window)
            {
                isInitWindow_   = true;
                isActive_       = true;
            }
        }
        break;
        
        case APP_CMD_SAVE_STATE:
        {
            if (SoftPixelDeviceAndroid::SaveStateWrite_)
                SoftPixelDeviceAndroid::SaveStateWrite_(App_->savedState, App_->savedStateSize);
        }
        break;
        
        case APP_CMD_TERM_WINDOW:
        {
            sp::io::Log::message("DEB: APP_CMD_TERM_WINDOW");
            
            /* Stop main activity */
            isTermWindow_ = true;
        }
        break;
        
        case APP_CMD_GAINED_FOCUS:
        {
            sp::io::Log::message("DEB: APP_CMD_GAINED_FOCUS");
            
            /* Configure accelerometer sensor */
            if (AccelerometerSensor_)
            {
                ASensorEventQueue_enableSensor(SensorEventQueue_, AccelerometerSensor_);
                ASensorEventQueue_setEventRate(SensorEventQueue_, AccelerometerSensor_, (1000L/60)*1000);
            }
            
            /* Configure gyroscope sensor */
            if (GyroscopeSensor_)
            {
                ASensorEventQueue_enableSensor(SensorEventQueue_, GyroscopeSensor_);
                ASensorEventQueue_setEventRate(SensorEventQueue_, GyroscopeSensor_, (1000L/60)*1000);
            }
            
            /* Configure light sensor */
            if (LightSensor_)
            {
                ASensorEventQueue_enableSensor(SensorEventQueue_, LightSensor_);
                ASensorEventQueue_setEventRate(SensorEventQueue_, LightSensor_, (1000L/60)*1000);
            }
        }
        break;
        
        case APP_CMD_LOST_FOCUS:
        {
            sp::io::Log::message("DEB: APP_CMD_LOST_FOCUS");
            
            if (AccelerometerSensor_)
                ASensorEventQueue_disableSensor(SensorEventQueue_, AccelerometerSensor_);
            if (GyroscopeSensor_)
                ASensorEventQueue_disableSensor(SensorEventQueue_, GyroscopeSensor_);
            if (LightSensor_)
                ASensorEventQueue_disableSensor(SensorEventQueue_, LightSensor_);
        }
        break;
    }
}

bool SoftPixelDeviceAndroid::updateEvents()
{
    do
    {
        #if 1
        if (!isActive_)
            io::Log::message("DEB: inactive");
        #endif
        
        if (!updateNextEvent())
            return false;
    }
    while (!isActive_);
    
    return true;
}

void SoftPixelDeviceAndroid::deleteDevice()
{
    stopActivity();
    
    io::Log::message("DEB: deleteDevice: stopded Activity");
    
    /* Close the possible debug log file */
    io::Log::close();
}

void SoftPixelDeviceAndroid::setSaveStateReadCallback(PFNSAVESTATECALLBACKPROC Callback)
{
    SaveStateRead_ = Callback;
}
void SoftPixelDeviceAndroid::setSaveStateWriteCallback(PFNSAVESTATECALLBACKPROC Callback)
{
    SaveStateWrite_ = Callback;
}


/*
 * ======= Private: =======
 */

bool SoftPixelDeviceAndroid::openGraphicsScreen()
{
    /* Get queried screen resolution */
    Resolution_.Width   = gSharedObjects.ScreenWidth;
    Resolution_.Height  = gSharedObjects.ScreenHeight;
    
    /* Create render context */
    if (!__spRenderContext->openGraphicsScreen(App_, Resolution_, Title_, ColorDepth_, isFullscreen_, Flags_.isVsync))
    {
        io::Log::error("Could not create render context");
        return false;
    }
    
    return true;
}

void SoftPixelDeviceAndroid::startActivity()
{
    io::Log::message("DEB: startActivity");
    
    isInitWindow_ = false;
    
    if (!__spVideoDriver)
    {
        /* Create renderer device */
        createRenderSystem();
        
        /* Create window, renderer context and open the screen */
        if (openGraphicsScreen())
        {
            __spVideoDriver->setupConfiguration();
            __spVideoDriver->createDefaultResources();
            __spRenderContext->setVsync(Flags_.isVsync);
        }
        
        /* Print console header */
        printConsoleHeader();
    }
}
void SoftPixelDeviceAndroid::stopActivity()
{
    io::Log::message("DEB: stopActivity");
    
    isTermWindow_ = false;
    
    if (__spVideoDriver)
    {
        /* Clear renderer resources */
        __spVideoDriver->clearTextureList();
        __spVideoDriver->clearBuffers();
        __spVideoDriver->deleteDefaultResources();
        
        /* Close screen and delete resource devices  */
        __spRenderContext->closeGraphicsScreen();
        deleteResourceDevices();
        
        io::Log::message("DEB: stoped activity successful and deleteed resource devices");
    }
}

bool SoftPixelDeviceAndroid::updateNextEvent()
{
    s32 Ident, Events;
    android_poll_source* Source;
    
    /* Reset keyboard and mouse information */
    memset(__wasKey, 0, sizeof(bool) * (io::KEY_BUTTON_MODE + 1));
    memset(__hitKey, 0, sizeof(bool) * (io::KEY_BUTTON_MODE + 1));
    
    /* Update base events */
    updateBaseEvents();
    
    __spAndroidInputEvent = 0;
    
    if (__spInputControl)
        __spInputControl->isMotionEvent_ = false;
    
    /* Wait for events */
    while ( ( Ident = ALooper_pollAll(isActive_ ? 0 : -1, 0, &Events, (void**)&Source) ) >= 0 )
    {
        /* Process current event */
        if (Source)
            Source->process(App_, Source);
        
        /* Process sensor events */
        processSensorEvent(Ident);
        
        /* Check if we are exiting */
        if (App_->destroyRequested)
        {
            io::Log::message("DEB: destroyRequested");
            return false;
        }
    }
    
    /* Reset cursor speed */
    if (__spInputControl && !__spAndroidInputEvent)
        __spInputControl->resetInputEvents();
    
    /* Start/stop main activity */
    if (isInitWindow_)
        startActivity();
    /*if (isTermWindow_)
        stopActivity();*/
    
    return true;
}

void SoftPixelDeviceAndroid::processSensorEvent(s32 Ident)
{
    if (Ident == LOOPER_ID_USER && GyroscopeSensor_ && __spInputControl)
    {
        ASensorEvent Event;
        
        while (ASensorEventQueue_getEvents(SensorEventQueue_, &Event, 1) > 0)
        {
            switch (Event.type)
            {
                case ASENSOR_TYPE_ACCELEROMETER:
                    __spInputControl->AccelerometerSensor_ = dim::vector3df(
                        Event.acceleration.x, Event.acceleration.y, Event.acceleration.z
                    );
                    break;
                    
                case ASENSOR_TYPE_MAGNETIC_FIELD:
                    __spInputControl->GyroscopeSensor_ = dim::vector3df(
                        Event.magnetic.x, Event.magnetic.y, Event.magnetic.z
                    );
                    break;
                    
                case ASENSOR_TYPE_LIGHT:
                    __spInputControl->LightSensor_ = Event.light;
                    break;
            }
        }
    }
}


} // /namespace sp


/*
 * ======= Static functions: =======
 */
 
static void spAndroidHandleCommand(android_app* App, int32_t Cmd)
{
    static_cast<sp::SoftPixelDeviceAndroid*>(App->userData)->processEvent(Cmd);
}

static int32_t spAndroidHandleInput(android_app* App, AInputEvent* Event)
{
    if (!sp::__spInputControl)
        return 0;
    
    __spAndroidInputEvent = sp::__spInputControl->processEvent(App, Event);
    
    return __spAndroidInputEvent ? 1 : 0;
}


#ifdef __DEBUG_JNI__

using namespace sp;

SoftPixelDevice* spDevice       = 0;
video::RenderSystem* spRenderer = 0;
video::RenderContext* spContext = 0;
scene::SceneGraph* spScene      = 0;
io::InputControl* spControl     = 0;
audio::SoundDevice* spListener  = 0;

scene::Camera* Cam  = 0;
scene::Light* Lit   = 0;
scene::Mesh* Obj    = 0;

/* === Functions === */

void ShaderCallbackGLES2(video::ShaderClass* ShdTable, const scene::MaterialNode* Object)
{
    ShdTable->getVertexShader()->setConstant(
        "WorldMatrix", spRenderer->getWorldMatrix()
    );
    
    dim::matrix4f Mat;
    //Mat[10] = -1;
    
    ShdTable->getVertexShader()->setConstant(
        "WorldViewProjectionMatrix",
        spRenderer->getProjectionMatrix() * Mat * (spRenderer->getViewMatrix() * spRenderer->getWorldMatrix())
    );
}

/* === Java bindings === */

void android_main(android_app* App)
{
    io::Log::message("DEB: main function");
    
    spDevice    = createGraphicsDevice(
        App, video::RENDERER_OPENGLES2, "SoftPixel Engine - Test1"
    );
    
    //App.activity.assetManager;
    
    io::Log::message("DEB: created graphics device");
    
    spRenderer  = spDevice->getRenderSystem();
    spContext   = spDevice->getRenderContext();
    spScene     = spDevice->getSceneGraph();
    spControl   = spDevice->getInputControl();
    spListener  = spDevice->getSoundDevice();
    
    // Create camera
    Cam = spScene->createCamera(),
    Cam->setPosition(dim::vector3df(0, 0, -3));
    Cam->setRange(0.1f, 150.0f);
    
    // Create light
    Lit = spScene->createLight();
    Lit->setRotation(dim::vector3df(25, 25, 0));
    spScene->setLighting(true);
    
    dim::vector3df Pos(-10, 0, 10);
    
    video::VertexFormatUniversal* VertFmt = 0;
    
    // Vertex format
    if (spRenderer->getRendererType() == video::RENDERER_OPENGLES2)
    {
        VertFmt = spRenderer->createVertexFormat<video::VertexFormatUniversal>();
        
        VertFmt->addUniversal(video::DATATYPE_FLOAT, 3, "VertexCoord", false, video::VERTEXFORMAT_COORD);
        VertFmt->addUniversal(video::DATATYPE_FLOAT, 3, "VertexNormal", false, video::VERTEXFORMAT_NORMAL);
        VertFmt->addUniversal(video::DATATYPE_FLOAT, 2, "VertexTexCoord", false, video::VERTEXFORMAT_TEXCOORDS);
        
        scene::SceneGraph::setDefaultVertexFormat(VertFmt);
    }
    
    // Create skybox
    video::Texture* TexSkyList[6] = { 0 };
    spRenderer->setTextureGenFlags(video::TEXGEN_WRAP, video::TEXWRAP_CLAMP);
    TexSkyList[0] = spRenderer->loadTexture("SkyboxNorth.jpg");
    TexSkyList[1] = spRenderer->loadTexture("SkyboxSouth.jpg");
    TexSkyList[2] = spRenderer->loadTexture("SkyboxTop.jpg");
    TexSkyList[3] = spRenderer->loadTexture("SkyboxBottom.jpg");
    TexSkyList[4] = spRenderer->loadTexture("SkyboxWest.jpg");
    TexSkyList[5] = spRenderer->loadTexture("SkyboxEast.jpg");
    spRenderer->setTextureGenFlags(video::TEXGEN_WRAP, video::TEXWRAP_REPEAT);
    
    scene::Mesh* SkyBox = spScene->createSkyBox(TexSkyList, 10.0f);
    
    // Load mesh
    scene::Mesh* Obj = spScene->loadMesh("beast.b3d");
    Obj->setScale(0.05f);
    Obj->setPosition(dim::vector3df(0, 0, 5));
    
    for (u32 s = 0; s < Obj->getMeshBufferCount(); ++s)
        Obj->getMeshBuffer(s)->clearTextureList();
    Obj->addTexture(spRenderer->loadTexture("beast1.jpg"));
    
    io::Log::message("created scene successful");
    
    // Shader program
    if (spRenderer->getRendererType() == video::RENDERER_OPENGLES2)
    {
        video::ShaderClass* ShdTable = spRenderer->createShaderClass();
        
        video::Shader* VertShader = spRenderer->loadShader(
            ShdTable, video::SHADER_VERTEX, video::GLSL_VERSION_1_20, "ProceduralTextures.glvert"
        );
        video::Shader* FragShader = spRenderer->loadShader(
            ShdTable, video::SHADER_PIXEL, video::GLSL_VERSION_1_20, "ProceduralTextures.glfrag"
        );
        
        if (ShdTable->link())
        {
            ShdTable->setObjectCallback(ShaderCallbackGLES2);
            
            if (Obj)
                Obj->setShaderClass(ShdTable);
            if (SkyBox)
                SkyBox->setShaderClass(ShdTable);
            
            ShdTable->getPixelShader()->setConstant("Tex", 0);
            
            io::Log::message("linking shader successul");
        }
        else
        {
            ShdTable = 0;
            
            io::Log::error("linking shader failed");
        }
    }
    
    // Play animation
    /*if (Obj)
    {
        scene::AnimationSkeletal* Anim = static_cast<scene::AnimationSkeletal*>(Obj->getAnimation());
        if (Anim)
            Anim->play(scene::ANIM_LOOP);
    }*/
    
    spRenderer->setClearColor(255);
    
    spControl->setKeyEnable(io::KEY_VOLUME_UP, false);
    spControl->setKeyEnable(io::KEY_VOLUME_DOWN, false);
    
    f32 Volume = 1.0f;
    
    // Load sound
    audio::Sound* Music = 0;
    
    /*Music = spListener->loadSound("124-codename-st.-anne.mp3");
    if (Music)
    {
        Music->play();
        Music->setSeek(0.5f);
    }*/
    
    while (spDevice->updateEvents())
    {
        spRenderer->clearBuffers();
        
        //const dim::point2df MouseSpeed(spControl->getMotionSpeed());
        //const f32 DragSpeed = spControl->getDragSpeed();
        
        spScene->updateAnimations();
        spScene->renderScene();
        
        if (spControl->keyHit(io::KEY_VOLUME_UP))
            Volume += 0.1f;
        if (spControl->keyHit(io::KEY_VOLUME_DOWN))
            Volume -= 0.1f;
        
        math::Clamp(Volume, 0.0f, 1.0f);
        //Anim->setSpeed(Volume);
        
        if (Music)
            Music->setVolume(Volume);
        
        tool::Toolset::moveCameraFree();
        
        if (SkyBox)
            SkyBox->setPosition(Cam->getPosition(true));
        
        spContext->flipBuffers();
    }
    
    io::Log::message("DEB: leave main function");
    
    deleteDevice();
    
    io::Log::message("DEB: return from main function");
}

#endif


#endif



// ================================================================================
