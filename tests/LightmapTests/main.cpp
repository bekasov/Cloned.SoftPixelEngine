//
// SoftPixel Engine - Lightmap Tests
//

#include <SoftPixelEngine.hpp>

using namespace sp;

#ifdef SP_COMPILE_WITH_LIGHTMAPGENERATOR

#include "../common.hpp"

SP_TESTS_DECLARE

static scene::Light* CreateLightSource(const dim::vector3df &Point, const video::color &Color = 255, f32 Radius = 1500.0f)
{
    // Create light source
    scene::Light* LightObj = spScene->createLight(scene::LIGHT_POINT);
    
    LightObj->setVolumetric(true);
    LightObj->setVolumetricRadius(Radius);
    LightObj->setLightingColor(Color);
    LightObj->setPosition(Point);
    
    // Create visual light model
    scene::Mesh* VisualLightModel = spScene->createMesh(scene::MESH_SPHERE);
    
    video::MaterialStates* Material = VisualLightModel->getMaterial();
    
    Material->setColorMaterial(false);
    Material->setDiffuseColor(0);
    Material->setAmbientColor(Color);
    
    VisualLightModel->setParent(LightObj);
    VisualLightModel->setScale(0.35f);
    
    return LightObj;
}

u64 ElapsedTime = 0;

/**
=== Timging results: ===

DEBUG MODE / With 2 light sources:
 -> Single threaded: ~2800 ms.
 -> Multi threaded (8 Threads): ~1900 ms.

RELEASE MODE / With 2 light sources:
 -> Single threaded: ~1080 ms.
 -> Multi threaded (8 Threads): 780 ms.


=== kd-Tree optimization results: ===

Without polygon clipping: 2543 ms. incl. 141 ms. tree generation
With    polygon clipping: 2293 ms. incl. 405 ms. tree generation

*/

static bool ProgressCallback(f32 Progress)
{
    return true;
}

static void StateCallback(const tool::ELightmapGenerationStates State, const io::stringc &Info)
{
    const io::stringc StateStr("State: " + tool::Debugging::toString(State));
    
    u64 CurrentTime = io::Timer::millisecs();
    const io::stringc TimeStr(" [ " + io::stringc(CurrentTime - ElapsedTime) + " elapsed ms. ]");
    ElapsedTime = CurrentTime;
    
    io::Log::message(
        (Info.empty() ? StateStr : StateStr + " ( " + Info + " )") + TimeStr
    );
}

int main()
{
    SP_TESTS_INIT_EX2(
        video::RENDERER_OPENGL, dim::size2di(800, 600), "Lightmap", false, SDeviceFlags()
    )
    
    spRenderer->setClearColor(video::color(255));
    
    // Setup scene
    Lit->setLightModel(scene::LIGHT_POINT);
    
    const io::stringc ResPath("../DeferredRendererTests/");
    
    scene::Mesh* World = spScene->loadMesh(ResPath + "TestScene.spm");
    
    //World->addTexture(spRenderer->loadTexture("../../help/tutorials/ShaderLibrary/media/StoneColorMap.jpg"));
    //World->textureAutoMap(0);
    
    math::Randomizer::seedRandom();
    
    const video::color AmbColors[] =
    {
        video::color(20),
        video::color(50),
        video::color(50, 0, 0),
        video::color(0, 50, 0),
        video::color(0, 0, 50),
        video::color(50, 50, 0)
    };
    
    // Lightmap generation
    //#define TEST_BARYCENTRIC_COORDS
    
    #ifndef TEST_BARYCENTRIC_COORDS
    
    std::list<tool::SCastShadowObject> CastObjList;
    std::list<tool::SGetShadowObject> GetObjList;
    std::list<tool::SLightmapLight> LitSources;
    
    CastObjList.push_back(World);
    GetObjList.push_back(World);
    
    #   if 1
    LitSources.push_back(CreateLightSource(0.0f, video::color(0, 0, 255), 150.0f));
    LitSources.push_back(CreateLightSource(dim::vector3df(2, -0.5f, -1), video::color(255, 0, 0), 150.0f));
    LitSources.push_back(CreateLightSource(dim::vector3df(-2, -0.5f, -1), video::color(0, 255, 0), 150.0f));
    #   else
    LitSources.push_back(CreateLightSource(0.0f));
    #   endif
    
    #   define RAW_LIGHTMAPS
    #   ifdef RAW_LIGHTMAPS
    spRenderer->setTextureGenFlags(video::TEXGEN_FILTER, video::FILTER_LINEAR);
    #   endif
    
    #ifdef RAW_LIGHTMAPS
    s32 BlurFactor = 0;
    #else
    s32 BlurFactor = tool::DEF_LIGHTMAP_BLURRADIUS;
    #endif
    
    u64 t = io::Timer::millisecs();
    ElapsedTime = t;
    
    tool::LightmapGenerator* LightmapPlotter = new tool::LightmapGenerator();
    
    LightmapPlotter->setProgressCallback(ProgressCallback);
    LightmapPlotter->setStateCallback(StateCallback);
    
    LightmapPlotter->generateLightmaps(
        CastObjList,
        GetObjList,
        LitSources,
        tool::DEF_LIGHTMAP_AMBIENT,
        256,//tool::DEF_LIGHTMAP_SIZE,
        tool::DEF_LIGHTMAP_DENSITY,
        BlurFactor,
        8,
        tool::LIGHTMAPFLAG_NOTRANSPARENCY
    );
    
    io::Log::message("Duration: " + io::stringc(io::Timer::millisecs() - t) + " ms.");
    
    //World->setVisible(false);
    
    #else
    
    video::Texture* Tex = spRenderer->loadTexture("TestImage.png");
    Tex->setFilter(video::FILTER_LINEAR);
    
    spRenderer->setClearColor(100);
    
    #endif
    
    // Command line
    tool::CommandLineUI* cmd = new tool::CommandLineUI();
    cmd->setBackgroundColor(video::color(0, 0, 0, 128));
    cmd->setRect(dim::rect2di(0, 0, spContext->getResolution().Width, spContext->getResolution().Height));
    
    bool isCmdActive = false;
    spControl->setWordInput(isCmdActive);
    
    // Main loop
    while (spDevice->updateEvents() && !spControl->keyDown(io::KEY_ESCAPE))
    {
        spRenderer->clearBuffers();
        
        #ifndef TEST_BARYCENTRIC_COORDS
        
        if (!isCmdActive && spContext->isWindowActive())
            tool::Toolset::moveCameraFree(0, 0.25, 0.25f, 90.0f, false);
        
        if (spControl->mouseHit(io::MOUSE_RIGHT))
            Cam->setPosition(0.0f);
        
        spScene->renderScene();
        
        //Draw2DText(dim::point2di(15, 15), "...", 0);
        
        if (!isCmdActive)
        {
            if (spControl->keyHit(io::KEY_PAGEUP))
            {
                if (++BlurFactor > 5)
                    BlurFactor = 5;
                if (LightmapPlotter->updateBluring(static_cast<u32>(BlurFactor)))
                    io::Log::message("Updated Bluring (Radius = " +  io::stringc(BlurFactor) +")");
            }
            if (spControl->keyHit(io::KEY_PAGEDOWN))
            {
                if (--BlurFactor < 0)
                    BlurFactor = 0;
                if (LightmapPlotter->updateBluring(static_cast<u32>(BlurFactor)))
                    io::Log::message("Updated Bluring (Radius = " +  io::stringc(BlurFactor) +")");
            }
            
            if (spControl->keyHit(io::KEY_RETURN))
            {
                static s32 ColorIndex;
                if (++ColorIndex > 5)
                    ColorIndex = 0;
                
                const video::color Color(AmbColors[ColorIndex]);
                
                if (LightmapPlotter->updateAmbientColor(Color))
                    io::Log::message("Updated Ambient Color " + tool::Debugging::toString(Color));
            }
        }
        
        #else
        
        spRenderer->beginDrawing2D();
        
        const dim::point2df MousePos(spControl->getCursorPosition().cast<f32>());
        const dim::size2df ScrSize(spContext->getResolution().cast<f32>());
        const dim::size2df TexSize(Tex->getSize().cast<f32>());
        
        const dim::triangle3d<f32, dim::point2df> Tri(
            dim::point2df(50, 150),
            dim::point2df(650, 50),
            dim::point2df(250, 650)
        );
        const dim::triangle3df Map(
            dim::vector3df(0.0f, 0.0f, 0.0f),
            dim::vector3df(1.0f, 0.0f, 0.0f),
            dim::vector3df(0.0f, 1.0f, 0.0f)
        );
        
        f32 x = math::MinMax(MousePos.X/ScrSize.Width, 0.0f, 1.0f);
        f32 y = math::MinMax(MousePos.Y/ScrSize.Height, 0.0f, 1.0f);
        
        x *= TexSize.Width;
        y *= TexSize.Height;
        
        x = floor(x);
        y = floor(y);
        
        x += 0.5f;
        y += 0.5f;
        
        x /= TexSize.Width;
        y /= TexSize.Height;
        
        //dim::vector3df Coord(Map.getBarycentricCoord(dim::point2df(x, y)));
        //Coord /= (Coord.X + Coord.Y + Coord.Z);
        dim::vector3df Coord(math::getBarycentricCoord(Map, dim::vector3df(x, y, 0.0f)));
        
        const dim::point2df Point(Tri.getBarycentricPoint(Coord));
        
        scene::SPrimitiveVertex2D Verts[3] =
        {
            scene::SPrimitiveVertex2D(Tri.PointA.X, Tri.PointA.Y, Map.PointA.X, Map.PointA.Y),
            scene::SPrimitiveVertex2D(Tri.PointB.X, Tri.PointB.Y, Map.PointB.X, Map.PointB.Y),
            scene::SPrimitiveVertex2D(Tri.PointC.X, Tri.PointC.Y, Map.PointC.X, Map.PointC.Y)
        };
        
        spRenderer->draw2DPolygonImage(video::PRIMITIVE_TRIANGLES, Tex, Verts, 3);
        
        const video::color PointColor(255, 0, 0);
        const dim::point2di RealPoint(Point.cast<s32>());
        
        #   if 0
        spRenderer->draw2DBox(RealPoint, 15, PointColor);
        #   else
        spRenderer->draw2DLine(dim::point2di(RealPoint.X - 20, RealPoint.Y), dim::point2di(RealPoint.X + 20, RealPoint.Y), PointColor);
        spRenderer->draw2DLine(dim::point2di(RealPoint.X, RealPoint.Y - 20), dim::point2di(RealPoint.X, RealPoint.Y + 20), PointColor);
        #   endif
        
        spRenderer->endDrawing2D();
        
        Draw2DText(
            dim::point2di(15, 15),
            "Mouse: X = " + io::stringc(x) + ", Y = " + io::stringc(y)
        );
        Draw2DText(
            dim::point2di(15, 30),
            "Barycentric Coordinate: X = " + io::stringc(Coord.X) + ", Y = " + io::stringc(Coord.Y) + ", Z = " + io::stringc(Coord.Z)
        );
        
        #endif
        
        if (isCmdActive)
            cmd->render();
        
        if (spControl->keyHit(io::KEY_F3))
        {
            isCmdActive = !isCmdActive;
            spControl->setWordInput(isCmdActive);
        }
        
        spContext->flipBuffers();
    }
    
    delete LightmapPlotter;
    delete cmd;
    
    deleteDevice();
    
    return 0;
}

#else

int main()
{
    io::Log::error("Engine was not compiled with \"LightmapGenerator\"");
    io::Log::pauseConsole();
    return 0;
}

#endif
