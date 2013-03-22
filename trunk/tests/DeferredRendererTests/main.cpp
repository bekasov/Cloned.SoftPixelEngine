//
// SoftPixel Engine - DeferredRenderer Tests
//

#include <SoftPixelEngine.hpp>
#include <RenderSystem/DeferredRenderer/spShadowMapper.hpp>

using namespace sp;

#ifdef SP_COMPILE_WITH_DEFERREDRENDERER

#include "../common.hpp"

SP_TESTS_DECLARE

video::Texture* DiffuseMap  = 0;
video::Texture* NormalMap   = 0;
video::Texture* HeightMap   = 0;

static void SetupTextures(scene::Mesh* Obj)
{
    if (Obj)
    {
        Obj->addTexture(DiffuseMap);
        Obj->addTexture(NormalMap);
        Obj->addTexture(HeightMap);
        
        Obj->updateTangentSpace(1, 2, false);
    }
}

static void SetupMaterial(scene::Mesh* Obj, bool AutoMap = false, f32 Density = 0.7f)
{
    if (Obj)
    {
        if (AutoMap)
            Obj->textureAutoMap(0, Density);
        
        Obj->getMaterial()->setBlending(false);
        //Obj->setShaderClass(DefRenderer->getGBufferShader());
    }
}

static void SetupShading(scene::Mesh* Obj, bool AutoMap = false, f32 Density = 0.7f, s32 Surface = -1)
{
    #if 1
    
    if (Obj)
    {
        if (Surface >= 0)
        {
            video::MeshBuffer* Surf = Obj->getMeshBuffer(static_cast<u32>(Surface));
            
            if (Surf)
            {
                Surf->addTexture(DiffuseMap);
                Surf->addTexture(NormalMap);
                Surf->addTexture(HeightMap);
                
                if (AutoMap)
                    Obj->textureAutoMap(0, Density, static_cast<u32>(Surface));
                
                Surf->updateTangentSpace(1, 2, false);
            }
        }
        else
        {
            Obj->addTexture(DiffuseMap);
            Obj->addTexture(NormalMap);
            Obj->addTexture(HeightMap);
            
            /**
            !TODO! -> exchanging the next two called ('textureAutoMap' and
            'updateTangentSpace') will procduce an error (graphics error).
            */
            
            if (AutoMap)
                Obj->textureAutoMap(0, Density);
            
            Obj->updateTangentSpace(1, 2, false);
        }
        
        Obj->getMaterial()->setBlending(false);
    }
    
    #else
    
    SetupTextures(Obj);
    SetupMaterial(Obj, AutoMap, Density);
    
    #endif
}

static scene::Mesh* CreateBox(const dim::vector3df &Pos, f32 RotationY)
{
    static scene::Mesh* ObjRef;
    
    if (!ObjRef)
    {
        ObjRef = spSceneMngr->createMesh(scene::MESH_CUBE);
        SetupTextures(ObjRef);
    }
    
    scene::Mesh* Obj = spScene->createMesh();
    Obj->setReference(ObjRef);
    
    Obj->setPosition(Pos);
    Obj->setRotation(dim::vector3df(0, RotationY, 0));
    
    SetupMaterial(Obj, true);
    
    return Obj;
}

scene::Light* CreateSpotLight(const dim::vector3df &Pos, const video::color &Color = 255, bool Shadow = true)
{
    scene::Light* SpotLit = spScene->createLight(scene::LIGHT_SPOT);
    
    SpotLit->setSpotCone(15.0f, 20.0f);
    //SpotLit->setSpotCone(15.0f, 30.0f);
    //SpotLit->setSpotCone(43.0f, 45.0f);
    SpotLit->setDiffuseColor(Color);
    SpotLit->setPosition(Pos);
    SpotLit->setShadow(Shadow);
    
    return SpotLit;
}

scene::Light* CreatePointLight(const dim::vector3df &Pos, const video::color &Color = 255, f32 Radius = 100.0f)
{
    scene::Light* PointLight = spScene->createLight(scene::LIGHT_POINT);

    PointLight->setPosition(Pos);
    PointLight->setDiffuseColor(Color);
    PointLight->setVolumetric(true);
    PointLight->setVolumetricRadius(Radius);

    return PointLight;
}

video::Texture* CreateSimpleTexture(const video::color &Color)
{
    video::Texture* Tex = spRenderer->createTexture(1);
    Tex->getImageBuffer()->setPixelColor(0, Color);
    Tex->updateImageBuffer();
    return Tex;
}

/**
Timing documentation:
=====================
DEBUG Mode, Normal-Mapping Only, 1280x768:
    490 FPS

DEBUG Mode, Normal-Mapping Only, 1280x768, Unform Optimization:
    530 FPS


*/

int main()
{
    SP_TESTS_INIT_EX2(
        video::RENDERER_OPENGL,
        //video::RENDERER_DIRECT3D11,
        dim::size2di(1024, 600),
        //video::VideoModeEnumerator().getDesktop().Resolution,
        "DeferredRenderer",
        false,
        SDeviceFlags()
    )
    
    spContext->setVsync(false);
    
    // Create deferred renderer
    video::DeferredRenderer* DefRenderer = new video::DeferredRenderer();
    
    DefRenderer->generateResources(
        video::DEFERREDFLAG_NORMAL_MAPPING
        //| video::DEFERREDFLAG_PARALLAX_MAPPING
        //| video::DEFERREDFLAG_BLOOM
        | video::DEFERREDFLAG_SHADOW_MAPPING
        | video::DEFERREDFLAG_GLOBAL_ILLUMINATION
        
        #if 0
        | video::DEFERREDFLAG_DEBUG_GBUFFER
        | video::DEFERREDFLAG_DEBUG_GBUFFER_WORLDPOS
        //| video::DEFERREDFLAG_DEBUG_GBUFFER_TEXCOORDS
        #endif
        
        //,256,50,0
        ,256,15,15
    );
    
    //DefRenderer->setAmbientColor(0.0f);
    
    // Load textures
    const io::stringc Path = "../../help/tutorials/ShaderLibrary/media/";
    
    spRenderer->setTextureGenFlags(video::TEXGEN_MIPMAPFILTER, video::FILTER_ANISOTROPIC);
    spRenderer->setTextureGenFlags(video::TEXGEN_ANISOTROPY, 8);
    
    DiffuseMap  = spRenderer->loadTexture(Path + "StoneColorMap.jpg");
    NormalMap   = spRenderer->loadTexture(Path + "StoneNormalMap.jpg");
    HeightMap   = spRenderer->loadTexture("StonesHeightMap.jpg");
    
    video::Texture* DefDiffuseMap = CreateSimpleTexture(video::color(255));
    video::Texture* DefNormalMap = CreateSimpleTexture(video::color(128, 128, 255));
    
    video::Texture* RedColorMap = CreateSimpleTexture(video::color(255, 0, 0));
    video::Texture* GreenColorMap = CreateSimpleTexture(video::color(0, 255, 0));
    
    // Create scene
    //Cam->setPosition(dim::vector3df(0, 0, -1.5f));
    
    math::Randomizer::seedRandom();
    
    scene::SceneManager::setDefaultVertexFormat(DefRenderer->getVertexFormat());
    
    #define SCENE_STANDARD      1
    #define SCENE_CORNELLBOX    2
    #define SCENE_POINTLIGHTS   3

    #define SCENE               SCENE_CORNELLBOX//SCENE_POINTLIGHTS

    #if SCENE == SCENE_CORNELLBOX
    
    scene::SceneManager::setTextureLoadingState(false);
    
    scene::Mesh* Obj = spScene->loadMesh("CornellBox.spm");
    Obj->setScale(4);
    
    scene::SceneManager::setTextureLoadingState(true);
    
    #   define SIMPLE_TEXTURING
    #   ifdef SIMPLE_TEXTURING
    DiffuseMap = DefDiffuseMap;
    //NormalMap = DefNormalMap;
    #   endif
    
    SetupShading(Obj, true, 0.35f, 1);
    
    #   ifdef SIMPLE_TEXTURING
    DiffuseMap = RedColorMap;
    #   else
    DiffuseMap  = spRenderer->loadTexture("Tiles.jpg");
    NormalMap   = spRenderer->loadTexture("Tiles_NORM.png");
    #   endif
    SetupShading(Obj, true, 0.35f, 0);
    
    #   ifdef SIMPLE_TEXTURING
    DiffuseMap = GreenColorMap;
    #   endif
    SetupShading(Obj, true, 0.35f, 2);
    
    #elif SCENE == SCENE_STANDARD || SCENE == SCENE_POINTLIGHTS
    
    scene::Mesh* Obj = spScene->loadMesh("TestScene.spm");
    Obj->setScale(2);
    
    SetupShading(Obj, true, 0.35f);
    
    #else
    
    scene::Mesh* Obj = spScene->createMesh(scene::MESH_CUBE);
    
    SetupShading(Obj, true, 0.35f);
    
    #endif
    
    // Create boxes
    #if SCENE != SCENE_CORNELLBOX && 1
    
    f32 Rot = 0.0f;
    for (s32 i = -5; i <= 5; ++i)
    {
        CreateBox(dim::vector3df(1.5f*i, -1.5f, 0), Rot);
        Rot += 9.0f;
    }
    
    #endif
    
    // Setup lighting
    scene::Light* Lit = spScene->getLightList().front();
    
    Lit->setLightModel(scene::LIGHT_POINT);
    Lit->setPosition(dim::vector3df(3.0f, 1.0f, 0.0f));
    Lit->setVolumetric(true);
    Lit->setVolumetricRadius(50.0f);
    
    #if SCENE != SCENE_STANDARD
    Lit->setVisible(false);
    #elif 0
    scene::Mesh* obj = spScene->createMesh(scene::MESH_CUBE);
    obj->translate(0.5f);
    SetupShading(obj);
    #endif
    
    //#define MULTI_SPOT_LIGHT
    #ifdef MULTI_SPOT_LIGHT
    
    Lit->setVisible(false);
    
    const u32 LightCount = 40;
    
    std::vector<scene::Light*> MultiLights(LightCount);
    //std::vector<dim::vector3df> MultiDirs(LightCount);
    
    for (u32 i = 0; i < LightCount; ++i)
    {
        MultiLights[i] = CreateSpotLight(
            dim::vector3df(
                math::Randomizer::randFloat(-5.0f, 5.0f),
                0,
                math::Randomizer::randFloat(-5.0f, 5.0f)
            ),
            math::Randomizer::randColor(),
            !false // Shadow?
        );
        MultiLights[i]->setRotation(math::Randomizer::randVector() * 360.0f);
        //MultiDirs[i] = math::Randomizer::randVector();
    }
    
    #else
    
    #   if SCENE == SCENE_CORNELLBOX
    scene::Light* SpotLit = CreateSpotLight(dim::vector3df(-3, 0, 0));
    //SpotLit->setSpotCone(40, 45);
    #   elif SCENE == SCENE_STANDARD
    scene::Light* SpotLit = CreateSpotLight(dim::vector3df(-3, 0, 0), video::color(255, 32, 32));
    #   elif SCENE == SCENE_POINTLIGHTS

    static const u32 LightCount = 50;

    for (u32 i = 0; i < LightCount; ++i)
    {
        CreatePointLight(
            dim::vector3df(
                math::Randomizer::randFloat(-7.0f, 7.0f),
                math::Randomizer::randFloat(-1.5f, 1.5f),
                math::Randomizer::randFloat(-7.0f, 7.0f)
            ),
            math::Randomizer::randColor(),
            2.0f
        );
    }

    #   endif
    
    #endif
    
    // Create font
    video::Font* Fnt = spRenderer->createFont("Arial", 20, video::FONT_BOLD);
    
    f64 MinFPS = 999999.0, MaxFPS = 0.0, AvgFPS = 0.0;
    u32 Samples = 0;
    
    tool::CommandLineUI* Cmd = new tool::CommandLineUI();
    bool isCmdActive = false;
    Cmd->setBackgroundColor(video::color(0, 0, 0, 128));
    Cmd->setRect(dim::rect2di(0, 0, spContext->getResolution().Width, spContext->getResolution().Height));
    spControl->setWordInput(isCmdActive);

    Cam->setPosition(0);
    
    // Main loop
    while (spDevice->updateEvents() && !spControl->keyDown(io::KEY_ESCAPE))
    {
        spRenderer->clearBuffers();
        
        // Update scene
        #if defined(MULTI_SPOT_LIGHT)
        
        #   if 0
        for (u32 i = 0; i < MultiLights.size(); ++i)
            MultiLights[i]->turn(dim::vector3df(0.5f, 0, 0) * io::Timer::getGlobalSpeed());
        #   endif
        
        #elif SCENE == SCENE_STANDARD || SCENE == SCENE_CORNELLBOX
        
        f32 LitTurnSpeed = io::Timer::getGlobalSpeed();
        f32 LitMoveSpeed = io::Timer::getGlobalSpeed() * 0.1f;

        if (spControl->keyDown(io::KEY_PAGEUP))
            SpotLit->turn(dim::vector3df(0, LitTurnSpeed, 0));
        if (spControl->keyDown(io::KEY_PAGEDOWN))
            SpotLit->turn(dim::vector3df(0, -LitTurnSpeed, 0));
        
        if (spControl->keyDown(io::KEY_INSERT))
            SpotLit->turn(dim::vector3df(LitTurnSpeed, 0, 0));
        if (spControl->keyDown(io::KEY_DELETE))
            SpotLit->turn(dim::vector3df(-LitTurnSpeed, 0, 0));
        
        if (spControl->keyDown(io::KEY_NUMPAD4))
            SpotLit->translate(dim::vector3df(-LitMoveSpeed, 0, 0));
        if (spControl->keyDown(io::KEY_NUMPAD6))
            SpotLit->translate(dim::vector3df(LitMoveSpeed, 0, 0));
        if (spControl->keyDown(io::KEY_NUMPAD5))
            SpotLit->translate(dim::vector3df(0, 0, -LitMoveSpeed));
        if (spControl->keyDown(io::KEY_NUMPAD8))
            SpotLit->translate(dim::vector3df(0, 0, LitMoveSpeed));

        #endif
        
        if (spContext->isWindowActive() && !isCmdActive)
        {
            tool::Toolset::moveCameraFree(
                0, (spControl->keyDown(io::KEY_SHIFT) ? 0.25f : 0.125f) * io::Timer::getGlobalSpeed()
            );
        }
        
        #if 1
        if (!isCmdActive)
        {
            s32 w = spControl->getMouseWheel();
            if (w)
            {
                static f32 g = 0.6f;
                g += static_cast<f32>(w) * 0.1f;
                DefRenderer->getBloomEffect()->setFactor(g);
            }
        }
        #endif
        
        // Render scene
        #if 1
        DefRenderer->renderScene(spScene, Cam);
        #else
        spScene->renderScene();
        #endif
        
        #if 1
        
        f64 FPS = io::Timer::getFPS();
        
        if (spControl->keyHit(io::KEY_RETURN))
        {
            MinFPS = 999999.0;
            MaxFPS = 0.0;
            AvgFPS = 0.0;
            Samples = 0;
        }
        
        ++Samples;
        AvgFPS += FPS;
        
        math::Increase(MaxFPS, FPS);
        math::Decrease(MinFPS, FPS);
        
        spRenderer->draw2DText(Fnt, dim::point2di(15, 15), "FPS: " + io::stringc(FPS));
        spRenderer->draw2DText(Fnt, dim::point2di(15, 40), "Min: " + io::stringc(MinFPS));
        spRenderer->draw2DText(Fnt, dim::point2di(15, 65), "Max: " + io::stringc(MaxFPS));
        spRenderer->draw2DText(Fnt, dim::point2di(15, 90), "Avg: " + io::stringc(AvgFPS / Samples));
        
        spRenderer->draw2DText(Fnt, dim::point2di(15, 125), "Draw Calls: " + io::stringc(video::RenderSystem::queryDrawCalls()));
        spRenderer->draw2DText(Fnt, dim::point2di(15, 150), "MeshBuffer Bindings: " + io::stringc(video::RenderSystem::queryMeshBufferBindings()));
        spRenderer->draw2DText(Fnt, dim::point2di(15, 175), "TextureLayer Bindings: " + io::stringc(video::RenderSystem::queryTextureLayerBindings()));
        
        #endif
        
        if (spControl->keyHit(io::KEY_F3))
        {
            isCmdActive = !isCmdActive;
            spControl->setWordInput(isCmdActive);
        }
        
        if (isCmdActive)
            Cmd->render();
        
        spContext->flipBuffers();
    }
    
    delete Cmd;
    delete DefRenderer;
    
    deleteDevice();
    
    return 0;
}

#else

int main()
{
    io::Log::error("This engine was not compiled with deferred renderer");
    io::Log::pauseConsole();
    return 0;
}

#endif
