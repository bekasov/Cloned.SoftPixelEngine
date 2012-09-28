/*
 * Standard definition file
 * 
 * This file is part of the "SoftPixel Engine Wrapper" (Copyright (c) 2008 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "spwStandard.hpp"


/*
 * ===================== Global members =====================
 */

SoftPixelDevice*                            g_Device            = 0;
io::InputControl*                           g_InputControl      = 0;

video::RenderSystem*                        g_RenderSystem      = 0;
video::RenderContext*                       g_RenderContext     = 0;

scene::SceneGraph*                          g_SceneGraph        = 0;
scene::CollisionGraph*                      g_CollisionDetector = 0;
scene::StencilManager*                      g_StencilManager    = 0;

io::OSInformator*                           g_OSInformator      = 0;

tool::ScriptLoader*                         g_ScriptLoader      = 0;
audio::SoundDevice*                         g_SoundDevice       = 0;
physics::PhysicsSimulator*                  g_PhysicsSystem     = 0;

io::FileSystem                              g_FileSystem;
io::Timer                                   g_Timer(true);

io::stringc                                 g_AppTitle          = "SoftPixel Engine Wrapper";
io::stringc                                 g_EndMessage        = "";
video::ERenderSystems                       g_DriverType        = video::RENDERER_DIRECT3D9;

video::color                                g_Color, g_PixelColor, g_MultiColor[4];
dim::point2di                               g_Origin2D;
dim::point2di                               g_CameraProject;
dim::size2di                                g_ScreenSize;
bool                                        g_AutoMidHandle     = false;

video::Font*                                g_Font              = 0;
video::Font*                                g_StdFont           = 0;
scene::SceneNode*                           g_ListenerParent    = 0;

int                                         g_ColorDepth        = 0;
bool                                        g_EnableMultiColor  = false;

int                                         g_ScancodeList[237] = { 0 };
io::EMouseKeyCodes                          g_MouseKeyList[3]   = { io::MOUSE_LEFT, io::MOUSE_RIGHT, io::MOUSE_MIDDLE };
int                                         g_MouseZ            = 0;

SDeviceFlags                                g_DeviceFlags(false, false);
LP                                          g_WholeSurfaceCount = 0;

u64                                         g_PickingTime       = 0;
scene::SPickingContact                      g_PickingContact;
std::list<scene::SPickingContact>::iterator g_PickingIterator;

dim::vector3df                              g_PatchAnchorPoints[4][4];
int                                         g_PatchAnchorIndexVert = 0, g_PatchAnchorIndexHorz = 0;

std::map<LP, SWrapSurface>                  g_SurfaceList;
std::map<LP, SWrapTexture>                  g_TextureList;
std::map<LP, SWrapImage>                    g_ImageList;
std::map<int, scene::Collision*>            g_CollisionList;
std::map<LP, SWrapMovie>                    g_MovieList;
std::map<LP, SWrapCamera>                   g_CameraList;
std::map<LP, scene::SShadowLightSource*>    g_ShadowLightList;
std::map<LP, SWrapAnimation>                g_AnimationList;

std::vector<SWrapBrush>                     g_BrushList;
std::vector<SWrapAnimSeq>                   g_AnimSeqList;

std::list<scene::SPickingContact>           g_PickingList;
std::list<dim::matrix4f*>                   g_MatrixList;
std::list<char*>                            g_BufferList;
std::list<SWrapCaptureObject>               g_CaptureWorldList;



// ================================================================================
