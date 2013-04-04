//
// SoftPixel Engine - Billboarding Tests
//

#include <SoftPixelEngine.hpp>

using namespace sp;

#include "../common.hpp"

SP_TESTS_DECLARE

#define USE_SHADER
//#define STORE_ANIM_POINTS
#define DRAW_LOGO

#ifdef USE_SHADER

struct SAnimVecDesc
{
    SAnimVecDesc() :
        Radius  (0.0f),
        Speed   (0.0f),
        Angle   (0.0f)
    {
    }
    ~SAnimVecDesc()
    {
    }
    
    /* Members */
    f32 Radius;
    f32 Speed;
    f32 Angle;
};

static const u32 ANIM_VECTOR_COUNT = 10;

dim::vector3df AnimVectors[ANIM_VECTOR_COUNT];
SAnimVecDesc AnimVectorsDesc[ANIM_VECTOR_COUNT];

void ShaderCallback(video::ShaderClass* ShdClass, const scene::MaterialNode* Object)
{
    video::Shader* VertShd = ShdClass->getVertexShader();
    
    VertShd->setConstant("ViewMatrix", spRenderer->getViewMatrix());
    VertShd->setConstant("ViewPos", Cam->getPosition(true));
    VertShd->setConstant("AnimVectors", &AnimVectors[0].X, ANIM_VECTOR_COUNT * 3);
}

#endif

int main()
{
    const dim::size2di ScrSize(
        //1920, 1080
        //1280, 768
        1024, 600
    );
    const bool Fullscreen = !true;
    
    SP_TESTS_INIT_EX2(
        video::RENDERER_OPENGL,
        ScrSize,
        "Billboarding",
        Fullscreen,
        SDeviceFlags(false, false)
    )
    
    const io::stringc MediaPath = ROOT_PATH + "Media/";
    const io::stringc RootPath = ROOT_PATH + "BillboardingTests/";
    
    #ifdef DRAW_LOGO
    video::Texture* LogoTex = spRenderer->loadTexture(MediaPath + "SoftPixelEngine Logo Small.png");
    #endif
    
    // Create position map
    const dim::size2di PMapSize(256);
    const s32 BlBrdCount = PMapSize.getArea();
    
    video::STextureCreationFlags CreationFlags;
    {
        CreationFlags.Dimension     = video::TEXTURE_2D;
        CreationFlags.MipMaps       = false;
        CreationFlags.MinFilter     = video::FILTER_LINEAR;
        CreationFlags.MagFilter     = video::FILTER_LINEAR;
        CreationFlags.BufferType    = video::IMAGEBUFFER_FLOAT;
        CreationFlags.Format        = video::PIXELFORMAT_RGBA;
        CreationFlags.HWFormat      = video::HWTEXFORMAT_FLOAT32;
        CreationFlags.Size          = PMapSize;
    }
    video::Texture* PositionMap = spRenderer->createTexture(CreationFlags);
    
    // Create billboard
    scene::Mesh* BlBrd = spScene->createMesh(scene::MESH_PLANE);
    
    BlBrd->meshTurn(dim::vector3df(-90, 0, 0));
    BlBrd->meshTranslate(dim::vector3df(0, 0.5f, 0));
    
    BlBrd->setScale(2);
    BlBrd->setOrder(-1);
    BlBrd->setPosition(dim::vector3df(0, -2, 0));
    BlBrd->addTexture(spRenderer->loadTexture(MediaPath + "Leaves1.png"));
    
    #ifdef USE_SHADER
    BlBrd->addTexture(PositionMap);
    #endif
    
    BlBrd->getMaterial()->setLighting(false);
    BlBrd->getMaterial()->setAlphaMethod(video::CMPSIZE_GREATER, 0.5f);
    BlBrd->getMaterial()->setBlending(false);
    
    // Load shader
    #ifdef USE_SHADER
    
    math::Randomizer::seedRandom();
    
    video::ShaderClass* ShdClass = spRenderer->createShaderClass();
    
    spRenderer->loadShader(ShdClass, video::SHADER_VERTEX, video::GLSL_VERSION_1_20, RootPath + "Billboarding.glvert");
    spRenderer->loadShader(ShdClass, video::SHADER_PIXEL, video::GLSL_VERSION_1_20, RootPath + "Billboarding.glfrag");
    
    if (ShdClass->link())
    {
        ShdClass->setObjectCallback(ShaderCallback);
        
        BlBrd->setShaderClass(ShdClass);
        BlBrd->getMeshBuffer(0)->setHardwareInstancing(BlBrdCount);
        
        ShdClass->getVertexShader()->setConstant("TexWidth", PMapSize.Width);
        ShdClass->getVertexShader()->setConstant("TexHeight", PMapSize.Height);
        
        ShdClass->getPixelShader()->setConstant("ColorMap", 0);
        ShdClass->getPixelShader()->setConstant("PositionMap", 1);
    }
    
    #endif
    
    // Create height field
    video::Texture* HeightMap = spRenderer->loadTexture(MediaPath + "HeightMap.jpg");
    
    scene::Mesh* HeightField = spScene->createHeightField(HeightMap, 150);
    
    HeightField->setPosition(dim::vector3df(0, -2, 0));
    HeightField->setScale(dim::vector3df(100, 8, 100));
    HeightField->addTexture(spRenderer->loadTexture(MediaPath + "Grass1.jpg"));
    HeightField->getMeshBuffer(0)->textureTransform(0, 15.0f);
    
    Cam->setPosition(dim::vector3df(0, 0, -5));
    
    // Generate random positions
    video::ImageBuffer* PMapImgBuffer = PositionMap->getImageBuffer();
    video::ImageBuffer* HMapImgBuffer = HeightMap->getImageBuffer();
    
    const dim::vector3df FieldSize(
        HeightField->getScale() * dim::vector3df(0.5f, 1.0f, 0.5f)
    );
    dim::vector4df Pos;
    
    for (s32 y = 0; y < PMapSize.Height; ++y)
    {
        for (s32 x = 0; x < PMapSize.Width; ++x)
        {
            Pos.X = math::Randomizer::randFloat();
            Pos.Z = math::Randomizer::randFloat();
            
            const u8 HeightFactor = HMapImgBuffer->getPixelColor(
                HMapImgBuffer->getPixelCoord(dim::point2df(Pos.X, 1.0f - Pos.Z))
            ).Red;
            
            Pos.X = -FieldSize.X + Pos.X * (FieldSize.X * 2.0f);
            Pos.Y = -2.0f + FieldSize.Y * (static_cast<f32>(HeightFactor) / 255.0f);
            Pos.Z = -FieldSize.Z + Pos.Z * (FieldSize.Z * 2.0f);
            Pos.W = math::Randomizer::randFloat(0.35f, 2.0f);
            
            PMapImgBuffer->setPixelVector(dim::point2di(x, y), Pos);
        }
    }
    
    PositionMap->updateImageBuffer();
    
    // Create skybox
    spRenderer->setTextureGenFlags(video::TEXGEN_WRAP, video::TEXWRAP_CLAMP);
    
    video::Texture* SBoxTexList[6] =
    {
        spRenderer->loadTexture(MediaPath + "SkyboxNorth.jpg"),
        spRenderer->loadTexture(MediaPath + "SkyboxSouth.jpg"),
        spRenderer->loadTexture(MediaPath + "SkyboxTop.jpg"),
        spRenderer->loadTexture(MediaPath + "SkyboxBottom.jpg"),
        spRenderer->loadTexture(MediaPath + "SkyboxWest.jpg"),
        spRenderer->loadTexture(MediaPath + "SkyboxEast.jpg"),
    };
    
    spRenderer->setTextureGenFlags(video::TEXGEN_WRAP, video::TEXWRAP_REPEAT);
    
    scene::Mesh* SkyBox = spScene->createSkyBox(SBoxTexList);
    
    // Create font and timer for FPS display
    video::Font* Fnt = spRenderer->createFont("Arial", 20, video::FONT_BOLD);
    io::Timer FPSTimer;
    
    // Initialize anim vector descriptors
    for (u32 i = 0; i < ANIM_VECTOR_COUNT; ++i)
    {
        AnimVectorsDesc[i].Radius = math::Randomizer::randFloat(0.05f, 0.15f);
        AnimVectorsDesc[i].Speed = math::Randomizer::randFloat(0.1f, 1.0f);
        AnimVectorsDesc[i].Angle = math::Randomizer::randFloat(0.0f, 360.0f);
    }
    
    #ifdef STORE_ANIM_POINTS
    
    io::FileSystem FileSys;
    io::File* AnimPointFile = FileSys.openFile("AnimPoints.h", io::FILE_WRITE);
    
    #else
    
    scene::NodeAnimation* CamAnim = spSceneMngr->createAnimation<scene::NodeAnimation>("Camera Animation");
    Cam->addAnimation(CamAnim);
    
    #include "AnimPoints.h"
    
    CamAnim->setSplineTranslation(true);
    CamAnim->setSpeed(0.25f);
    CamAnim->play(scene::PLAYBACK_LOOP);
    
    #endif
    
    bool FreeLookMode = false;
    bool FreeWalkMode = false;
    
    while (spDevice->updateEvents() && !spControl->keyDown(io::KEY_ESCAPE))
    {
        spRenderer->clearBuffers();
        
        // Billboarding
        #ifndef USE_SHADER
        
        dim::vector3df XAxis, YAxis, ZAxis;
        
        YAxis = dim::vector3df(0, 1, 0);
        YAxis.normalize();
        
        ZAxis = BlBrd->getPosition(true) - Cam->getPosition(true);
        ZAxis.normalize();
        
        XAxis = YAxis.cross(ZAxis);
        XAxis.normalize();
        
        ZAxis = XAxis.cross(YAxis);
        ZAxis.normalize();
        
        // Final billboard matrix
        dim::matrix4f Mat;
        
        Mat[0] = XAxis.X; Mat[4] = YAxis.X; Mat[ 8] = ZAxis.X;
        Mat[1] = XAxis.Y; Mat[5] = YAxis.Y; Mat[ 9] = ZAxis.Y;
        Mat[2] = XAxis.Z; Mat[6] = YAxis.Z; Mat[10] = ZAxis.Z;
        
        BlBrd->setRotationMatrix(Mat);
        
        #endif
        
        // Input and rendering
        const f32 GameSpeed = io::Timer::getGlobalSpeed();
        
        if (spContext->isWindowActive() && FreeWalkMode)
            tool::Toolset::moveCameraFree(Cam, 0.1f * GameSpeed);
        
        if (spControl->keyHit(io::KEY_TAB))
        {
            static bool Wire;
            Wire = !Wire;
            spScene->setWireframe(Wire ? video::WIREFRAME_LINES : video::WIREFRAME_SOLID);
        }
        
        if (spControl->keyHit(io::KEY_SPACE))
            FreeLookMode = !FreeLookMode;
        
        if (spControl->keyHit(io::KEY_RETURN))
        {
            FreeWalkMode = !FreeWalkMode;
            #ifndef STORE_ANIM_POINTS
            if (CamAnim)
                CamAnim->pause(FreeWalkMode);
            #endif
        }
        
        #ifdef STORE_ANIM_POINTS
        
        if (spControl->mouseHit(io::MOUSE_LEFT))
        {
            io::Log::message("Point Stored");
            
            const dim::vector3df Pos(Cam->getPosition(true));
            const dim::quaternion Rot(dim::quaternion(Cam->getRotationMatrix(true)));
            
            AnimPointFile->writeStringN("CamAnim->addKeyframe(scene::Transformation(");
            AnimPointFile->writeStringN(
                "    dim::vector3df(" + io::stringc(Pos.X) + "f, " + io::stringc(Pos.Y) + "f, " + io::stringc(Pos.Z) + "f),"
            );
            AnimPointFile->writeStringN(
                "    dim::quaternion(" + io::stringc(Rot.X) + "f, " + io::stringc(Rot.Y) + "f, " + io::stringc(Rot.Z) + "f, " + io::stringc(Rot.W) + "f)"
            );
            AnimPointFile->writeStringN("));");
        }
        
        #endif
        
        #ifndef STORE_ANIM_POINTS
        
        if (CamAnim && !FreeWalkMode)
            CamAnim->updateAnimation(Cam);
        
        #endif
        
        // Update camera position and skybox
        dim::vector3df CamPos(Cam->getPosition(true));
        
        if (!FreeLookMode && FreeWalkMode)
        {
            const dim::point2df CamPosOnField(
                0.5f + (CamPos.X / (FieldSize.X * 2.0f)),
                0.5f - (CamPos.Z / (FieldSize.Z * 2.0f))
            );
            
            CamPos.Y = FieldSize.Y * HMapImgBuffer->getInterpolatedPixel(CamPosOnField).X;
            
            const f32 FieldWidth = FieldSize.X - 7.0f;
            const f32 FieldDepth = FieldSize.Z - 7.0f;
            
            math::Clamp(CamPos.X, -FieldWidth, FieldWidth);
            math::Clamp(CamPos.Z, -FieldDepth, FieldDepth);
            
            Cam->setPosition(CamPos);
        }
        
        SkyBox->setPosition(CamPos);
        
        // Update anim offset vectors
        for (u32 i = 0; i < ANIM_VECTOR_COUNT; ++i)
        {
            AnimVectorsDesc[i].Angle += AnimVectorsDesc[i].Speed * GameSpeed;
            
            if (AnimVectorsDesc[i].Angle > 360.0f)
                AnimVectorsDesc[i].Angle -= 360.0f;
            
            f32 Angle = AnimVectorsDesc[i].Angle;
            f32 Radius = AnimVectorsDesc[i].Radius;
            
            AnimVectors[i].X = math::Sin(Angle) * Radius;
            AnimVectors[i].Z = math::Cos(Angle) * Radius;
        }
        
        // Render the scene
        spScene->renderScene();
        
        spRenderer->beginDrawing2D();
        {
            // Draw FPS and triangle count information
            spRenderer->draw2DText(
                Fnt, dim::point2di(15, 15), io::stringc(2 * BlBrdCount + 2*6 + 2) + " Triangles Rendered (" + io::stringc(BlBrdCount) + " Billboard Instances)"
            );
            spRenderer->draw2DText(
                Fnt, dim::point2di(15, 45), spRenderer->getRenderer() + ": " + spRenderer->getVendor()
            );
            spRenderer->draw2DText(
                Fnt, dim::point2di(15, 75), "FPS: " + io::stringc(FPSTimer.getFPS())
            );
            
            #ifdef DRAW_LOGO
            spRenderer->draw2DImage(
                LogoTex, dim::point2di(ScrSize.Width - LogoTex->getSize().Width - 25, 25)
            );
            #endif
        }
        spRenderer->endDrawing2D();
        
        spContext->flipBuffers();
    }
    
    deleteDevice();
    
    return 0;
}
