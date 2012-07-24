/*
 * Format structures header
 * 
 * This file is part of the "SoftPixel Scene Importer/Exporter" (Copyright (c) 2012 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef __SPS_FORMAT_STRUCTURES_H__
#define __SPS_FORMAT_STRUCTURES_H__


#if defined(_MSC_VER)
#   pragma pack(push, packing)
#   pragma pack(1)
#   define SPS_PACK_STRUCT
#elif defined(__GNUC__)
#   define SPS_PACK_STRUCT __attribute__((packed))
#else
#   define SPS_PACK_STRUCT
#endif


//! Lump directory.
struct SpLump
{
    SpLump() :
        Offset  (0),
        Size    (0),
        Count   (0)
    {
    }
    ~SpLump()
    {
    }
    
    /* Inline functions */
    inline bool IsUsed() const
    {
        return Count > 0;
    }
    
    /* Members */
    uint32 Offset;  //!< Lump offset (in bytes).
    uint32 Size;    //!< Lump size (in bytes).
    uint32 Count;   //!< Entry count.
}
SPS_PACK_STRUCT;

//! Format header.
struct SpHeader
{
    SpHeader() :
        Magic   (0),
        Version (0)
    {
    }
    ~SpHeader()
    {
    }
    
    /* Members */
    int32 Magic;                //!< Magic number ("SpSb").
    int32 Version;              //!< Version number (100 -> v.1.00, 101 -> v.1.01 etc.).
    
    SpLump Lumps[LUMP_COUNT];   //!< Lump directories.
}
SPS_PACK_STRUCT;

//! Scene configuration.
struct SpSceneConfig
{
    SpSceneConfig()
    {
    }
    ~SpSceneConfig()
    {
    }
    
    /* Members */
    std::string ResourcePath;           //!< Global resource path for each texture- and mesh file. \since Version 1.03
    std::string ScriptTemplateFile;     //!< Script template filename. \since Version 1.05
    std::string SkyBoxTexFilenames[6];  //!< Sky box texture filenames: North, South, Top, Bottom, West, East.
}
SPS_PACK_STRUCT;

//! View culling. \since Version 1.02
struct SpViewCulling
{
    SpViewCulling() :
        BoundingType(0      ),
        SphereRadius(0.0f   ),
        BoxSize     (0.0f   )
    {
    }
    ~SpViewCulling()
    {
    }
    
    /* Members */
    int8 BoundingType;      //!< 0 -> None, 1 -> Sphere, 2 -> Box.
    float32 SphereRadius;   //!< Bounding sphere radius.
    float32 BoxSize;        //!< Bounding box scaling factor. By default 1.0.
}
SPS_PACK_STRUCT;

//! Shader render target object. \since Version 1.02
struct SpShaderRTObject
{
    SpShaderRTObject() :
        ShaderClassId   (0),
        TexId           (0)
    {
    }
    ~SpShaderRTObject()
    {
    }
    
    /* Members */
    uint32 ShaderClassId;   //!< Shader class ID number.
    uint32 TexId;           //!< Texture ID number.
}
SPS_PACK_STRUCT;

//! Script template data. \since Version 1.05
struct SpScriptTemplateData
{
    SpScriptTemplateData()
    {
    }
    ~SpScriptTemplateData()
    {
    }
    
    /* Members */
    std::string TemplateName;               //!< Template name.
    std::vector<std::string> Parameters;    //!< Parameter values.
}
SPS_PACK_STRUCT;

//! Script data. \since Version 1.05
struct SpScriptData
{
    SpScriptData()
    {
    }
    ~SpScriptData()
    {
    }
    
    /* Members */
    std::vector<SpScriptTemplateData> ScriptTemplates; //!< Before version 1.07 this has always only one element.
}
SPS_PACK_STRUCT;

//! Base object with ID, name, flags and transformation.
struct SpBaseObject
{
    SpBaseObject() :
        Type    (0),
        Id      (0),
        ParentId(0),
        Visible (0)
    {
    }
    ~SpBaseObject()
    {
    }
    
    /* Members */
    
    /**
    Object type: 1 -> Mesh, 2 -> Light, 3 -> Sound, 4 -> Camera, 5 -> Way-Point,
    6 -> Bounding-Volume, 7 -> Sprite, 8 -> Animation-Node.
    */
    int8 Type;
    
    uint32 Id;                  //!< Object ID number.
    uint32 ParentId;            //!< Parent object ID number. By default 0 which means the object has no parent.
    std::string Name;           //!< Object name.
    
    int8 Visible;               //!< Visible flag: 0 -> Hidden, 1 -> Visible. \since Version 1.04
    std::string Flags;          //!< Flags string. \since Version 1.01
    
    SpVector3 Position;         //!< Global position.
    SpMatrix4 Rotation;         //!< Global rotation.
    SpVector3 Scaling;          //!< Global scaling.
    
    SpViewCulling ViewCulling;  //!< View culling data. \since Version 1.05
    SpScriptData ScriptData;    //!< Script data. Since version 1.08 only used in each individual object which is a script object, too.
}
SPS_PACK_STRUCT;

//! Mesh material.
struct SpMaterial
{
    SpMaterial() :
        PolygonOffsetFactor (0.0f   ),
        PolygonOffsetUnits  (0.0f   ),
        WireframeFront      (0      ),
        WireframeBack       (0      ),
        Shading             (0      ),
        Shininess           (0.0f   ),
        AlphaReference      (0.0f   ),
        DepthMethod         (0      ),
        AlphaMethod         (0      ),
        BlendSource         (0      ),
        BlendTarget         (0      ),
        RenderFace          (0      ),
        ColorMaterial       (0      ),
        Lighting            (0      ),
        Blending            (0      ),
        DepthBuffer         (0      ),
        Fog                 (0      ),
        PolygonOffset       (0      ),
        CastShadow          (0      ),
        GetShadow           (0      )
    {
    }
    ~SpMaterial()
    {
    }
    
    /* Members */
    SpColor DiffuseColor;
    SpColor AmbientColor;
    SpColor SpecularColor;
    SpColor EmissionColor;
    
    float32 PolygonOffsetFactor;
    float32 PolygonOffsetUnits;
    
    int8 WireframeFront;    //!< 0 -> Solid, 1 -> Lines, 2 -> Points.
    int8 WireframeBack;     //!< 0 -> Solid, 1 -> Lines, 2 -> Points.
    int8 Shading;           //!< 0 -> Flat-Shading, 1 -> Gouraud-Shading, 2 -> Original-Shading.
    
    float32 Shininess;
    float32 AlphaReference;
    int8 DepthMethod;
    int8 AlphaMethod;
    int8 BlendSource;
    int8 BlendTarget;
    int8 RenderFace;
    
    int8 ColorMaterial;
    int8 Lighting;
    int8 Blending;
    int8 DepthBuffer;
    int8 Fog;
    int8 PolygonOffset;
    
    int8 CastShadow;
    int8 GetShadow;
}
SPS_PACK_STRUCT;

struct SpAnimationKeyframe
{
    SpAnimationKeyframe() :
        AnimNodeId  (0),
        Duration    (0)
    {
    }
    ~SpAnimationKeyframe()
    {
    }
    
    /* Members */
    uint32 AnimNodeId;
    uint32 Duration;
}
SPS_PACK_STRUCT;

struct SpAnimation
{
    SpAnimation() :
        Flags           (0      ),
        SplineExpansion (0.0f   )
    {
    }
    ~SpAnimation()
    {
    }
    
    /* Members */
    std::string Name;
    int8 Flags;
    float32 SplineExpansion;
    
    std::vector<SpAnimationKeyframe> Keyframes;
}
SPS_PACK_STRUCT;

struct SpAnimationObject
{
    SpAnimationObject()
    {
    }
    ~SpAnimationObject()
    {
    }
    
    /* Members */
    std::vector<SpAnimation> Animations;
}
SPS_PACK_STRUCT;

struct SpAnimScriptBaseObject
{
    SpAnimScriptBaseObject()
    {
    }
    virtual ~SpAnimScriptBaseObject()
    {
    }
    
    /* Members */
    SpBaseObject BaseObject;
    SpScriptData ScriptData;        //!< Script data. \since Version 1.08
    SpAnimationObject AnimObject;   //!< Animation object. \since Version 1.08.
}
SPS_PACK_STRUCT;

//! Mesh surface layer.
struct SpSurfaceLayer
{
    SpSurfaceLayer() :
        AutoMapMode (0      ),
        Density     (0.0f   ),
        IsCustomTex (0      ),
        TexId       (0      )
    {
    }
    ~SpSurfaceLayer()
    {
    }
    
    /* Members */
    int8 AutoMapMode;
    float32 Density;
    int8 IsCustomTex;
    uint32 TexId;       //!< Texture ID number.
}
SPS_PACK_STRUCT;

//! Mesh surface.
struct SpSurface
{
    SpSurface() :
        VertexFormat        (0  ),
        IndexFormat         (1  ),
        TangentVectorLayer  (-1 ),
        BinormalVectorLayer (-1 ),
        InstanceCount       (1  ),
        TexClassId          (0  )
    {
    }
    ~SpSurface()
    {
    }
    
    /* Members */
    std::string Name;           //!< Surface name.
    
    int8 VertexFormat;          //!< Vertex buffer format. \since Version 1.02
    int8 IndexFormat;           //!< Index buffer format: 0 -> 8 bit, 1 -> 16 bit, 2 -> 32 bit. \since Version 1.02
    int8 TangentVectorLayer;    //!< -1 means disabled. \since Version 1.02
    int8 BinormalVectorLayer;   //!< -1 means disabled. \since Version 1.02
    int32 InstanceCount;        //!< Count of hardware instances. \since Version 1.02
    
    uint32 TexClassId;
    
    std::vector<SpSurfaceLayer> Layers;
}
SPS_PACK_STRUCT;

struct SpCollision
{
    SpCollision() :
        CollisionModel  (0),
        PickingModel    (0)
    {
    }
    ~SpCollision()
    {
    }
    
    /* Members */
    int8 CollisionModel;
    int8 PickingModel;
}
SPS_PACK_STRUCT;

//! Basic mesh construction.
struct SpMeshConstructionBasic
{
    SpMeshConstructionBasic() :
        BasicMeshType   (0      ),
        SegsVert        (0      ),
        SegsHorz        (0      ),
        InnerRadius     (0.0f   ),
        OuterRadius     (0.0f   ),
        HasCap          (0      ),
        Shading         (0      ),
        RotationDeg     (0.0f   ),
        RotationDist    (0.0f   )
    {
    }
    ~SpMeshConstructionBasic()
    {
    }
    
    /* Members */
    int8 BasicMeshType;
    int32 SegsVert;
    int32 SegsHorz;
    float32 InnerRadius;
    float32 OuterRadius;
    int8 HasCap;
    int8 Shading;
    float32 RotationDeg;
    float32 RotationDist;
}
SPS_PACK_STRUCT;

//! Procedural staircase mesh construction.
struct SpMeshConstructionProcStaircase
{
    SpMeshConstructionProcStaircase() :
        StaircaseType   (0      ),
        Sides           (0      ),
        LeftSide        (0      ),
        RightSide       (0      ),
        StairsCount     (0      ),
        Width           (0.0f   ),
        Height          (0.0f   ),
        StraightDepth   (0.0f   ),
        SpiralAngle     (0.0f   ),
        SpiralRadius    (0.0f   )
    {
    }
    ~SpMeshConstructionProcStaircase()
    {
    }
    
    /* Members */
    int8 StaircaseType;
    int8 Sides;
    int8 LeftSide;
    int8 RightSide;
    int32 StairsCount;
    float32 Width;
    float32 Height;
    float32 StraightDepth;
    float32 SpiralAngle;
    float32 SpiralRadius;
}
SPS_PACK_STRUCT;

//! Procedural mesh construction.
struct SpMeshConstructionProc
{
    SpMeshConstructionProc() :
        ProcMeshType(0)
    {
    }
    ~SpMeshConstructionProc()
    {
    }
    
    /* Members */
    int8 ProcMeshType;                          //!< 0 -> Staircase.
    
    SpMeshConstructionProcStaircase Staircase;
}
SPS_PACK_STRUCT;

//! Resource mesh construction.
struct SpMeshConstructionResource
{
    SpMeshConstructionResource()
    {
    }
    ~SpMeshConstructionResource()
    {
    }
    
    /* Members */
    std::string Filename;
}
SPS_PACK_STRUCT;

struct SpMeshConstruction
{
    SpMeshConstruction() :
        BuildType(0)
    {
    }
    ~SpMeshConstruction()
    {
    }
    
    /* Members */
    int8 BuildType; //!< Mesh construction build type: 0 -> Modified Mesh, 1 -> Basic-Mesh, 2 -> Procedural-Mesh, 3 -> Resource-Mesh.
    
    SpMeshConstructionBasic BasicMesh;
    SpMeshConstructionProc ProcMesh;
    SpMeshConstructionResource ResourceMesh;
}
SPS_PACK_STRUCT;

//! Geometry mesh object.
struct SpMesh : public SpAnimScriptBaseObject
{
    SpMesh() :
        SpAnimScriptBaseObject  (   ),
        ShaderClassId           (0  )
    {
    }
    ~SpMesh()
    {
    }
    
    /* Members */
    SpMeshConstruction Construct;
    SpMaterial Material;
    SpCollision Collision;              //!< Collision data. \since Versionn 1.01
    
    std::vector<SpSurface> Surfaces;
    
    uint32 ShaderClassId;               //!< Shader class ID number. \since Version 1.02
}
SPS_PACK_STRUCT;

struct SpCamera : public SpAnimScriptBaseObject
{
    SpCamera() :
        SpAnimScriptBaseObject  (       ),
        FieldOfView             (0.0f   ),
        NearPlane               (0.0f   ),
        FarPlane                (0.0f   ),
        RenderTargetTexId       (0      )
    {
    }
    ~SpCamera()
    {
    }
    
    /* Members */
    float32 FieldOfView;            //!< Field of view angle. \since Version 1.01
    float32 NearPlane;              //!< Near clipping plane. \since Version 1.01
    float32 FarPlane;               //!< Far clipping plane. \since Version 1.01
    
    uint32 RenderTargetTexId;       //!< Render target texture ID number. \since Version 1.04
}
SPS_PACK_STRUCT;

struct SpWayPoint
{
    SpWayPoint()
    {
    }
    ~SpWayPoint()
    {
    }
    
    /* Members */
    SpBaseObject BaseObject;
    
    std::vector<uint32> NeighborIdList;
}
SPS_PACK_STRUCT;

struct SpLight : public SpAnimScriptBaseObject
{
    SpLight() :
        SpAnimScriptBaseObject  (       ),
        LightModel              (0      ),
        Volumetric              (0      ),
        VolumetricRadius        (0.0f   ),
        UseForLightmaps         (0      ),
        InnerSpotCone           (30.0f  ),
        OuterSpotCone           (60.0f  )
    {
        LightColor.r = 200;
        LightColor.g = 200;
        LightColor.b = 200;
    }
    ~SpLight()
    {
    }
    
    /* Members */
    SpShaderRTObject RTObject;      //!< Shader render target object. \since Version 1.02
    
    int8 LightModel;
    int8 Volumetric;
    float32 VolumetricRadius;
    
    SpColor LightColor;             //!< Light color. \since Version 1.04
    int8 UseForLightmaps;           //!< Usage for lightmaps. \since Version 1.04
    float32 InnerSpotCone;          //!< Outer spot light cone. \since Version 1.04
    float32 OuterSpotCone;          //!< Inner spot light cone. \since Version 1.04
}
SPS_PACK_STRUCT;

struct SpBoundVolume : public SpAnimScriptBaseObject
{
    SpBoundVolume() :
        SpAnimScriptBaseObject()
    {
    }
    ~SpBoundVolume()
    {
    }
}
SPS_PACK_STRUCT;

struct SpSound : public SpAnimScriptBaseObject
{
    SpSound() :
        SpAnimScriptBaseObject  (       ),
        Flags                   (0      ),
        Volume                  (0.0f   ),
        Speed                   (0.0f   ),
        Balance                 (0.0f   ),
        Radius                  (0.0f   )
    {
    }
    ~SpSound()
    {
    }
    
    /* Members */
    std::string Filename;
    
    /**
    Flag options:
    0x01 -> Background (none 3D sound),
    0x02 -> Loop (endless sound),
    0x04 -> Play at start up,
    0x08 -> Unique sound (referencing to previously loaded sounds is disabled).
    */
    int8 Flags;
    
    float32 Volume;
    float32 Speed;
    float32 Balance;
    float32 Radius;                 //!< Volumetric radius.
}
SPS_PACK_STRUCT;

struct SpSprite : public SpAnimScriptBaseObject
{
    SpSprite() :
        SpAnimScriptBaseObject  (   ),
        TexId                   (0  )
    {
    }
    ~SpSprite()
    {
    }
    
    /* Members */
    uint32 TexId;
    SpColor Color;
}
SPS_PACK_STRUCT;

struct SpAnimNode
{
    SpAnimNode()
    {
    }
    ~SpAnimNode()
    {
    }
    
    /* Members */
    SpBaseObject BaseObject;
}
SPS_PACK_STRUCT;

struct SpTexture
{
    SpTexture() :
        Id          (0),
        Format      (0),
        HWFormat    (0),
        Filter      (0),
        MipMapFilter(0),
        Anisotropy  (0),
        HasMipMaps  (0),
        WrapModeX   (0),
        WrapModeY   (0),
        WrapModeZ   (0),
        Dimension   (0),
        RenderTarget(0)
    {
        Size.w = 1;
        Size.h = 1;
    }
    ~SpTexture()
    {
    }
    
    /* Members */
    uint32 Id;
    std::string Filename;
    
    SpDimension Size;
    SpColor ColorKey;
    
    int8 Format;
    int8 HWFormat;
    int8 Filter;
    int8 MipMapFilter;
    int8 Anisotropy;
    int8 HasMipMaps;
    int8 WrapModeX;
    int8 WrapModeY;
    int8 WrapModeZ;
    int8 Dimension;
    int8 RenderTarget;
}
SPS_PACK_STRUCT;

struct SpTextureClassLayer
{
    SpTextureClassLayer() :
        Type        (0),
        Environment (0),
        MappingGen  (0),
        DefaultTexId(0)
    {
    }
    ~SpTextureClassLayer()
    {
    }
    
    /* Members */
    std::string Name;
    int8 Type;
    int8 Environment;
    int8 MappingGen;
    uint32 DefaultTexId;
}
SPS_PACK_STRUCT;

struct SpTextureClass
{
    SpTextureClass() :
        Id(0)
    {
    }
    ~SpTextureClass()
    {
    }
    
    /* Members */
    uint32 Id;
    std::string Name;
    
    std::vector<SpTextureClassLayer> Layers;
}
SPS_PACK_STRUCT;

struct SpLightmap
{
    SpLightmap() :
        Size(0)
    {
    }
    ~SpLightmap()
    {
    }
    
    /* Members */
    int32 Size;
    std::vector<uint8> ImageBuffer; //!< Image buffer with RGB color. Buffer size is (SpLightmap.Size * SpLightmap.Size * 3) Bytes.
}
SPS_PACK_STRUCT;

struct SpLightmapSceneVertex
{
    SpLightmapSceneVertex()
    {
    }
    ~SpLightmapSceneVertex()
    {
    }
    
    /* Members */
    SpVector3 Coordinate;
    SpVector3 Normal;
    SpVector2 LightmapTexCoord;
    SpVector2 TexCoords[7];     //!< Maximum number of texture-coordinate layers is 7. There are not always needed all.
}
SPS_PACK_STRUCT;

struct SpLightmapSceneLayer
{
    SpLightmapSceneLayer() :
        TexId       (0),
        Environment (0),
        MappingGen  (0)
    {
    }
    ~SpLightmapSceneLayer()
    {
    }
    
    /* Members */
    uint32 TexId;
    int8 Environment;
    int8 MappingGen;
}
SPS_PACK_STRUCT;

struct SpLightmapSceneSurface
{
    SpLightmapSceneSurface() :
        VertexFormat    (0),
        IndexFormat     (0),
        LightmapTexIndex(0)
    {
    }
    ~SpLightmapSceneSurface()
    {
    }
    
    /* Members */
    std::string Name;
    
    int8 VertexFormat;
    int8 IndexFormat;
    
    uint32 LightmapTexIndex;
    
    std::vector<SpLightmapSceneLayer> Layers;
    std::vector<SpLightmapSceneVertex> Vertices;
}
SPS_PACK_STRUCT;

struct SpLightmapScene
{
    SpLightmapScene()
    {
    }
    ~SpLightmapScene()
    {
    }
    
    /* Members */
    std::string Name;
    std::vector<SpLightmapSceneSurface> Surfaces;
}
SPS_PACK_STRUCT;

struct SpShaderParameter
{
    SpShaderParameter() :
        Flags       (0                  ),
        Type        (SHADERPARAM_UNKNOWN),
        ValueFlt    (0.0f               ),
        ValueInt    (0                  ),
        ValueBool   (false              )
    {
    }
    ~SpShaderParameter()
    {
    }
    
    /* Members */
    std::string Name;
    int8 Flags;
    
    EShaderParamTypes Type;
    
    float32 ValueFlt;
    int32 ValueInt;
    bool ValueBool;
    SpVector2 ValueVec2;
    SpVector3 ValueVec3;
    SpVector4 ValueVec4;
    
    /**
    Shader matrix parameters are stored as strings. The following strings are accepted:
    - $(IdentityMatrix)
    - $(CameraMatrix)
    - $(WorldMatrix)
    - $(ViewMatrix)
    - $(ProjectionMatrix)
    - $(WorldViewMatrix)
    - $(ViewProjectionMatrix)
    - $(WorldViewProjectionMatrix)
    - $(CameraMatrixInv)
    - $(WorldMatrixInv)
    - $(ViewMatrixInv)
    - $(ProjectionMatrixInv)
    - $(WorldViewMatrixInv)
    - $(ViewProjectionMatrixInv)
    - $(WorldViewProjectionMatrixInv)
    "Inv" indicates an inverse matrix.
    */
    std::string ValueMatrix;
}
SPS_PACK_STRUCT;

struct SpShader
{
    SpShader() :
        Version(0)
    {
    }
    ~SpShader()
    {
    }
    
    /* Members */
    int8 Version;
    std::string EntryPoint;
    std::string ShaderCode;
    
    std::vector<SpShaderParameter> Parameters;
}
SPS_PACK_STRUCT;

struct SpShaderClass
{
    SpShaderClass() :
        Id      (0),
        Flags   (0)
    {
    }
    ~SpShaderClass()
    {
    }
    
    /* Members */
    uint32 Id;
    std::string Name;
    int8 Flags;
    
    SpShader Shaders[5];
}
SPS_PACK_STRUCT;


#ifdef _MSC_VER
#   pragma pack(pop, packing)
#endif

#undef SPS_PACK_STRUCT


#endif



// ================================================================================
