/*
 * Extended toolset file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "Framework/Tools/spExtendedToolset.hpp"
#include "FileFormats/Sound/spSoundLoader.hpp"
#include "Platform/spSoftPixelDeviceOS.hpp"
#include "Base/spSharedObjects.hpp"
#include "Base/spTimer.hpp"
#include "SceneGraph/spSceneMesh.hpp"
#include "SceneGraph/spSceneCamera.hpp"


namespace sp
{

extern SoftPixelDevice* GlbEngineDev;
extern io::InputControl* GlbInputCtrl;
extern scene::SceneGraph* GlbSceneGraph;
extern video::RenderSystem* GlbRenderSys;

namespace tool
{


namespace Toolset
{

#if defined(SP_PLATFORM_ANDROID) || defined(SP_PLATFORM_IOS)

static bool spMobileUIDrawCtrl(
    const dim::point2di &Position, s32 SizeInner, s32 SizeOuter, dim::point2df &Offset,
    s32 &Picked, const video::color &Color = video::color(64, 128, 255))
{
    /* General settings */
    bool Result = false;
    Offset = 0.0f;
    
    const dim::rect2di Rect(
        Position.X - SizeOuter, Position.Y - SizeOuter, Position.X + SizeOuter, Position.Y + SizeOuter
    );
    
    bool PickedMem[5] = { 0 };
    
    /* Update user input */
    for (s32 i = 0; i < 5; ++i)
    {
        if (!GlbInputCtrl->isPointerActive(i))
            continue;
        
        const dim::point2di Pos(GlbInputCtrl->getPointerPosition(i).cast<s32>());
        
        if (Rect.overlap(Pos))// || Picked == i)
        {
            /* Apply controller translation */
            Result = true;
            PickedMem[i] = true;
            Picked = i;
            
            Offset = (Pos - Position).cast<f32>();
            
            /* Clamp controller position */
            math::clamp(Offset.X, static_cast<f32>(SizeInner - SizeOuter), static_cast<f32>(SizeOuter - SizeInner));
            math::clamp(Offset.Y, static_cast<f32>(SizeInner - SizeOuter), static_cast<f32>(SizeOuter - SizeInner));
            
            break;
        }
    }
    
    if (Picked >= 0 && !PickedMem[Picked])
        Picked = -1;
    
    /* Draw controller */
    const dim::point2di CtrlPosition(
        Position.X + static_cast<s32>(Offset.X),
        Position.Y + static_cast<s32>(Offset.Y)
    );
    
    GlbRenderSys->draw2DRectangle(Rect, video::color(Color.Red, Color.Green, Color.Blue, 128));
    GlbRenderSys->draw2DRectangle(
        dim::rect2di(CtrlPosition.X - SizeInner, CtrlPosition.Y - SizeInner, CtrlPosition.X + SizeInner, CtrlPosition.Y + SizeInner),
        //Color
        (Picked == 0 ? video::color(255, 0, 0) : (Picked == 1 ? video::color(0, 255, 0) : Color))
    );
    
    return Result;
}

SP_EXPORT void moveCameraFree(
    scene::Camera* Cam, const f32 MoveSpeed, const f32 TurnSpeed, const f32 MaxTurnDegree, bool UseArrowKeys)
{
    /* Static variables */
    static f32 Pitch, Yaw;
    static s32 PickedMove = -1, PickedTurn = -1;
    dim::point2df CtrlVec;
    
    /* Check for default camera usage */
    if (!Cam && !( Cam = GlbSceneGraph->getActiveCamera() ) )
        return;
    
    /* Draw controller in 2D */
    const dim::size2di ScrSize(GlbEngineDev->getResolution());
    
    /* Control translation movement */
    if (spMobileUIDrawCtrl(dim::point2di(100, ScrSize.Height - 100), 35, 75, CtrlVec, PickedMove))
        Cam->move(dim::vector3df(CtrlVec.X, 0, -CtrlVec.Y) * MoveSpeed * 0.01f);
    
    /* Control rotation movement */
    if (spMobileUIDrawCtrl(dim::point2di(ScrSize.Width - 100, ScrSize.Height - 100), 35, 75, CtrlVec, PickedTurn))
    {
        Pitch   += CtrlVec.Y * TurnSpeed * 0.1f;
        Yaw     += CtrlVec.X * TurnSpeed * 0.1f;
    }
    
    if (!math::equal(MaxTurnDegree, 0.0f))
    {
        if (Pitch > MaxTurnDegree)
            Pitch = MaxTurnDegree;
        else if (Pitch < -MaxTurnDegree)
            Pitch = -MaxTurnDegree;
    }
    
    Cam->setRotation(dim::vector3df(Pitch, Yaw, 0));
}

SP_EXPORT void presentModel(scene::Mesh* Model, bool UseZoom)
{
    //!todo! -> touch-screen interactions
    if (Model)
        Model->turn(1.0f);
}

#else

SP_EXPORT void moveCameraFree(
    scene::Camera* Cam, f32 MoveSpeed, f32 TurnSpeed, f32 MaxTurnDegree, bool UseArrowKeys)
{
    /* Static variables */
    static f32 Pitch, Yaw;
    
    /* Check for default camera usage */
    if ( !GlbSceneGraph || !GlbInputCtrl || !Cam && !( Cam = GlbSceneGraph->getActiveCamera() ) )
        return;
    
    /* Control translation movement */
    if ( ( UseArrowKeys && GlbInputCtrl->keyDown(io::KEY_RIGHT) ) || GlbInputCtrl->keyDown(io::KEY_D) )
        Cam->move(dim::vector3df(MoveSpeed, 0, 0));
    if ( ( UseArrowKeys && GlbInputCtrl->keyDown(io::KEY_LEFT) ) || GlbInputCtrl->keyDown(io::KEY_A) )
        Cam->move(dim::vector3df(-MoveSpeed, 0, 0));
    if ( ( UseArrowKeys && GlbInputCtrl->keyDown(io::KEY_UP) ) || GlbInputCtrl->keyDown(io::KEY_W) )
        Cam->move(dim::vector3df(0, 0, MoveSpeed));
    if ( ( UseArrowKeys && GlbInputCtrl->keyDown(io::KEY_DOWN) ) || GlbInputCtrl->keyDown(io::KEY_S) )
        Cam->move(dim::vector3df(0, 0, -MoveSpeed));
    
    /*if (GlbInputCtrl->keyHit(io::KEY_TAB))
    {
        static bool Wire;
        Wire = !Wire;
        GlbSceneGraph->setWireframe(Wire ? video::WIREFRAME_LINES : video::WIREFRAME_SOLID);
    }*/
    
    /* Control rotation movement */
    Pitch += static_cast<f32>(GlbInputCtrl->getCursorSpeed().Y) * TurnSpeed;
    Yaw   += static_cast<f32>(GlbInputCtrl->getCursorSpeed().X) * TurnSpeed;
    
    if (!math::equal(MaxTurnDegree, 0.0f))
    {
        if (Pitch > MaxTurnDegree)
            Pitch = MaxTurnDegree;
        else if (Pitch < -MaxTurnDegree)
            Pitch = -MaxTurnDegree;
    }
    
    Cam->setRotation(dim::vector3df(Pitch, Yaw, 0));
    
    GlbInputCtrl->setCursorPosition(dim::point2di(gSharedObjects.ScreenWidth/2, gSharedObjects.ScreenHeight/2));
}

SP_EXPORT void presentModel(scene::Mesh* Model, bool UseZoom)
{
    if (!GlbInputCtrl || !Model)
        return;
    
    const dim::point2df MouseSpeed(GlbInputCtrl->getCursorSpeed().cast<f32>());
    
    /* Turn object */
    if (GlbInputCtrl->mouseDown(io::MOUSE_LEFT))
    {
        dim::matrix4f Rot;
        
        Rot.rotateY(-MouseSpeed.X * 0.5f);
        Rot.rotateX(-MouseSpeed.Y * 0.5f);
        
        Model->setRotationMatrix(Rot * Model->getRotationMatrix());
    }
    
    /* Move object */
    if (UseZoom)
    {
        Model->translate(dim::vector3df(
            0, 0, static_cast<f32>(-GlbInputCtrl->getMouseWheel()) * 0.2f
        ));
    }
}
    
#endif

SP_EXPORT void drawDebugInfo(
    const video::Font* FontObj, bool Reset, const dim::point2di &Origin, const video::color &Color)
{
    static f64 MinFPS = math::OMEGA, MaxFPS = 0.0, AvgFPS = 0.0;
    static u32 Samples;
    
    /* Get font object */
    if (!FontObj)
        return;
    
    /* Check for internal counting reset */
    if (Reset)
    {
        MinFPS = math::OMEGA;
        MaxFPS = 0.0;
        AvgFPS = 0.0;
        Samples = 0;
    }
    
    /* Get FPS and count frame samples */
    const f64 FPS = io::Timer::getFPS();
    
    ++Samples;
    AvgFPS += FPS;
    
    math::increase(MaxFPS, FPS);
    math::decrease(MinFPS, FPS);
    
    /* Draw text */
    GlbRenderSys->draw2DText(FontObj, dim::point2di(Origin.X, Origin.Y      ), "FPS: " + io::stringc(FPS                ));
    GlbRenderSys->draw2DText(FontObj, dim::point2di(Origin.X, Origin.Y + 25 ), "Min: " + io::stringc(MinFPS             ));
    GlbRenderSys->draw2DText(FontObj, dim::point2di(Origin.X, Origin.Y + 50 ), "Max: " + io::stringc(MaxFPS             ));
    GlbRenderSys->draw2DText(FontObj, dim::point2di(Origin.X, Origin.Y + 75 ), "Avg: " + io::stringc(AvgFPS / Samples   ));
    
    #ifdef SP_DEBUGMODE
    GlbRenderSys->draw2DText(FontObj, dim::point2di(Origin.X, Origin.Y + 110), "Draw Calls: "               + io::stringc(video::RenderSystem::queryDrawCalls           ()));
    GlbRenderSys->draw2DText(FontObj, dim::point2di(Origin.X, Origin.Y + 135), "MeshBuffer Bindings: "      + io::stringc(video::RenderSystem::queryMeshBufferBindings  ()));
    GlbRenderSys->draw2DText(FontObj, dim::point2di(Origin.X, Origin.Y + 160), "TextureLayer Bindings: "    + io::stringc(video::RenderSystem::queryTextureLayerBindings()));
    #endif
}

} // /namespace Toolset


} // /namespace tool

} // /namespace sp



// ================================================================================
