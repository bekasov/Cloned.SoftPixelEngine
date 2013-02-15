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

static void SetupShading(scene::Mesh* Obj, bool AutoMap = false, f32 Density = 0.7f)
{
    if (Obj)
    {
        if (AutoMap)
            Obj->textureAutoMap(0, Density);
        
        Obj->addTexture(DiffuseMap);
        Obj->addTexture(NormalMap);
        Obj->addTexture(HeightMap);
        
        Obj->updateTangentSpace(1, 2, false);
        
        Obj->getMaterial()->setBlending(false);
        //Obj->setShaderClass(DefRenderer->getGBufferShader());
    }
}

static scene::Mesh* CreateBox(const dim::vector3df &Pos, f32 RotationY)
{
    scene::Mesh* Obj = spScene->createMesh(scene::MESH_CUBE);
    
    Obj->setPosition(Pos);
    Obj->setRotation(dim::vector3df(0, RotationY, 0));
    
    SetupShading(Obj, true);
    
    return Obj;
}

scene::Light* CreateSpotLight(const dim::vector3df &Pos, const video::color &Color = 255, bool Shadow = true)
{
    scene::Light* SpotLit = spScene->createLight(scene::LIGHT_SPOT);
    
    SpotLit->setSpotCone(15.0f, 30.0f);
    SpotLit->setDiffuseColor(Color);
    SpotLit->setPosition(Pos);
    SpotLit->setShadow(Shadow);
    
    return SpotLit;
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
        dim::size2di(1280, 768),
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
        //| video::DEFERREDFLAG_SHADOW_MAPPING
        
        #if 0
        | video::DEFERREDFLAG_DEBUG_GBUFFER
        | video::DEFERREDFLAG_DEBUG_GBUFFER_WORLDPOS
        | video::DEFERREDFLAG_DEBUG_GBUFFER_TEXCOORDS
        #endif
        
        ,256,1,15
    );
    
    //DefRenderer->setAmbientColor(0.0f);
    
    // Load textures
    const io::stringc Path = "../../help/tutorials/ShaderLibrary/media/";
    
    spRenderer->setTextureGenFlags(video::TEXGEN_MIPMAPFILTER, video::FILTER_ANISOTROPIC);
    spRenderer->setTextureGenFlags(video::TEXGEN_ANISOTROPY, 8);
    
    DiffuseMap  = spRenderer->loadTexture(Path + "StoneColorMap.jpg");
    NormalMap   = spRenderer->loadTexture(Path + "StoneNormalMap.jpg");
    HeightMap   = spRenderer->loadTexture("StonesHeightMap.jpg");
    
    // Create scene
    Cam->setPosition(dim::vector3df(0, 0, -1.5f));
    
    math::Randomizer::seedRandom();
    
    scene::SceneManager::setDefaultVertexFormat(DefRenderer->getVertexFormat());
    
    #define SCENE_WORLD
    #ifdef SCENE_WORLD
    
    scene::Mesh* Obj = spScene->loadMesh("TestScene.spm");
    Obj->setScale(2);
    
    #else
    
    scene::Mesh* Obj = spScene->createMesh(scene::MESH_CUBE);
    
    #endif
    
    SetupShading(Obj, true, 0.35f);
    
    f32 Rot = 0.0f;
    for (s32 i = -5; i <= 5; ++i)
    {
        CreateBox(dim::vector3df(1.5f*i, -1.5f, 0), Rot);
        Rot += 9.0f;
    }
    
    // Setup lighting
    scene::Light* Lit = spScene->getLightList().front();
    
    Lit->setLightModel(scene::LIGHT_POINT);
    Lit->setPosition(dim::vector3df(3.0f, 1.0f, 0.0f));
    Lit->setVolumetric(true);
    Lit->setVolumetricRadius(50.0f);
    
    #define MULTI_SPOT_LIGHT
    #ifdef MULTI_SPOT_LIGHT
    
    Lit->setVisible(false);
    
    const u32 LightCount = 15;
    
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
            false
        );
        MultiLights[i]->setRotation(math::Randomizer::randVector() * 360.0f);
        //MultiDirs[i] = math::Randomizer::randVector();
    }
    
    #else
    scene::Light* SpotLit = CreateSpotLight(dim::vector3df(-3, 0, 0), video::color(255, 32, 32));
    #endif
    
    // Create font
    video::Font* Fnt = spRenderer->createFont("Arial", 20, video::FONT_BOLD);
    
    f64 MinFPS = 999999.0, MaxFPS = 0.0, AvgFPS = 0.0;
    u32 Samples = 0;
    
    tool::CommandLineUI* Cmd = new tool::CommandLineUI();
    bool isCmdActive = false;
    Cmd->setBackgroundColor(video::color(0, 0, 0, 128));
    spControl->setWordInput(isCmdActive);
    
    // Main loop
    while (spDevice->updateEvents() && !spControl->keyDown(io::KEY_ESCAPE))
    {
        spRenderer->clearBuffers();
        
        // Update scene
        #ifdef MULTI_SPOT_LIGHT
        
        for (u32 i = 0; i < MultiLights.size(); ++i)
            MultiLights[i]->turn(dim::vector3df(0.5f, 0, 0) * io::Timer::getGlobalSpeed());
        
        #else
        
        if (spControl->keyDown(io::KEY_PAGEUP))
            SpotLit->turn(dim::vector3df(0, 1, 0));
        if (spControl->keyDown(io::KEY_PAGEDOWN))
            SpotLit->turn(dim::vector3df(0, -1, 0));
        
        if (spControl->keyDown(io::KEY_INSERT))
            SpotLit->turn(dim::vector3df(1, 0, 0));
        if (spControl->keyDown(io::KEY_DELETE))
            SpotLit->turn(dim::vector3df(-1, 0, 0));
        
        #endif
        
        #ifdef SCENE_WORLD
        if (spContext->isWindowActive() && !isCmdActive)
            tool::Toolset::moveCameraFree(0, spControl->keyDown(io::KEY_SHIFT) ? 0.25f : 0.125f);
        #else
        //tool::Toolset::presentModel(Obj);
        #endif
        
        #if 1
        if (!isCmdActive)
        {
            s32 w = spControl->getMouseWheel();
            if (w)
            {
                static f32 g = 0.6f;
                g += static_cast<f32>(w) * 0.1f;
                DefRenderer->changeBloomFactor(g);
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
        
        spRenderer->beginDrawing2D();
        {
            spRenderer->draw2DText(Fnt, dim::point2di(15, 15), "FPS: " + io::stringc(FPS));
            spRenderer->draw2DText(Fnt, dim::point2di(15, 40), "Min: " + io::stringc(MinFPS));
            spRenderer->draw2DText(Fnt, dim::point2di(15, 65), "Max: " + io::stringc(MaxFPS));
            spRenderer->draw2DText(Fnt, dim::point2di(15, 90), "Avg: " + io::stringc(AvgFPS / Samples));
        }
        spRenderer->endDrawing2D();
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
