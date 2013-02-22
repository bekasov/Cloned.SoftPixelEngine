/*
 * Extended toolset header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_UTILITY_TOOLSET_H__
#define __SP_UTILITY_TOOLSET_H__


#include "Base/spStandard.hpp"
#include "Framework/Tools/spStoryboardOpLogicGate.hpp"
#include "Framework/Tools/spLightmapGenerator.hpp"
#include "Framework/Tools/spToolScriptLoader.hpp"
#include "Framework/Tools/spToolModelCombiner.hpp"
#include "Framework/Tools/spToolTextureManipulator.hpp"
#include "Framework/Tools/spToolParticleAnimator.hpp"
#include "Framework/Tools/spToolXMLParser.hpp"
#include "Framework/Tools/spToolPathFinder.hpp"
#include "Framework/Tools/spUtilityDebugging.hpp"
#include "Framework/Tools/spUtilityInputService.hpp"
#include "Framework/Tools/spUtilityCommandLine.hpp"
#include "Framework/Tools/spUtilityMatrixTexture.hpp"


namespace sp
{

namespace video
{
    class Texture;
}
namespace scene
{
    class Scene;
    class Mesh;
    class Camera;
}

namespace tool
{


/**
Extended toolset class. Here you will find some useful functions when you are making just small
applications or test programs.
*/
namespace Toolset
{

/**
Moves a Camera object free. This function is normally used for debuging while the project is still in progress.
Use the arrow keys for W/A/S/D like in first-person-shooters.
\param[in] Cam Camera object which is to be moved.
\param[in] MoveSpeed Specifies the speed for moving around. Multiply this with
the global speed, to adjust the movement when the FPS goes up and down.
\param[in] TurnSpeed Specifies the speed for looking around.
\param[in] MaxTurnDegree Specifies the maximal degree for the rotation on X-Axis.
\param[in] UseArrowKeys Specifies whether the arrow keys can also be used for moving or not.
Set this to false if the arrow keys are used for other operations.
\see io::Timer::getGlobalSpeed
*/
SP_EXPORT void moveCameraFree(
    scene::Camera* Cam = 0, f32 MoveSpeed = 0.25f, f32 TurnSpeed = 0.25f, f32 MaxTurnDegree = 90.0f, bool UseArrowKeys = true
);

SP_EXPORT void presentModel(scene::Mesh* Model, bool UseZoome = true);

} // /namespace Toolset


} // /namespace tool

} // /namespace sp


#endif



// ================================================================================
