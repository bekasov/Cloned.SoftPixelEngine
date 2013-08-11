/*
 * Debugging header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_UTILTIY_DEBUGGING_H__
#define __SP_UTILTIY_DEBUGGING_H__


#include "Base/spStandard.hpp"
#include "RenderSystem/spTextureFlags.hpp"
#include "FileFormats/Image/spImageFormatInterfaces.hpp"
#include "FileFormats/Sound/spSoundLoader.hpp"
#include "Framework/Tools/spToolScriptLoader.hpp"
#include "Framework/Tools/spToolModelCombiner.hpp"
#include "Framework/Tools/spToolTextureManipulator.hpp"
#include "Framework/Tools/spToolParticleAnimator.hpp"
#include "Framework/Tools/spToolXMLParser.hpp"
#include "Framework/Tools/spToolPathFinder.hpp"
#include "Framework/Tools/spUtilityInputService.hpp"
#include "Framework/Tools/spStoryboardLogicGate.hpp"
#include "Framework/Tools/spLightmapGenerator.hpp"
#include "Framework/Network/spNetworkSystem.hpp"


namespace sp
{
namespace tool
{


//! Namespace for debugging functions.
namespace Debugging
{

SP_EXPORT io::stringc toString(const dim::vector3df     &Value);
SP_EXPORT io::stringc toString(const dim::quaternion    &Value);
SP_EXPORT io::stringc toString(const dim::matrix4f      &Value);
SP_EXPORT io::stringc toString(const dim::matrix2f      &Value);
SP_EXPORT io::stringc toString(const video::color       &Value);

SP_EXPORT io::stringc toString(const video::EPixelFormats               PixelFormat         );
SP_EXPORT io::stringc toString(const video::EImageFileFormats           ImageFileFormat     );
SP_EXPORT io::stringc toString(const video::ETextureFilters             TextureFilter       );
SP_EXPORT io::stringc toString(const video::ETextureMipMapFilters       TextureMipMapFilter );
SP_EXPORT io::stringc toString(const video::ETextureWrapModes           TextureWrapMode     );
SP_EXPORT io::stringc toString(const video::ETextureTypes               TextureType         );
SP_EXPORT io::stringc toString(const video::EHWTextureFormats           HWTextureFormat     );
SP_EXPORT io::stringc toString(const video::ECubeMapDirections          CubeMapDirection    );
SP_EXPORT io::stringc toString(const video::EAlphaBlendingTypes         AlphaBlendingType   );
SP_EXPORT io::stringc toString(const video::EImageBufferTypes           ImageBufferType     );
SP_EXPORT io::stringc toString(const video::ERendererDataTypes          RendererDataType    );

SP_EXPORT io::stringc toString(const audio::EWaveBufferFormats          WaveFormat          );

#ifdef SP_COMPILE_WITH_NETWORKSYSTEM
SP_EXPORT io::stringc toString(const network::ENetworkAdapterTypes      NetworkType         );
#endif

#ifdef SP_COMPILE_WITH_STORYBOARD
SP_EXPORT io::stringc toString(const tool::ELogicGates                  GateType            );
#endif

#ifdef SP_COMPILE_WITH_LIGHTMAPGENERATOR
SP_EXPORT io::stringc toString(const tool::ELightmapGenerationStates    LightmapGenState    );
#endif

} // /namespace Debugging


} // /namespace tool

} // /namespace sp


#endif



// ================================================================================
