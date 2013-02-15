//
// SoftPixel Engine - SceneGraph Tests
//

#include <SoftPixelEngine.hpp>

using namespace sp;

#include "../common.hpp"

#include <boost/foreach.hpp>

SP_TESTS_DECLARE

scene::SceneGraphPortalBased* MainScene = 0;

class CustomSceneLoader : public scene::SceneLoaderSPSB
{
    
    public:
       
        CustomSceneLoader() :
            scene::SceneLoaderSPSB()
        {
        }
        ~CustomSceneLoader()
        {
        }
        
        /* === Functions === */
        
        scene::Mesh* loadScene(const io::stringc &Filename, const io::stringc &TexturePath, const s32 Flags)
        {
            io::Log::message("Load scene: \"" + Filename + "\"");
            io::Log::ScopedTab Tab;
            
            /* Load scene */
            SceneLoaderSPSB::loadScene(Filename, TexturePath, Flags);
            
            /* Connect portals (programmatically for testing) */
            foreach (scene::Sector* SectorObj, MainScene->getSectorList())
            {
                foreach (scene::Portal* PortalObj, MainScene->getPortalList())
                    SectorObj->addPortal(PortalObj);
            }
            
            return 0;
        }
        
    private:
        
        /* === Functions === */
        
        bool CatchBoundVolume(const SpBoundVolume &Object)
        {
            if (!SceneLoaderSPSB::CatchBoundVolume(Object))
                return false;
            
            #if 1
            
            if (Object.BaseObject.Flags == "sector")
                MainScene->createSector(convertTransformation(Object.BaseObject));
            else if (Object.BaseObject.Flags == "portal")// && MainScene->getPortalList().empty())
                MainScene->createPortal(convertTransformation(Object.BaseObject));
            
            #endif
            
            return true;
        }
        
};

int main()
{
    SP_TESTS_INIT("SceneGraph")
    
    MainScene = spDevice->createSceneGraph<scene::SceneGraphPortalBased>();
    MainScene->addSceneNode(Lit);
    
    const bool ShowPerformance = false;
    
    if (ShowPerformance)
        spContext->setVsync(false);
    
    math::Randomizer::seedRandom();
    
    //Cam->setPosition(dim::vector3df(0, 0, -15));
    scene::Mesh* RefModel = spSceneMngr->createMesh(scene::MESH_CUBE);
    
    RefModel->getBoundingVolume().setType(scene::BOUNDING_BOX);
    RefModel->getBoundingVolume().setBox(RefModel->getMeshBoundingBox());
    
    spRenderer->setClearColor(video::color(255));
    
    //spScene->setDepthSorting(false);
    
    #if 0
    
    const s32 c = 5;
    
    for (s32 x = -c; x <= c; ++x)
    {
        for (s32 y = -c; y <= c; ++y)
        {
            for (s32 z = -c; z <= c; ++z)
            {
                scene::Mesh* Obj = MainScene->createMesh();
                Obj->setReference(RefModel);
                
                Obj->getMaterial()->setColorMaterial(false);
                Obj->getMaterial()->setDiffuseColor(math::Randomizer::randColor());
                Obj->getMaterial()->setAmbientColor(Obj->getMaterial()->getDiffuseColor() / 3);
                Obj->getMaterial()->setDiffuseAlpha(0.5f);
                
                Obj->setPosition(dim::vector3df(2.0f*x, 2.0f*y, 2.0f*z));
            }
        }
    }
    
    #else
    
    CustomSceneLoader Loader;
    Loader.loadScene("PortalBasedScene.spsb", video::TEXPATH_IGNORE, scene::DEF_SCENE_FLAGS);
    
    #endif
    
    MainScene->insertRenderNodes();
    
    SP_TESTS_MAIN_BEGIN
    {
        if (spContext->isWindowActive())
            tool::Toolset::moveCameraFree(0, 0.25, 0.25f, 90.0f, false);
        
        MainScene->renderScene(Cam);
        
        if (ShowPerformance)
            DrawFPS(15, 0);
        
        if (spControl->keyHit(io::KEY_TAB))
        {
            static bool Wire;
            Wire = !Wire;
            MainScene->setWireframe(Wire ? video::WIREFRAME_LINES : video::WIREFRAME_SOLID);
        }
        
        Draw2DText(dim::point2di(15, 50), "Draw Calls: " + io::stringc(video::RenderSystem::queryDrawCalls()), 0);
        Draw2DText(dim::point2di(15, 70), "Mesh Buffer Bindings: " + io::stringc(video::RenderSystem::queryMeshBufferBindings()), 0);
    }
    SP_TESTS_MAIN_END
}
