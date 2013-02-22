//
// SoftPixel Engine - SceneGraph Tests
//

#include <SoftPixelEngine.hpp>

using namespace sp;

#include "../common.hpp"

#include <boost/foreach.hpp>

#include <windows.h>

SP_TESTS_DECLARE

//#define USE_PORTAL_SCENE

#ifdef USE_PORTAL_SCENE
scene::SceneGraphPortalBased* MainScene = 0;
#endif

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
            
            return 0;
        }
        
    private:
        
        /* === Functions === */
        
        bool CatchBoundVolume(const SpBoundVolume &Object)
        {
            if (!SceneLoaderSPSB::CatchBoundVolume(Object))
                return false;
            
            #ifdef USE_PORTAL_SCENE
            
            if (Object.BaseObject.Flags == "sector")
                MainScene->createSector(convertTransformation(Object.BaseObject));
            else if (Object.BaseObject.Flags == "portal")
                MainScene->createPortal(convertTransformation(Object.BaseObject));
            
            #endif
            
            return true;
        }
        
};

/**

DEBUG Mode:
 - Without texture binding optimization: ~33 FPS
 - With texture binding optimization: ??? FPS

*/

int main()
{
    SP_TESTS_INIT_EX2(
        //video::RENDERER_OPENGL,
        video::RENDERER_DIRECT3D11,
        dim::size2di(1024, 768), "SceneGraph", false,
        SDeviceFlags(false, !false)
    )
    
    SetCurrentDirectory("D:/SoftwareEntwicklung/C++/HLC/Tools/SoftPixelEngine/repository/tests/SceneGraphTests/");
    
    #ifdef USE_PORTAL_SCENE
    MainScene = spDevice->createSceneGraph<scene::SceneGraphPortalBased>();
    //MainScene->addSceneNode(Lit);
    #else
    scene::SceneGraph* MainScene = spScene;
    #endif
    
    const bool ShowPerformance = true;
    
    #if 0
    if (ShowPerformance)
        spContext->setVsync(false);
    #else
    spContext->setVsync(true);
    #endif
    
    math::Randomizer::seedRandom();
    
    spRenderer->setClearColor(video::color(200));
    
    //spScene->setDepthSorting(false);
    
    #if 1
    
    //Cam->setPosition(dim::vector3df(0, 0, -15));
    scene::Mesh* RefModel = spSceneMngr->createMesh(scene::MESH_CUBE);
    
    RefModel->getBoundingVolume().setType(scene::BOUNDING_BOX);
    RefModel->getBoundingVolume().setBox(RefModel->getMeshBoundingBox());
    RefModel->getBoundingVolume().setRadius(RefModel->getMeshBoundingSphere());
    
    #   if 1
    const io::stringc ResPath = "../Media/";
    RefModel->addTexture(spRenderer->loadTexture(ResPath + "SoftPixelEngine Logo Small.png"));
    #   endif
    
    const s32 c = 1;//5;
    
    for (s32 x = -c; x <= c; ++x)
    {
        for (s32 y = -c; y <= c; ++y)
        {
            for (s32 z = -c; z <= c; ++z)
            {
                scene::Mesh* Obj = MainScene->createMesh();
                
                #if 0
                
                Obj->setReference(RefModel);
                
                #else
                
                video::MeshBuffer* Surf = Obj->createMeshBuffer();
                Surf->setReference(RefModel->getMeshBuffer(0));
                if (math::Randomizer::randBool(1))
                    Surf->setTexturesReference(RefModel->getMeshBuffer(0));
                
                Obj->getBoundingVolume().setType(scene::BOUNDING_SPHERE);
                Obj->getBoundingVolume().setRadius(RefModel->getBoundingVolume().getRadius());
                
                #endif
                
                Obj->getMaterial()->setColorMaterial(false);
                Obj->getMaterial()->setDiffuseColor(math::Randomizer::randColor());
                Obj->getMaterial()->setAmbientColor(Obj->getMaterial()->getDiffuseColor() / 3);
                Obj->getMaterial()->setDiffuseAlpha(0.5f);
                
                Obj->setPosition(dim::vector3df(2.0f*x, 2.0f*y, 2.0f*z));
            }
        }
    }
    
    #else
    
    Lit->setVisible(false);
    
    CustomSceneLoader Loader;
    Loader.loadScene("PortalBasedScene.spsb", video::TEXPATH_IGNORE, scene::DEF_SCENE_FLAGS);
    
    #endif
    
    #ifdef USE_PORTAL_SCENE
    MainScene->connectSectors();
    MainScene->insertRenderNodes();
    #endif
    
    SP_TESTS_MAIN_BEGIN
    {
        if (spContext->isWindowActive())
            tool::Toolset::moveCameraFree(0, 0.25, 0.25f, 90.0f, false);
        
        MainScene->renderScene(Cam);
        
        if (ShowPerformance)
            DrawFPS(15, video::color(255, 0, 0));
        
        if (spControl->keyHit(io::KEY_TAB))
        {
            static bool Wire;
            Wire = !Wire;
            MainScene->setWireframe(Wire ? video::WIREFRAME_LINES : video::WIREFRAME_SOLID);
        }
        
        Draw2DText(dim::point2di(15, 50), "Draw Calls: " + io::stringc(video::RenderSystem::queryDrawCalls()), 0);
        Draw2DText(dim::point2di(15, 70), "Mesh Buffer Bindings: " + io::stringc(video::RenderSystem::queryMeshBufferBindings()), 0);
        Draw2DText(dim::point2di(15, 90), "Texture Layer Bindings: " + io::stringc(video::RenderSystem::queryTextureLayerBindings()), 0);
    }
    SP_TESTS_MAIN_END
}
