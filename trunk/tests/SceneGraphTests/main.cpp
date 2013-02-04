//
// SoftPixel Engine - SceneGraph Tests
//

#include <SoftPixelEngine.hpp>

using namespace sp;

#include "../common.hpp"

SP_TESTS_DECLARE

/*

Notes:
==============
(DEBUG)
- Scene nodes using std::list container -> max. 49 FPS
- Scene nodes using std::vector container -> max. 76 FPS
- Scene nodes using std::vector container and mesh-buffer binding optimization -> max. 100 FPS
- Scene nodes using std::vector container, mesh-buffer binding optimization and without sorting -> max. 140 FPS

(RELEASE)
- Scene nodes using std::vector container -> max. 252 FPS
- Scene nodes using std::vector container and mesh-buffer binding optimization -> max. 500 FPS
- Scene nodes using std::vector container, mesh-buffer binding optimization and without sorting -> max. 528 FPS

*/

int main()
{
    SP_TESTS_INIT("SceneGraph")
    
    spContext->setVsync(false);
    
    math::Randomizer::seedRandom();
    
    Cam->setPosition(dim::vector3df(0, 0, -15));
    scene::Mesh* RefModel = spSceneMngr->createMesh(scene::MESH_CUBE);
    
    //RefModel->getBoundingVolume().setType(scene::BOUNDING_BOX);
    //RefModel->getBoundingVolume().setBox(RefModel->getMeshBoundingBox());
    
    spRenderer->setClearColor(video::color(255));
    
    //spScene->setDepthSorting(false);
    
    const s32 c = 5;
    
    for (s32 x = -c; x <= c; ++x)
    {
        for (s32 y = -c; y <= c; ++y)
        {
            for (s32 z = -c; z <= c; ++z)
            {
                scene::Mesh* Obj = spScene->createMesh();
                Obj->setReference(RefModel);
                
                Obj->getMaterial()->setColorMaterial(false);
                Obj->getMaterial()->setDiffuseColor(math::Randomizer::randColor());
                Obj->getMaterial()->setDiffuseAlpha(0.5f);
                
                Obj->setPosition(dim::vector3df(2.0f*x, 2.0f*y, 2.0f*z));
            }
        }
    }
    
    SP_TESTS_MAIN_BEGIN
    {
        if (spContext->isWindowActive())
            tool::Toolset::moveCameraFree(0, 0.25, 0.25f, 90.0f, false);
        
        spScene->renderScene();
        
        DrawFPS(15, 0);
        
        Draw2DText(dim::point2di(15, 50), "Draw Calls: " + io::stringc(video::RenderSystem::queryDrawCalls()), 0);
        Draw2DText(dim::point2di(15, 70), "Mesh Buffer Bindings: " + io::stringc(video::RenderSystem::queryMeshBufferBindings()), 0);
    }
    SP_TESTS_MAIN_END
}
