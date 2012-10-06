//
// SoftPixel Engine - DeferredRenderer Tests
//

#include <SoftPixelEngine.hpp>
#include <RenderSystem/DeferredRenderer/spShadowMapper.hpp>

using namespace sp;

#ifdef SP_COMPILE_WITH_DEFERREDRENDERER

#include "../common.hpp"

SP_TESTS_DECLARE

int main()
{
    SP_TESTS_INIT_EX(
        video::RENDERER_OPENGL,
        dim::size2di(1024, 768),//video::VideoModeEnumerator().getDesktop().Resolution,
        "DeferredRenderer",
        false
    )
    
    // Create deferred renderer
    video::DeferredRenderer* DefRenderer = new video::DeferredRenderer();
    
    DefRenderer->generateResources(
        video::DEFERREDFLAG_NORMAL_MAPPING
        | video::DEFERREDFLAG_PARALLAX_MAPPING
        | video::DEFERREDFLAG_BLOOM
        | video::DEFERREDFLAG_SHADOW_MAPPING
        //| video::DEFERREDFLAG_DEBUG_GBUFFER
    );
    
    // Load textures
    const io::stringc Path = "../../help/tutorials/ShaderLibrary/media/";
    
    spRenderer->setTextureGenFlags(video::TEXGEN_MIPMAPFILTER, video::FILTER_ANISOTROPIC);
    spRenderer->setTextureGenFlags(video::TEXGEN_ANISOTROPY, 8);
    
    video::Texture* DiffuseMap  = spRenderer->loadTexture(Path + "StoneColorMap.jpg");
    video::Texture* NormalMap   = spRenderer->loadTexture(Path + "StoneNormalMap.jpg");
    video::Texture* HeightMap   = spRenderer->loadTexture("StonesHeightMap.jpg");
    
    // Create scene
    Cam->setPosition(dim::vector3df(0, 0, -1.5f));
    
    scene::SceneGraph::setDefaultVertexFormat(DefRenderer->getVertexFormat());
    
    #define SCENE_WORLD
    #ifdef SCENE_WORLD
    
    scene::Mesh* Obj = spScene->loadMesh("TestScene.spm");
    
    Obj->textureAutoMap(0, 0.7f);
    Obj->setScale(2);
    
    #else
    
    scene::Mesh* Obj = spScene->createMesh(scene::MESH_CUBE);
    
    #endif
    
    Obj->addTexture(DiffuseMap);
    Obj->addTexture(NormalMap);
    Obj->addTexture(HeightMap);
    
    Obj->updateTangentSpace(1, 2);
    
    //Obj->setShaderClass(DefRenderer->getGBufferShader());
    Obj->getMaterial()->setBlending(false);
    
    // Setup lighting
    scene::Light* Lit = spScene->getLightList().front();
    
    Lit->setLightModel(scene::LIGHT_POINT);
    Lit->setPosition(dim::vector3df(3.0f, 1.0f, 0.0f));
    
    scene::Light* SpotLit = spScene->createLight(scene::LIGHT_SPOT);
    SpotLit->setSpotCone(15.0f, 30.0f);
    SpotLit->setDiffuseColor(video::color(255, 32, 32));
    SpotLit->setPosition(dim::vector3df(-3, 0, 0));
    SpotLit->setShadow(true);
    
    // Main loop
    while (spDevice->updateEvent() && !spControl->keyDown(io::KEY_ESCAPE))
    {
        spRenderer->clearBuffers();
        
        // Update scene
        if (spControl->keyDown(io::KEY_L))
            SpotLit->turn(dim::vector3df(0, 1, 0));
        
        #ifdef SCENE_WORLD
        if (spContext->isWindowActive())
            tool::Toolset::moveCameraFree();
        #else
        tool::Toolset::presentModel(Obj);
        #endif
        
        #if 1
        s32 w = spControl->getMouseWheel();
        if (w)
        {
            static f32 g = 0.6f;
            g += static_cast<f32>(w) * 0.1f;
            DefRenderer->changeBloomFactor(g);
        }
        #endif
        
        // Render scene
        #if 1
        DefRenderer->renderScene(spScene);
        #else
        spScene->renderScene();
        #endif
        
        spContext->flipBuffers();
    }
    
    delete DefRenderer;
    
    deleteDevice();
    
    return 0;
}

#else

int main()
{
    io::Log::error("This engine was not compiled with deferred renderer");
    return 0;
}

#endif
