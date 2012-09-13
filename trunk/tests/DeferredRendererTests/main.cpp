//
// SoftPixel Engine - DeferredRenderer Tests
//

#include <SoftPixelEngine.hpp>

using namespace sp;

#ifdef SP_COMPILE_WITH_DEFERREDRENDERER

#include "../common.hpp"

SP_TESTS_DECLARE

int main()
{
    SP_TESTS_INIT("DeferredRenderer")
    
    // Create deferred renderer
    video::DeferredRenderer* DefRenderer = new video::DeferredRenderer();
    
    DefRenderer->generateResources(
        video::DEFERREDFLAG_NORMAL_MAPPING
        | video::DEFERREDFLAG_PARALLAX_MAPPING
        //| video::DEFERREDFLAG_DEBUG_GBUFFER
    );
    
    // Load textures
    const io::stringc Path = "../../help/tutorials/ShaderLibrary/media/";
    
    video::Texture* DiffuseMap  = spRenderer->loadTexture(Path + "StoneColorMap.jpg");
    video::Texture* NormalMap   = spRenderer->loadTexture(Path + "StoneNormalMap.jpg");
    video::Texture* HeightMap   = spRenderer->loadTexture("StonesHeightMap.jpg");
    
    // Create scene
    scene::SceneGraph::setDefaultVertexFormat(DefRenderer->getVertexFormat());
    
    scene::Mesh* Obj = spScene->createMesh(scene::MESH_CUBE);
    
    Obj->addTexture(DiffuseMap);
    Obj->addTexture(NormalMap);
    Obj->addTexture(HeightMap);
    
    Obj->updateTangentSpace(1, 2);
    
    Cam->setPosition(dim::vector3df(0, 0, -1.5f));
    
    spScene->getLightList().front()->setLightModel(scene::LIGHT_POINT);
    
    Obj->setShaderClass(DefRenderer->getGBufferShader());
    Obj->getMaterial()->setBlending(false);
    
    // Main loop
    while (spDevice->updateEvent() && !spControl->keyDown(io::KEY_ESCAPE))
    {
        spRenderer->clearBuffers();
        
        #if 1
        DefRenderer->renderScene(spScene);
        #else
        spScene->renderScene();
        #endif
        
        tool::Toolset::presentModel(Obj);
        
        spContext->flipBuffers();
    }
    
    delete DefRenderer;
}

#else

int main()
{
    io::Log::error("This engine was not compiled with deferred renderer");
    return 0;
}

#endif
