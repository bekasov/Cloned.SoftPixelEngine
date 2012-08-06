//
// ================== MeshViewer - 14/08/2011 ==================
//
// zlib/libpng License
// Copyright (c) 2011 by Lukas Hermanns
//

#include <SoftPixelEngine.hpp>
#include <windows.h>

#include <boost/foreach.hpp>

using namespace sp;

/* === Global members === */

SoftPixelDevice* spDevice           = 0;
video::RenderSystem* spRenderer     = 0;
video::RenderContext* spContext     = 0;
scene::CollisionDetector* spColl    = 0;
scene::SceneGraph* spScene          = 0;
io::InputControl* spControl         = 0;

scene::Camera* Cam                  = 0;
scene::Light* Lit                   = 0;
scene::Mesh* Obj                    = 0;
scene::SkeletalAnimation* BoneAnim  = 0;
scene::AnimationSkeleton* Skeleton  = 0;

f32 AnimSeek = 0.0f;
bool AnimSeekDrag = false, ObjTurn = false;
bool AnimSeekHighlighted = false;

bool Wire = false, ShowSkeleton = true;

io::stringc MeshFilename;
FILETIME LastChange;

/* === Functions === */

static void CreateDevice(const dim::size2di &ScrSize)
{
    /* Create basic devices */
    spDevice    = createGraphicsDevice(
        video::RENDERER_OPENGL, ScrSize, 32, "SoftPixel Engine MeshViewer (v.1.2.1)",
        false, SDeviceFlags(false, true, false, 0, true)
    );
    
    spContext   = spDevice->getRenderContext();
    spRenderer  = spDevice->getRenderSystem();
    spControl   = spDevice->getInputControl();
    
    spScene     = spDevice->createSceneGraph();
    spColl      = spDevice->getCollisionDetector();
    
    /* Create scene*/
    Cam = spScene->createCamera();
    Cam->setRange(0.1f, 150.0f);
    
    Lit = spScene->createLight();
    spScene->setLighting();
    
    /* Create empty object */
    Obj = spScene->createMesh();
}

static FILETIME GetFileChangeTime(const io::stringc &Filename)
{
    /* Get file change time data */
    WIN32_FILE_ATTRIBUTE_DATA Attrib;
    GetFileAttributesEx(Filename.c_str(), GetFileExInfoStandard, &Attrib);
    return Attrib.ftLastWriteTime;
}

static bool IsTimeEqual(const FILETIME &TimeA, const FILETIME &TimeB)
{
    return (
        TimeA.dwHighDateTime == TimeB.dwHighDateTime &&
        TimeA.dwLowDateTime == TimeB.dwLowDateTime
    );
}

static void LoadMesh(const io::stringc &Filename)
{
    /* Store mesh filename and change date */
    MeshFilename = Filename;
    
    LastChange = GetFileChangeTime(Filename);
    
    /* Delete old objects */
    //spRenderer->clearTextureList();
    spColl->clearScene();
    spScene->clearScene(false, true, false, false, false, false);
    spScene->clearAnimations();
    
    /* Load mesh */
    Obj = spScene->loadMesh(Filename);
    
    /* Get animation */
    scene::Animation* Anim = Obj->getAnimation();
    
    if (Anim && Anim->getType() == scene::ANIMATION_SKELETAL)
    {
        BoneAnim = static_cast<scene::SkeletalAnimation*>(Anim);
        Skeleton = BoneAnim->getActiveSkeleton();
    }
    else
    {
        BoneAnim = 0;
        Skeleton = 0;
    }
    
    AnimSeek        = 0.0f;
    AnimSeekDrag    = false;
    ObjTurn         = false;
    
    if (!BoneAnim)
    {
        std::list<scene::Mesh*> MeshList = spScene->getMeshList();
        
        if (MeshList.size() > 1)
            Obj = spScene->createMeshList(MeshList, true);
        
        Obj->centerOrigin();
    }
    
    /* Configure mesh */
    const dim::aabbox3df BoundBox(Obj->getMeshBoundingBox());
    const f32 MaxSize = BoundBox.Max.getMax();
    
    Obj->setScale(1.0f / MaxSize);
    Obj->setPosition(dim::vector3df(0, 0, 2));
    
    Obj->getMaterial()->setPolygonOffset(true, 1.0f);
    Obj->getMaterial()->setAlphaMethod(video::CMPSIZE_GREATER, 0.5f);
    Obj->getMaterial()->setBlending(false);
    Obj->getMaterial()->setRenderFace(video::FACE_BOTH);
    
    /* Create picking object */
    spColl->addPickingMesh(Obj);
}

static void ApplyTexture(const io::stringc &Filename)
{
    /* Get pick intersection */
    const dim::line3df PickLine(Cam->getPickingLine(spControl->getCursorPosition()));
    
    std::list<scene::SPickingContact> PickList = spColl->pickIntersection(PickLine);
    
    if (PickList.empty())
        return;
    
    /* Check intersection and get mesh object */
    const scene::SPickingContact Contact = *PickList.begin();
    
    if (!Contact.Mesh)
        return;
    
    video::MeshBuffer* Surface = Contact.Mesh->getMeshBuffer(Contact.SurfaceIndex);
    
    if (!Surface)
        return;
    
    /* Apply texture to surface */
    Surface->clearTextureList();
    Surface->addTexture(spRenderer->loadTexture(Filename));
}

static void UpdateControls()
{
    if (spControl->keyHit(io::KEY_W))
    {
        Wire = !Wire;
        
        if (Wire)
        {
            spScene->setWireframe(video::WIREFRAME_LINES);
            Obj->getMaterial()->setRenderFace(video::FACE_FRONT);
        }
        else
        {
            spScene->setWireframe(video::WIREFRAME_SOLID);
            Obj->getMaterial()->setRenderFace(video::FACE_BOTH);
        }
    }
    
    if (spControl->keyHit(io::KEY_T))
        spRenderer->setRenderState(video::RENDER_TEXTURE, !spRenderer->getRenderState(video::RENDER_TEXTURE));
    
    if (spControl->keyHit(io::KEY_L))
        spScene->setLighting(!spScene->getLighting());
    
    if (spControl->keyHit(io::KEY_J))
        ShowSkeleton = !ShowSkeleton;
    
    if (spControl->keyHit(io::KEY_S))
    {
        Obj->setShading(
            (Obj->getMaterial()->getShading() == video::SHADING_GOURAUD) ? video::SHADING_FLAT : video::SHADING_GOURAUD, true
        );
    }
}

static void UpdateObjectMovement()
{
    /* Rotate object */
    const dim::point2df MouseSpeed(spControl->getCursorSpeed().cast<f32>());
    
    if (!AnimSeekHighlighted && spControl->mouseDown(io::MOUSE_LEFT))
    {
        dim::matrix4f Rot;
        
        Rot.rotateY(-MouseSpeed.X * 0.5f);
        Rot.rotateX(-MouseSpeed.Y * 0.5f);
        
        Obj->setRotationMatrix(Rot * Obj->getRotationMatrix());
        
        ObjTurn = true;
    }
    else
        ObjTurn = false;
    
    /* Move object */
    Obj->translate(dim::vector3df(
        0, 0, static_cast<f32>(-spControl->getMouseWheel()) * 0.2f
    ));
}

static void DrawAnimationTrack(const dim::rect2di &Rect)
{
    /* Get track rectangle */
    static const s32 BorderSize = 2;
    
    const f32 CtrlWidth = 20;
    const f32 TrackLen = Rect.Right - Rect.Left - CtrlWidth;
    
    const dim::rect2di CtrlRect(
        Rect.Left + static_cast<s32>(TrackLen * AnimSeek),
        Rect.Top - 5,
        Rect.Left + static_cast<s32>(TrackLen * AnimSeek + CtrlWidth),
        Rect.Bottom + 5
    );
    
    /* Update picking */
    AnimSeekHighlighted = false;
    
    const dim::point2di MousePos(spControl->getCursorPosition());
    
    if ( !ObjTurn && ( CtrlRect.isPointCollided(MousePos) || AnimSeekDrag ) )
    {
        AnimSeekHighlighted = true;
        AnimSeekDrag = spControl->mouseDown(io::MOUSE_LEFT);
    }
    else
        AnimSeekDrag = false;
    
    /* Update seek drag */
    if (AnimSeekDrag)
    {
        AnimSeek = static_cast<f32>(MousePos.X - Rect.Left - CtrlWidth/2) / TrackLen;
        math::Clamp(AnimSeek, 0.0f, 1.0f);
        
        /* Update animation sequence */
        BoneAnim->setupManualAnimation(Obj);
        BoneAnim->interpolateSequence(0, scene::ANIM_LAST_FRAME, AnimSeek);
        
        Skeleton->transformVertices();
        
        Obj->updateVertexBuffer();
    }
    
    /* Draw animation track */
    spRenderer->draw2DRectangle(Rect, video::color(0));
    spRenderer->draw2DRectangle(
        Rect + dim::rect2di(BorderSize, BorderSize, -BorderSize, -BorderSize), video::color(255)
    );
    
    /* Draw keyframes */
    const u32 KeyframeCount = BoneAnim->getKeyframeCount();
    
    if (KeyframeCount > 0 && KeyframeCount < (Rect.Right - Rect.Left) / (CtrlWidth/2))
    {
        const s32 FirstPos  = Rect.Left + static_cast<s32>(CtrlWidth / 2);
        const s32 LastPos   = Rect.Right - static_cast<s32>(CtrlWidth / 2);
        
        s32 Pos;
        
        for (u32 i = 0; i < KeyframeCount; ++i)
        {
            Pos = FirstPos + (LastPos - FirstPos) * i / (KeyframeCount - 1);
            
            spRenderer->draw2DLine(
                dim::point2di(Pos, Rect.Top + BorderSize),
                dim::point2di(Pos, Rect.Bottom - BorderSize),
                video::color(100)
            );
        }
    }
    
    /* Draw track controller */
    spRenderer->draw2DRectangle(CtrlRect, video::color(0));
    spRenderer->draw2DRectangle(
        CtrlRect + dim::rect2di(BorderSize, BorderSize, -BorderSize, -BorderSize),
        AnimSeekHighlighted ? video::color(128, 128, 255) : video::color(255)
    );
}

#ifdef SP_PLATFORM_WINDOWS
int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR lpCmdLine, int nShowCmd)
#else
int main(void)
#endif
{
    /* Setup mesh viewer */
    io::Log::open();
    
    /* Create basic devices */
    const dim::size2di ScrSize(800, 600);
    
    CreateDevice(ScrSize);
    
    /* Create background texture */
    video::Texture* BgTex = spRenderer->createTexture(2, video::PIXELFORMAT_RGBA);
    BgTex->setFilter(video::FILTER_LINEAR);
    BgTex->setMipMapping(false);
    
    video::color ImageBuffer[4] = { video::color(100), video::color(75), video::color(75), video::color(100) };
    BgTex->setupImageBuffer(&ImageBuffer[0].Red);
    
    /* Load font */
    video::Font* Font = spRenderer->createFont("Arial", 20, video::FONT_BOLD);
    
    /* User settings */
    u64 LastChangeCheckTime = 0;
    
    /* Main loop */
    while (spDevice->updateEvent() && !spControl->keyDown(io::KEY_ESCAPE))
    {
        spRenderer->clearBuffers();
        
        /* Wait for new mesh */
        const io::stringc Filename = static_cast<SoftPixelDeviceWin32*>(spDevice)->getDropFilename();
        
        if (Filename.size())
        {
            if (spRenderer->getImageFileFormat(Filename) != video::IMAGEFORMAT_UNKNOWN)
                ApplyTexture(Filename);
            else
                LoadMesh(Filename);
        }
        
        /* Check for file change */
        if (MeshFilename.size() && io::Timer::millisecs() > LastChangeCheckTime + 500 && io::FileSystem().findFile(MeshFilename))
        {
            LastChangeCheckTime = io::Timer::millisecs();
            
            FILETIME FileTime = GetFileChangeTime(MeshFilename);
            
            if (!IsTimeEqual(FileTime, LastChange))
                LoadMesh(MeshFilename);
        }
        
        /* Update controls and object movement */
        UpdateControls();
        UpdateObjectMovement();
        
        /* Draw background image */
        spRenderer->beginDrawing2D();
        {
            spRenderer->draw2DImage(
                BgTex,
                dim::rect2di(0, 0, ScrSize.Width, ScrSize.Height),
                dim::rect2df(0, 0, static_cast<f32>(ScrSize.Width) / 20, static_cast<f32>(ScrSize.Height) / 20)
            );
        }
        spRenderer->endDrawing2D();
        
        /* Draw 3D scene */
        spScene->renderScene();
        
        /* Draw animation skeleton */
        if (ShowSkeleton && Skeleton)
        {
            spRenderer->beginDrawing3D();
            {
                Skeleton->render(Obj->getTransformation(true));
            }
            spRenderer->endDrawing3D();
        }
        
        spRenderer->beginDrawing2D();
        {
            const video::color Color(255);
            
            spRenderer->draw2DText(Font, dim::point2di(10, 10), "W (Wireframe), T (Texture Mapping), L (Lighting), S (Shading), J (Animation Joints)", Color);
            
            if (Obj)
            {
                /* Draw information about the object */
                spRenderer->draw2DText(Font, dim::point2di(10,  40), "Filename: " + MeshFilename.getFilePart(), Color);
                spRenderer->draw2DText(Font, dim::point2di(10,  60), "Surfaces: " + io::stringc(Obj->getMeshBufferCount()), Color);
                spRenderer->draw2DText(Font, dim::point2di(10,  80), "Triangles: " + io::stringc(Obj->getTriangleCount()), Color);
                spRenderer->draw2DText(Font, dim::point2di(10, 100), "Vertices: " + io::stringc(Obj->getVertexCount()), Color);
                
                if (BoneAnim && Skeleton)
                {
                    /* Get current frame */
                    const u32 KeyframeCount = BoneAnim->getKeyframeCount();
                    const u32 CurrentFrame  = static_cast<u32>(AnimSeek * (KeyframeCount - 1));
                    
                    /* Draw information about the animation */
                    spRenderer->draw2DText(Font, dim::point2di(10, 130), "Animation Joints: " + io::stringc(Skeleton->getJointList().size()), Color);
                    spRenderer->draw2DText(Font, dim::point2di(10, 150), "Animation Keyframes: " + io::stringc(KeyframeCount), Color);
                    spRenderer->draw2DText(Font, dim::point2di(10, 180), "Current Keyframe: " + io::stringc(CurrentFrame), Color);
                    
                    /* Update and draw animation track */
                    const dim::rect2di TrackRect(15, ScrSize.Height - 35, ScrSize.Width - 15, ScrSize.Height - 15);
                    
                    DrawAnimationTrack(TrackRect);
                }
            }
        }
        spRenderer->endDrawing2D();
        
        spContext->flipBuffers();
    }
    
    deleteDevice();
    
    return 0;
}



// ======================================================
