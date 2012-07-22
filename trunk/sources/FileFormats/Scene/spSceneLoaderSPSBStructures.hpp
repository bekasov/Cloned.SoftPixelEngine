/*
 * SoftPixel Sandbox Scene structures header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#if defined(_MSC_VER)
#   pragma pack(push, packing)
#   pragma pack(1)
#   define SP_PACK_STRUCT
#elif defined(__GNUC__)
#   define SP_PACK_STRUCT __attribute__((packed))
#else
#   define SP_PACK_STRUCT
#endif

/* === Enumerations === */

enum EFileVersions
{
    SPSB_VERSION_1_00 = 100,
    SPSB_VERSION_1_01 = 101,
    SPSB_VERSION_1_02 = 102,
    SPSB_VERSION_1_03 = 103,
    SPSB_VERSION_1_04 = 104,
    SPSB_VERSION_1_05 = 105,
    SPSB_VERSION_1_06 = 106,
    SPSB_VERSION_1_07 = 107,
    SPSB_VERSION_1_08 = 108,
};

enum EMeshBuildTypes
{
    MESHBUILD_MODIFIED,
    MESHBUILD_BASICMESH,
    MESHBUILD_PROCMESH,
    MESHBUILD_MESHFILE,
};

enum EProceduralMeshes
{
    PROCMESH_UNKNOWN,
    PROCMESH_STAIRCASE,
};

enum EAutoMapModes
{
    AUTOMAP_DISABLE = 0,
    AUTOMAP_LOCAL,
    AUTOMAP_GLOBAL,
};

enum ETextureClassLayerTypes
{
    TEXCLASSLAYER_CUSTOM = 0,
    TEXCLASSLAYER_LIGHTMAP,
    TEXCLASSLAYER_SHADER,
};

enum ECollisionModels
{
    COLLMODEL_NONE = 0,
    COLLMODEL_COLLISION_BOX,
    COLLMODEL_COLLISION_SPHERE,
    COLLMODEL_COLLISION_CAPSULE,
    COLLMODEL_COLLISION_MESH,
    COLLMODEL_DYNAMIC_BOX,
    COLLMODEL_DYNAMIC_CONE,
    COLLMODEL_DYNAMIC_SPHERE,
    COLLMODEL_DYNAMIC_CYLINDER,
    COLLMODEL_DYNAMIC_CHAMFERCYLINDER,
    COLLMODEL_DYNAMIC_CAPSULE,
    COLLMODEL_DYNAMIC_MESH,
    COLLMODEL_STATIC_MESH,
};

enum ESoundFlags
{
    SOUNDFLAG_BACKGROUND    = 0x01,
    SOUNDFLAG_LOOP          = 0x02,
    SOUNDFLAG_STARTUP       = 0x04,
    SOUNDFLAG_UNIQUE        = 0x08,
};

enum EAnimationFlags
{
    ANIMFlAG_USESPLINES = 0x01,
};

/* === Structures === */

struct SLump
{
    u32 Offset; //!< Start offset (in bytes)
    u32 Size;   //!< Size for the whole lump directory (in bytes)
    u32 Count;  //!< Count of elements
};

struct SHeader
{
    s32 Magic;      //!< Magic number "SbSb". You can use (*(int*)"SbSb").
    s32 Version;    //!< Format version: 100, 101 and 102.
    
    SLump SceneConfig;
    
    SLump Meshes;
    SLump Lights;
    SLump Cameras;
    SLump WayPoints;
    
    SLump Textures;
    SLump TextureClasses;
    
    SLump Lightmaps;
    SLump LightmapScene;
    
    SLump Shaders;
    
    SLump BoundVolumes;
    SLump Sounds;
    SLump Sprites;
    SLump AnimNodes;
}
SP_PACK_STRUCT;

struct STextureClassLayer
{
    STextureClassLayer()
        : Type(TEXCLASSLAYER_CUSTOM), Environment(video::TEXENV_MODULATE),
        Mapping(video::MAPGEN_DISABLE), DefaultTexture(0)
    {
    }
    ~STextureClassLayer()
    {
    }
    
    /* Members */
    ETextureClassLayerTypes Type;
    video::ETextureEnvTypes Environment;
    video::EMappingGenTypes Mapping;
    video::Texture* DefaultTexture;
};

struct STextureClass
{
    std::vector<STextureClassLayer> Layers;
};

struct SViewCulling
{
    SViewCulling() : BoundingType(scene::BOUNDING_NONE), SphereRadius(1.0f), BoxSize(1.0f)
    {
    }
    ~SViewCulling()
    {
    }
    
    /* Members */
    scene::EBoundingVolumes BoundingType;
    f32 SphereRadius;
    f32 BoxSize;
};

struct SScriptData
{
    io::stringc TemplateName;
    std::vector<io::stringc> Parameters;
};

struct SBaseObject
{
    s8 Type;
    u32 Id;
    u32 ParentId;
    io::stringc Name;
    bool Visible;
    io::stringc Flags;
    dim::matrix4f Position;
    dim::matrix4f Rotation;
    dim::matrix4f Scaling;
    
    SViewCulling ViewCulling;
    std::vector<SScriptData> ScriptData;
};

struct SWayPoint
{
    SBaseObject BaseObject;
    std::vector<u32> NeighborIdList;
};

struct SMaterial
{
    video::color DiffuseColor;
    video::color AmbientColor;
    video::color SpecularColor;
    video::color EmissionColor;
    
    f32 PolygonOffsetFactor;
    f32 PolygonOffsetUnits;
    
    s8 WireframeFront;
    s8 WireframeBack;
    s8 Shading;
    
    f32 Shininess;
    f32 AlphaReference;
    s8 DepthMethod;
    s8 AlphaMethod;
    s8 BlendSource;
    s8 BlendTarget;
    s8 RenderFace;
    
    s8 ColorMaterial;
    s8 Lighting;
    s8 Blending;
    s8 DepthBuffer;
    s8 Fog;
    s8 PolygonOffset;
    
    s8 CastShadow;
    s8 GetShadow;
};
//SP_PACK_STRUCT;

struct SBasicMesh
{
    s8 BasicMeshType;
    s32 SegsVert;
    s32 SegsHorz;
    f32 InnerRadius;
    f32 OuterRadius;
    s8 HasCap;
    s8 Shading;
    f32 RotationDeg;
    f32 RotationDist;
}
SP_PACK_STRUCT;

struct SProcMeshStaircase
{
    s8 StaricaseType;
    s8 Sides;
    s8 LeftSide;
    s8 RightSide;
    s32 StairsCount;
    f32 Width;
    f32 Height;
    f32 StraightDepth;
    f32 SpiralAngle;
    f32 SpiralRadius;
}
SP_PACK_STRUCT;

struct SHardwareBuffer
{
    s8 VertexFormat;
    s8 IndexFormat;
    s8 TangentVectorLayer;
    s8 BinormalVectorLayer;
    s32 InstanceCount;
}
SP_PACK_STRUCT;

struct SSurfaceLayer
{
    s8 AutoMapMode;
    f32 Density;
    s8 IsCustomTex;
    u32 TexId;
}
SP_PACK_STRUCT;

struct STextureConfig
{
    s8 Format;
    s8 HWFormat;
    s8 Filter;
    s8 MipMapFilter;
    s8 Anisotropy;
    s8 HasMipMaps;
    s8 WrapModeX;
    s8 WrapModeY;
    s8 WrapModeZ;
    video::color ColorKey;
    s8 Dimension;
    s8 RenderTarget;
};
//SP_PACK_STRUCT;

struct SSound
{
    SBaseObject BaseObject;
    io::stringc Filename;
    s8 Flags;
    f32 Volume;
    f32 Speed;
    f32 Balance;
    f32 Radius;
};

struct SSprite
{
    SBaseObject BaseObject;
    video::Texture* Tex;
    video::color Color;
};

struct SShaderRTObject
{
    u32 ShaderClassId;
    u32 TexId;
}
SP_PACK_STRUCT;

struct SParentQueue
{
    scene::SceneNode* Object;
    u32 ParentId;
};

#ifdef _MSC_VER
#   pragma pack(pop, packing)
#endif

#undef SP_PACK_STRUCT



// ================================================================================
