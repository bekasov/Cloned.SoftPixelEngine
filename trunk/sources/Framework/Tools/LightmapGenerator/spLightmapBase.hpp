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

//! Lightmap generation option masks.
enum ELightmapGenerationsFlags
{
    LIGHTMAPFLAG_NOCOLORS           = 0x00000001, //!< Colored lighting is disabled. When all lights have the diffuse color (255, 255, 255) this flag has no effect.
    LIGHTMAPFLAG_NOTRANSPARENCY     = 0x00000002, //!< Transparency textures ray-casting is disabled. This may occur in much faster lightmap generation.
    LIGHTMAPFLAG_GPU_ACCELERATION   = 0x00000004, //!< Since version 3.3 hardware acclerated lightmap generation is supported. This requires that Direct3D 11 or OpenGL 4.3 is used.
    LIGHTMAPFLAG_RADIOSITY          = 0x00000008, //!< Since version 3.3 radiosity lightmap generation is supported. This requires that the 'LIGHTMAPFLAG_GPU_ACCELERATION' flag is also enabled.
};

//! States of the lightmap generation process.
enum ELightmapGenerationStates
{
    LIGHTMAPSTATE_INITIALIZING, //!< Initialization state. Occrus at start up.
    LIGHTMAPSTATE_PARTITIONING, //!< Scene partitioning state. Occurs when the scene will be partitioned.
    LIGHTMAPSTATE_SHADING,      //!< Lightmap texel generation state. Occurs every time a new light source is used for lightmap texel generation (on the CPU) or a new lightmap is shaded (on the GPU).
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

/**
Lightmap generation configuration structure.
\since Version 3.3
*/
struct SP_EXPORT SLightmapGenConfig
{
    SLightmapGenConfig(
        const video::color &Ambient = DEF_LIGHTMAP_AMBIENT,
        const u32 MaxSize = DEF_LIGHTMAP_SIZE,
        const f32 Density = DEF_LIGHTMAP_DENSITY,
        const u8 BlurRadius = DEF_LIGHTMAP_BLURRADIUS
    );
    ~SLightmapGenConfig();
    
    /* === Members === */
    
    /**
    Specifies the darkest color for each lightmap texel. Normally (with radiosity) those parameters are not used
    because the light calculations are physicially correct and the light rays can arrive into dark rooms where no light
    sources is placed. But in this case (with just simple-shadows) the texels are complete black when it has no direct
    connection to the light source. This happens when a cast-shadow-object stands in the texel's way. So this
    parameter makes sure that a texel is not darker than AmbientColor. It is an adjustment.
    */
    video::color AmbientColor;
    /**
    Specifies the size of each lightmap texture. By default 512. Consider that the lightmap textures should not
    be too large and not too small. If they are too large the memory costs are higher because the MIP-map sub-textures
    are also generated. If they are too small more surfaces must be created for the resulting 3d model which makes
    the application slower.
    */
    u32 MaxLightmapSize;
    /**
    Density specifies the scaling factor for generating the texture space from world space.
    By this procedure larger triangle faces get larger texture faces in the lightmap. When a calculated texture face is
    larger then "MaxLightmapSize" it is clamped to it.
    */
    f32 DefaultDensity;
    /**
    Specifies the blur factor for the lightmaps. This is also a balance for none-radiosity technic.
    This reduces hard-shadows and changes it to soft-shadows. The lightmap textures are not only blured completly.
    This technic is a little bit more complicate but can however cause some unbeautiful areas on the lightmaps.
    Mostly in the edges of a room. Be default 2 which causes a nice smooth light scene. If the blur factor is 0
    no bluring computations will proceeded.
    */
    u8 TexelBlurRadius;
};


} // /namespace tool

} // /namespace sp


#endif

#endif



// ================================================================================
