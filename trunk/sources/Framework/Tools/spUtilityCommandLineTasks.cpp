/*
 * Command line tasks file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "Framework/Tools/spUtilityCommandLineTasks.hpp"

#ifdef SP_COMPILE_WITH_COMMANDLINE


#include "Base/spSharedObjects.hpp"
#include "Base/spInputOutputControl.hpp"
#include "Base/spInputOutputOSInformator.hpp"
#include "RenderSystem/spRenderSystem.hpp"
#include "RenderSystem/spRenderContext.hpp"
#include "SceneGraph/spSceneGraph.hpp"
#include "Platform/spSoftPixelDevice.hpp"
#include "Framework/Network/spNetworkSystem.hpp"
#include "Framework/Tools/spUtilityCommandLine.hpp"
#include "Framework/Tools/spUtilityDebugging.hpp"

#include <boost/foreach.hpp>


namespace sp
{

extern SoftPixelDevice* GlbEngineDev;
extern video::RenderSystem* GlbRenderSys;
extern video::RenderContext* GlbRenderCtx;
extern scene::SceneGraph* GlbSceneGraph;
extern io::InputControl* GlbInputCtrl;
extern io::OSInformator* GlbPlatformInfo;

namespace tool
{


namespace CommandLineTasks
{

static const c8* ERR_NO_SCENE_GRAPH = "No active scene graph";
static const c8* ERR_NO_CAMERA      = "No active camera";
static const c8* ERR_NO_CONTEXT     = "No active render context";

SP_EXPORT void cmdWireframe(CommandLineUI &Cmd, scene::SceneGraph* Graph, const video::EWireframeTypes Type)
{
    if (Graph)
    {
        Graph->setWireframe(Type);
        Cmd.confirm("Switched Wireframe Mode");
    }
    else
        Cmd.error(ERR_NO_SCENE_GRAPH);
}

SP_EXPORT void cmdFullscreen(CommandLineUI &Cmd)
{
    video::RenderContext* ActiveContext = video::RenderContext::getActiveRenderContext();
    
    if (ActiveContext)
    {
        ActiveContext->setFullscreen(!ActiveContext->getFullscreen());
        if (ActiveContext->getFullscreen())
            Cmd.confirm("Switched To Fullscreen Mode");
        else
            Cmd.confirm("Switched To Window Mode");
    }
    else
        Cmd.error(ERR_NO_CONTEXT);
}

SP_EXPORT void cmdView(CommandLineUI &Cmd, scene::Camera* Cam)
{
    if (Cam)
    {
        const dim::vector3df Pos(Cam->getPosition(true));
        const dim::vector3df Rot(Cam->getRotation(true));
        
        Cmd.confirm(
            "Camera Position = ( " +
            io::stringc::numberFloat(Pos.X, 1, true) + " , " +
            io::stringc::numberFloat(Pos.Y, 1, true) + " , " +
            io::stringc::numberFloat(Pos.Z, 1, true) + " )"
        );
        Cmd.confirm(
            "Camera Rotation = ( " +
            io::stringc::numberFloat(Rot.X, 1, true) + " , " +
            io::stringc::numberFloat(Rot.Y, 1, true) + " , " +
            io::stringc::numberFloat(Rot.Z, 1, true) + " )"
        );
    }
    else
        Cmd.error(ERR_NO_CAMERA);
}

SP_EXPORT void cmdVsync(CommandLineUI &Cmd)
{
    video::RenderContext* ActiveContext = video::RenderContext::getActiveRenderContext();
    
    if (ActiveContext)
    {
        ActiveContext->setVsync(!ActiveContext->getVsync());
        Cmd.confirm(
            io::stringc("Vertical Synchronisation ") + (ActiveContext->getVsync() ? "Enabled" : "Disabled")
        );
    }
    else
        Cmd.error(ERR_NO_CONTEXT);
}

SP_EXPORT void cmdScene(CommandLineUI &Cmd)
{
    Cmd.confirm("Objects:      " + io::stringc(gSharedObjects.SceneMngr->getSceneObjectsCount())    );
    Cmd.confirm("Scene Nodes:  " + io::stringc(gSharedObjects.SceneMngr->getNodeList().size())      );
    Cmd.confirm("Cameras:      " + io::stringc(gSharedObjects.SceneMngr->getCameraList().size())    );
    Cmd.confirm("Lights:       " + io::stringc(gSharedObjects.SceneMngr->getLightList().size())     );
    Cmd.confirm("Billboards:   " + io::stringc(gSharedObjects.SceneMngr->getBillboardList().size()) );
    Cmd.confirm("Terrains:     " + io::stringc(gSharedObjects.SceneMngr->getTerrainList().size())   );
    Cmd.confirm("Meshes:       " + io::stringc(gSharedObjects.SceneMngr->getMeshList().size())      );
    Cmd.confirm("Mesh Buffers: " + io::stringc(gSharedObjects.SceneMngr->getSceneMeshBufferCount()) );
    Cmd.confirm("Vertices:     " + io::stringc(gSharedObjects.SceneMngr->getSceneVertexCount())     );
    Cmd.confirm("Triangles:    " + io::stringc(gSharedObjects.SceneMngr->getSceneTriangleCount())   );
}

SP_EXPORT void cmdHardware(CommandLineUI &Cmd)
{
    /* Print OS information */
    Cmd.confirm(GlbPlatformInfo->getOSVersion());
    
    /* Print CPU information */
    const u32 CPUCores = GlbPlatformInfo->getProcessorCount();
    const u32 CPUSpeed = GlbPlatformInfo->getProcessorSpeed();
    
    if (CPUCores > 1)
        Cmd.confirm("CPU @ " + io::stringc(CPUCores) + " x " + io::stringc(CPUSpeed) + " MHz");
    else
        Cmd.confirm("CPU @ " + io::stringc(CPUSpeed) + " MHz");
    
    /* Print virtual memory information */
    u64 TotalMem = 0, FreeMem = 0;
    
    GlbPlatformInfo->getVirtualMemory(TotalMem, FreeMem);
    
    Cmd.confirm(
        "Free Virtual Memory: " +
        io::stringc::numberSeperators(FreeMem) + " / " +
        io::stringc::numberSeperators(TotalMem) + " (MB)"
    );
    
    /* Print graphics hardware information */
    Cmd.confirm(GlbRenderSys->getRenderer() + ": " + GlbRenderSys->getVendor());
}

SP_EXPORT void cmdNetwork(CommandLineUI &Cmd)
{
    #ifdef SP_COMPILE_WITH_NETWORKSYSTEM
    
    Cmd.confirm("Network Adapters:");
    
    std::list<network::SNetworkAdapter> Adapters = network::NetworkSystem::getNetworkAdapters();
    
    if (!Adapters.empty())
    {
        foreach (const network::SNetworkAdapter &Adapter, Adapters)
        {
            Cmd.confirm(
                "  " + (Adapter.Description.size() ? Adapter.Description : io::stringc("Unnamed")) +
                " [ IP = " + Adapter.IPAddress + ", Mask = " + Adapter.IPMask + (Adapter.Enabled ? ", Enabled ]" : ", Disabled ]")
            );
        }
    }
    else
        Cmd.confirm("No network adapters available");
    
    if (!GlbEngineDev->getNetworkSystemList().empty())
    {
        foreach (network::NetworkSystem* NetSys, GlbEngineDev->getNetworkSystemList())
        {
            Cmd.confirm(NetSys->getDescription() + ":");
            
            if (NetSys->isConnected() && NetSys->getServer())
            {
                if (NetSys->isServer())
                    Cmd.confirm("  Server IP: 127.0.0.1");
                else
                    Cmd.confirm("  Server IP: " + NetSys->getServer()->getAddress().getIPAddressName());
            }
            else
                Cmd.confirm("  Server IP: none");
            
            Cmd.confirm("  Clients: " + io::stringc(NetSys->getClientList().size()));
            Cmd.confirm("  Connected: " + io::stringc(NetSys->isConnected() ? "Yes" : "No"));
            Cmd.confirm("  Running Session: " + io::stringc(NetSys->isSessionRunning() ? "Yes" : "No"));
        }
    }
    else
        Cmd.confirm("No network system has been created");
    
    #else
    
    Cmd.error("Engine was not compiled with network system");
    
    #endif
}

SP_EXPORT void cmdResolution(CommandLineUI &Cmd, const io::stringc &Command)
{
    /* Get active render context */
    video::RenderContext* ActiveContext = video::RenderContext::getActiveRenderContext();
    
    if (!ActiveContext)
    {
        Cmd.error(ERR_NO_CONTEXT);
        return;
    }
    
    /* Get resolution string */
    io::stringc ResParam;
    if (!Cmd.getCmdParam(Command, ResParam))
        return;
    
    /* Find separator character 'x' */
    const s32 SeparatorPos = ResParam.find("x");
    
    if (SeparatorPos == -1)
    {
        Cmd.error("Missing 'x' separator character in resolution parameter (e.g. \"800x600\")");
        return;
    }
    
    /* Extract resolution values */
    const dim::size2di Resolution(
        ResParam.left(SeparatorPos).val<s32>(),
        ResParam.right(ResParam.size() - SeparatorPos - 1).val<s32>()
    );
    
    /* Change new resolution */
    ActiveContext->setResolution(Resolution);
    
    /* Update scene resolution */
    Cmd.setRect(dim::rect2di(0, 0, Resolution.Width, Resolution.Height/2));
    
    if (GlbSceneGraph && GlbSceneGraph->getActiveCamera())
        GlbSceneGraph->getActiveCamera()->setViewport(dim::rect2di(0, 0, Resolution.Width, Resolution.Height));
    
    /* Print confirmation message */
    Cmd.confirm("Changed Resolution: ( " + io::stringc(Resolution.Width) + " x " + io::stringc(Resolution.Height) + " )");
}

SP_EXPORT void cmdDrawCalls(CommandLineUI &Cmd)
{
    #ifdef SP_DEBUGMODE
    Cmd.confirm("Draw Calls: " + io::stringc(video::RenderSystem::queryDrawCalls()));
    Cmd.confirm("Mesh Buffer Bindings: " + io::stringc(video::RenderSystem::queryMeshBufferBindings()));
    Cmd.confirm("Texture Layer Bindings: " + io::stringc(video::RenderSystem::queryTextureLayerBindings()));
    Cmd.confirm("Material Updates: " + io::stringc(video::RenderSystem::queryMaterialUpdates()));
    #else
    Cmd.error("Draw call counter is only available in debug mode");
    #endif
}

SP_EXPORT void cmdShowImages(CommandLineUI &Cmd, const io::stringc &Command)
{
    /* Get search filter string */
    io::stringc FilterParam;
    
    if (Command.size() > 6)
    {
        if (!Cmd.getCmdParam(Command, FilterParam))
            return;
        FilterParam.makeLower();
    }
    
    /* Search all textures */
    u32 ImgCount = 0;
    
    foreach (video::Texture* Tex, GlbRenderSys->getTextureList())
    {
        if (FilterParam.empty() || Tex->getFilename().lower().find(FilterParam) != -1)
        {
            if (Tex->getFilename().size())
            {
                Cmd.confirm("Filename: " + Tex->getFilename().getFilePart());
                
                const io::stringc Path(Tex->getFilename().getPathPart());
                if (!Path.empty())
                    Cmd.confirm("Path: " + Path);
            }
            else
                Cmd.confirm("[ No Filename ]");
            
            Cmd.confirm("Size: " + io::stringc(Tex->getSize().Width) + " x " + io::stringc(Tex->getSize().Height));
            Cmd.confirm(
                "Pixel Format: " + tool::Debugging::toString(Tex->getFormat()) +
                " ( " + tool::Debugging::toString(Tex->getHardwareFormat()) + " )"
            );
            
            Cmd.image(Tex);
            Cmd.blank();
            
            ++ImgCount;
        }
    }
    
    Cmd.confirm(io::stringc(ImgCount) + " image(s) found");
}

SP_EXPORT void cmdBackground(CommandLineUI &Cmd, const io::stringc &Command)
{
    /* Get alpha string */
    io::stringc AlphaParam;
    if (!Cmd.getCmdParam(Command, AlphaParam))
        return;
    
    /* Setup new background transparency */
    video::color BgColor(Cmd.getBackgroundColor());
    BgColor.Alpha = static_cast<u8>(math::MinMax(AlphaParam.val<s32>(), 0, 255));
    Cmd.setBackgroundColor(BgColor);
}

} // /namespace CommandLineTasks


} // /namespace tool

} // /namespace sp


#endif



// ================================================================================
