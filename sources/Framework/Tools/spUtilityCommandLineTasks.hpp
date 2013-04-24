/*
 * Command line tasks header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_UTILITY_COMMANDLINE_TASKS_H__
#define __SP_UTILITY_COMMANDLINE_TASKS_H__


#include "Base/spStandard.hpp"

#ifdef SP_COMPILE_WITH_COMMANDLINE


#include "Base/spInputOutputString.hpp"
#include "SceneGraph/spSceneGraph.hpp"


namespace sp
{
namespace tool
{


class CommandLineUI;

namespace CommandLineTasks
{

SP_EXPORT void cmdWireframe (CommandLineUI &Cmd, scene::SceneGraph* Graph, const video::EWireframeTypes Type);
SP_EXPORT void cmdFullscreen(CommandLineUI &Cmd);
SP_EXPORT void cmdView      (CommandLineUI &Cmd, scene::Camera* Cam);
SP_EXPORT void cmdVsync     (CommandLineUI &Cmd);
SP_EXPORT void cmdScene     (CommandLineUI &Cmd);
SP_EXPORT void cmdHardware  (CommandLineUI &Cmd);
SP_EXPORT void cmdNetwork   (CommandLineUI &Cmd);
SP_EXPORT void cmdResolution(CommandLineUI &Cmd, const io::stringc &Command);
SP_EXPORT void cmdDrawCalls (CommandLineUI &Cmd);
SP_EXPORT void cmdShowImages(CommandLineUI &Cmd, const io::stringc &Command);
SP_EXPORT void cmdBackground(CommandLineUI &Cmd, const io::stringc &Command);

} // /namespace CommandLineTasks


} // /namespace tool

} // /namespace sp


#endif

#endif



// ================================================================================
