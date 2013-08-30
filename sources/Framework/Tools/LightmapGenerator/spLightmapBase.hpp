/*
 * Lightmap generator base header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_LIGHTMAP_GENERATOR_BASE_H__
#define __SP_LIGHTMAP_GENERATOR_BASE_H__


#include "Base/spStandard.hpp"

#ifdef SP_COMPILE_WITH_LIGHTMAPGENERATOR


#include "Base/spDimensionMatrix4.hpp"
#include "Base/spMaterialColor.hpp"
#include "Base/spTreeNodeImage.hpp"
#include "Base/spBaseExceptions.hpp"
#include "SceneGraph/spSceneLight.hpp"

#include <boost/function.hpp>
#include <vector>


namespace sp
{

namespace scene
{

class Mesh;
class Light;

}

namespace tool
{


/*
 * Declarations
 */

namespace LightmapGen
{

struct SVertex;
struct STriangle;
struct SFace;
struct SLight;
struct SLightmap;
struct SLightmapTexel;
struct SModel;
struct SAxisData;
struct SRasterizerVertex;

typedef scene::ImageTreeNode<SLightmap> TRectNode;

} // /namespace LightmapGen


/*
 * Macros
 */

static const video::color DEF_LIGHTMAP_AMBIENT  = video::color(20);
static const u32 DEF_LIGHTMAP_SIZE              = 512;
static const f32 DEF_LIGHTMAP_DENSITY           = 10.0f;
static const u32 DEF_LIGHTMAP_BLURRADIUS        = 2;


/*
 * Enumerations
 */

enum ELightmapGenerationsFlags
{
    LIGHTMAPFLAG_NOCOLORS           = 0x00000001, //!< Colored lighting is disabled. When all lights have the diffuse color (255, 255, 255) this flag has no effect.
    LIGHTMAPFLAG_NOTRANSPARENCY     = 0x00000002, //!< Transparency textures ray-casting is disabled. This may occur in much faster lightmap generation.
    LIGHTMAPFLAG_GPU_ACCELERATION   = 0x00000004, //!< Since version 3.3 hardware acclerated lightmap generation is supported. This requires that Direct3D 11 or OpenGL 4.3 is used.
    LIGHTMAPFLAG_RADIOSITY          = 0x00000008, //!< Since version 3.3 radiosity lightmap generation is supported. This requires that the 'LIGHTMAPFLAG_GPU_ACCELERATION' flag is also enabled.
};

enum ELightmapGenerationStates
{
    LIGHTMAPSTATE_INITIALIZING, //!< Initialization state. Occrus at start up.
    LIGHTMAPSTATE_PARTITIONING, //!< Scene partitioning state. Occurs when the scene will be partitioned.
    LIGHTMAPSTATE_SHADING,      //!< Lightmap texel generation state. Occurs every time a new light source is used for lightmap texel generation.
    LIGHTMAPSTATE_BLURING,      //!< Lightmap texture bluring. Occurs when the lightmap image buffers are being blured. Only occurs when bluring is enabled.
    LIGHTMAPSTATE_BAKING,       //!< Final lightmap texture baking state. Occurs when texture bleeding is reduced and the final lightmap textures are created.
    LIGHTMAPSTATE_COMPLETED,    //!< Lightmap generation has been completed successful. Occurs when lightmap generation is done.
};


/*
 * Typedefinitions
 */

/**
The lightmap progress callback will be called several times during lightmap generation as feedback for the user.
\param[in] Progress Specifies the generation progress. This is in the range [0.0 .. 1.0].
\return True when the generation is to be continued. Otherwise lightmap generation will be canceled.
*/
typedef boost::function<bool (f32 Progress)> LightmapProgressCallback;
/**
The lightmap state callback will be called every time the generation state changes,
i.e. when the generation state changes from lightmap-texel-generation to lightmap-texture-bluring.
\param[in] State Specifies the current state of lightmap generation.
\param[in] Info Specifies a short information string about the current state.
\see ELightmapGenerationStates
*/
typedef boost::function<void (const ELightmapGenerationStates State, const io::stringc &Info)> LightmapStateCallback;


/*
 * Structures
 */

struct SP_EXPORT SCastShadowObject
{
    SCastShadowObject(scene::Mesh* Obj) throw(io::NullPointerException);
    ~SCastShadowObject();
    
    /* Members */
    scene::Mesh* Mesh;
};

struct SP_EXPORT SGetShadowObject
{
    SGetShadowObject(scene::Mesh* Obj, bool DefStayAlone = false) throw(io::NullPointerException);
    SGetShadowObject(
        scene::Mesh* Obj, const std::vector< std::vector<f32> > &DefTrianglesDensity,
        bool DefStayAlone = false
    ) throw(io::NullPointerException);
    ~SGetShadowObject();
    
    /* Members */
    scene::Mesh* Mesh;
    bool StayAlone;
    
    std::vector< std::vector<f32> > TrianglesDensity;
};

struct SP_EXPORT SLightmapLight
{
    SLightmapLight();
    SLightmapLight(const scene::Light* Obj) throw(io::NullPointerException);
    ~SLightmapLight();
    
    /* Members */
    scene::ELightModels Type;
    
    dim::matrix4f Matrix;
    video::color Color;
    
    f32 Attn0, Attn1, Attn2;
    f32 InnerConeAngle, OuterConeAngle;
    
    bool Visible;
};


} // /namespace tool

} // /namespace sp


#endif

#endif



// ================================================================================
