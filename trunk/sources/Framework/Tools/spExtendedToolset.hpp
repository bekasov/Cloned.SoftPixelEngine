/*
 * Extended toolset header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_TOOL_EXTENSIONSET_H__
#define __SP_TOOL_EXTENSIONSET_H__


#include "Base/spStandard.hpp"
#include "SceneGraph/spSceneCamera.hpp"
#include "RenderSystem/spTextureFlags.hpp"
#include "FileFormats/Image/spImageFormatInterfaces.hpp"
#include "FileFormats/Sound/spSoundLoader.hpp"
#include "Framework/Tools/spToolScriptLoader.hpp"
#include "Framework/Tools/spToolModelCombiner.hpp"
#include "Framework/Tools/spToolLightmapGenerator.hpp"
#include "Framework/Tools/spToolTextureManipulator.hpp"
#include "Framework/Tools/spToolParticleAnimator.hpp"
#include "Framework/Tools/spToolXMLParser.hpp"
#include "Framework/Tools/spToolPathFinder.hpp"
#include "Framework/Tools/spUtilityInputService.hpp"
#include "Framework/Tools/spStoryboardOpLogicGate.hpp"
#include "Framework/Network/spNetworkSystem.hpp"


namespace sp
{
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
\param Cam: Camera object which is to be moved.
\param MoveSpeed: Speed which is to be used for moving around.
\param TurnSpeed: Speed which is to be used for looking around.
\param MaxTurnDegree: Maximal degree for the X-Axis in rotation.
\param UseArrowKeys: If true the arrow keys are enabled. Set it to false if the arrow keys are used for other operations.
*/
SP_EXPORT void moveCameraFree(
    scene::Camera* Cam = 0,
    const f32 MoveSpeed = 0.25f, const f32 TurnSpeed = 0.25f, const f32 MaxTurnDegree = 90.0f,
    bool UseArrowKeys = true
);

} // /namespace Toolset


//! Namespace for debugging functions.
namespace Debugging
{

SP_EXPORT io::stringc toString(const dim::vector3df     &Value);
SP_EXPORT io::stringc toString(const dim::quaternion    &Value);
SP_EXPORT io::stringc toString(const dim::matrix4f      &Value);
SP_EXPORT io::stringc toString(const dim::matrix2f      &Value);
SP_EXPORT io::stringc toString(const video::color       &Value);

SP_EXPORT io::stringc toString(const video::EPixelFormats           PixelFormat         );
SP_EXPORT io::stringc toString(const video::EImageFileFormats       ImageFileFormat     );
SP_EXPORT io::stringc toString(const video::ETextureFilters         TextureFilter       );
SP_EXPORT io::stringc toString(const video::ETextureMipMapFilters   TextureMipMapFilter );
SP_EXPORT io::stringc toString(const video::ETextureWrapModes       TextureWrapMode     );
SP_EXPORT io::stringc toString(const video::ETextureDimensions      TextureDimension    );
SP_EXPORT io::stringc toString(const video::EHWTextureFormats       HWTextureFormat     );
SP_EXPORT io::stringc toString(const video::ECubeMapDirections      CubeMapDirection    );
SP_EXPORT io::stringc toString(const video::EAlphaBlendingTypes     AlphaBlendingType   );
SP_EXPORT io::stringc toString(const video::EImageBufferTypes       ImageBufferType     );
SP_EXPORT io::stringc toString(const video::ERendererDataTypes      RendererDataType    );

SP_EXPORT io::stringc toString(const audio::EWaveBufferFormats      WaveFormat          );

#ifdef SP_COMPILE_WITH_NETWORKSYSTEM
SP_EXPORT io::stringc toString(const network::ENetworkAdapterTypes  NetworkType         );
#endif

#ifdef SP_COMPILE_WITH_STORYBOARD
SP_EXPORT io::stringc toString(const tool::EStoryboardLogicGates    GateType            );
#endif

} // /namespace Debugging


} // /namespace tool

} // /namespace sp


#endif



// ================================================================================
