/*
 * Importer file
 * 
 * This file is part of the "SoftPixel Scene Importer/Exporter" (Copyright (c) 2012 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "spsImporter.h"

#include <sstream>


namespace sps
{


/*
 * Internal macros
 */

#define BYTE(m)     m = File_.Read<int8>()
#define SHORT(m)    m = File_.Read<int16>()
#define INT(m)      m = File_.Read<int32>()

#define UBYTE(m)    m = File_.Read<uint8>()
#define USHORT(m)   m = File_.Read<uint16>()
#define UINT(m)     m = File_.Read<uint32>()

#define FLOAT(m)    m = File_.Read<float32>()
#define DOUBLE(m)   m = File_.Read<float64>()

#define VEC2(m)     m = File_.Read<SpVector2>()
#define VEC3(m)     m = File_.Read<SpVector3>()
#define MATRIX(m)   m = File_.Read<SpMatrix4>()
#define COLOR(m)    m = File_.Read<SpColor>()
#define DIM(m)      m = File_.Read<SpDimension>()

#define STR(m)                                                              \
    if (!File_.ReadString(m))                                               \
        return ReturnWithError("String is longer than file", ERROR_FILE);
#define TMPSTR(m)   \
    std::string m;  \
    STR(m);

#define BUFFER(m)   File_.Read(&m, sizeof(m))

#define VERSION(v)      if (AtLeastVersion(FORMATVERSION_##v))
#define NOT_VERSION(v)  if (!AtLeastVersion(FORMATVERSION_##v))

#define FOREACH(t, l, i) \
    for (std::vector<t>::iterator i = l.begin(); i != l.end(); ++i)

#define READSAFE(f) if (!f) return false

#define READLUMP(i)                                                                             \
    if (!ReadLump(Header_.Lumps[i], SpLumpNames[i], SpSceneImporter::ReadObjectProcList[i]))    \
        return ReturnWithError();

#define READLUMPS(s, e)             \
    for (uint32 i = s; i <= e; ++i) \
        READLUMP(i)

#ifdef __SPS_DEBUG__
#   define DEBUG_INFO(m)            \
        {                           \
            std::stringstream SStr; \
            SStr << m;              \
            Debug(SStr.str());      \
        }
#else
#   define DEBUG_INFO(m)
#endif


/*
 * Static functions
 */

template <typename T> static T ConvertString(const std::string &Str)
{
    T Value = T(0);
    std::istringstream SStr(Str);
    SStr >> Value;
    return Value;
}

static int32 RoundPow2(int32 Value)
{
    int32 i;
    
    for (i = 1; i < Value; i <<= 1);
    
    if (i - Value <= Value - i/2)
        return i;
    
    return i/2;
}


/*
 * SpSceneImporter class
 */

const SpSceneImporter::ReadObjectProc SpSceneImporter::ReadObjectProcList[SpSceneFormatHandler::LUMP_COUNT] = {
    &SpSceneImporter::ReadSceneConfig,
    
    &SpSceneImporter::ReadMesh,
    &SpSceneImporter::ReadLight,
    &SpSceneImporter::ReadCamera,
    &SpSceneImporter::ReadWayPoint,
    
    &SpSceneImporter::ReadTexture,
    &SpSceneImporter::ReadTextureClass,
    
    &SpSceneImporter::ReadLightmap,
    &SpSceneImporter::ReadLightmapScene,
    
    &SpSceneImporter::ReadShaderClass,
    
    &SpSceneImporter::ReadBoundVolume,
    &SpSceneImporter::ReadSound,
    &SpSceneImporter::ReadSprite,
    &SpSceneImporter::ReadAnimNode
};

SpSceneImporter::SpSceneImporter() :
    Progress_   (0),
    ProgressMax_(0)
{
}
SpSceneImporter::~SpSceneImporter()
{
}

bool SpSceneImporter::ImportScene(const std::string &Filename)
{
    /* Open file and read header first */
    if (!File_.Open(Filename))
        return ReturnWithError("Could not read file: \"" + Filename + "\"", ERROR_FILE);
    
    if (!ReadHeader())
        return ReturnWithError();
    
    /* Calculate progress maximum */
    ComputeProgressMaximum();
    
    /* Read lump directories */
    READLUMP    (LUMP_SCENECONFIG                       );
    READLUMP    (LUMP_ANIMNODES                         );
    READLUMPS   (LUMP_TEXTURES,     LUMP_TEXTURECLASSES );
    READLUMP    (LUMP_SHADERCLASSES                     );
    READLUMPS   (LUMP_LIGHTMAPS,    LUMP_LIGHTMAPSCENE  );
    READLUMPS   (LUMP_MESHES,       LUMP_WAYPOINTS      );
    READLUMPS   (LUMP_BOUNDVOLUMES, LUMP_SPRITES        );
    
    File_.Close();
    
    return true;
}


/*
 * ======= Protected: =======
 */

#define DEFINE_CATCH_DUMMY(n)                           \
    bool SpSceneImporter::Catch##n(const Sp##n &Unused) \
    {                                                   \
        return true;                                    \
    }

DEFINE_CATCH_DUMMY(Header       )
DEFINE_CATCH_DUMMY(SceneConfig  )
DEFINE_CATCH_DUMMY(Mesh         )
DEFINE_CATCH_DUMMY(Camera       )
DEFINE_CATCH_DUMMY(WayPoint     )
DEFINE_CATCH_DUMMY(Light        )
DEFINE_CATCH_DUMMY(BoundVolume  )
DEFINE_CATCH_DUMMY(Sound        )
DEFINE_CATCH_DUMMY(Sprite       )
DEFINE_CATCH_DUMMY(AnimNode     )
DEFINE_CATCH_DUMMY(Texture      )
DEFINE_CATCH_DUMMY(TextureClass )
DEFINE_CATCH_DUMMY(Lightmap     )
DEFINE_CATCH_DUMMY(LightmapScene)
DEFINE_CATCH_DUMMY(ShaderClass  )

#undef DEFINE_CATCH_DUMMY

bool SpSceneImporter::NotificationLump(const std::string &Name, int32 Progress, int32 MaxProgress)
{
    return true;
}


/*
 * ======= Private: =======
 */

bool SpSceneImporter::ReturnWithError()
{
    File_.Close();
    return false;
}
bool SpSceneImporter::ReturnWithError(const std::string &Message, const EErrorTypes Type)
{
    Error(Message, Type);
    return false;
}

void SpSceneImporter::ComputeProgressMaximum()
{
    Progress_       = 0;
    ProgressMax_    = 0;
    
    for (uint32 i = LUMP_SCENECONFIG; i < LUMP_COUNT; ++i)
        ProgressMax_ += Header_.Lumps[i].Count;
}

bool SpSceneImporter::ReadHeader()
{
    /* Read magic number */
    INT(Header_.Magic);
    
    if (Header_.Magic != SpSceneFormatHandler::GetMagicNumber())
        return ReturnWithError("Wrong magic number (Must be \"SpSb\")", ERROR_HEADER);
    
    /* Read version number */
    INT(Header_.Version);
    
    if (Header_.Version < SpSceneFormatHandler::GetOldestVersion())
    {
        return ReturnWithError(
            "Format version is too old (Oldest supported version is " +
            SpSceneFormatHandler::GetVersionString(SpSceneFormatHandler::GetOldestVersion()) + ")",
            ERROR_HEADER
        );
    }
    if (Header_.Version > SpSceneFormatHandler::GetLatestVersion())
    {
        return ReturnWithError(
            "Format version is unknown (Latest supported version is " +
            SpSceneFormatHandler::GetVersionString(SpSceneFormatHandler::GetLatestVersion()) + ")",
            ERROR_HEADER
        );
    }
    
    /* Read lump directories */
    File_.Read(&Header_.Lumps[0].Offset, sizeof(SpLump)*GetLumpCount());
    
    /* Catch header callback */
    return CatchHeader(Header_);
}

bool SpSceneImporter::ReadLump(const SpLump &Lump, const std::string &Name, ReadObjectProc Proc)
{
    /* Check if lump is used and seek to file offset */
    if (!Lump.IsUsed())
        return true;
    
    File_.Seek(Lump.Offset);
    
    for (uint32 i = 0; i < Lump.Count; ++i)
    {
        /* Call lump procedure and notify progress */
        READSAFE((*this.*Proc)());
        NotificationLump(Name, ++Progress_, ProgressMax_);
    }
    
    return true;
}


/*
 * ======= Private: Main Objects =======
 */

bool SpSceneImporter::ReadSceneConfig()
{
    SpSceneConfig Object;
    
    /* Read scene configuration */
    VERSION(1_03)
    {
        STR(Object.ResourcePath);
        
        VERSION(1_05)
            STR(Object.ScriptTemplateFile);
    }
    
    for (int32 i = 0; i < 6; ++i)
        STR(Object.SkyBoxTexFilenames[i]);
    
    return CatchSceneConfig(Object);
}

bool SpSceneImporter::ReadMesh()
{
    SpMesh Object;
    
    /* Read mesh object */
    READSAFE(ReadBaseObject         (Object.BaseObject  ));
    READSAFE(ReadMeshConstruction   (Object.Construct   ));
    
    VERSION(1_02)
        UINT(Object.ShaderClassId);
    
    READSAFE(ReadMaterial(Object.Material));
    
    /* Read surfaces */
    UINT(uint32 SurfaceCount);
    
    Object.Surfaces.resize(SurfaceCount);
    
    FOREACH(SpSurface, Object.Surfaces, it)
        READSAFE(ReadSurface(*it));
    
    /* Read collision, script and animation */
    VERSION(1_01)
    {
        READSAFE(ReadCollision              (Object.Collision   ));
        READSAFE(ReadAnimScriptBaseObject   (Object             ));
    }
    
    return CatchMesh(Object);
}

bool SpSceneImporter::ReadCamera()
{
    SpCamera Object;
    
    /* Read camera object */
    READSAFE(ReadBaseObject(Object.BaseObject));
    
    VERSION(1_01)
    {
        /* Read camera perspective settings */
        FLOAT(Object.FieldOfView);
        FLOAT(Object.NearPlane  );
        FLOAT(Object.FarPlane   );
        
        /* Read render target texture, script and animation */
        VERSION(1_04)
        {
            UINT(Object.RenderTargetTexId);
            READSAFE(ReadAnimScriptBaseObject(Object));
        }
    }
    
    return CatchCamera(Object);
}

bool SpSceneImporter::ReadWayPoint()
{
    SpWayPoint Object;
    
    /* Read way point object */
    READSAFE(ReadBaseObject(Object.BaseObject));
    
    /* Read neighbor IDs */
    UINT(uint32 NeighborCount);
    
    Object.NeighborIdList.resize(NeighborCount);
    
    FOREACH(uint32, Object.NeighborIdList, it)
        UINT(*it);
    
    return CatchWayPoint(Object);
}

bool SpSceneImporter::ReadLight()
{
    SpLight Object;
    
    /* Read light object */
    READSAFE(ReadBaseObject(Object.BaseObject));
    
    BYTE    (Object.LightModel      );
    BYTE    (Object.Volumetric      );
    FLOAT   (Object.VolumetricRadius);
    
    DEBUG_INFO("Light model = " << static_cast<int32>(Object.LightModel));
    
    VERSION(1_02)
    {
        /* Read color, lightmap and spot-cone settings */
        VERSION(1_04)
        {
            COLOR   (Object.LightColor      );
            BYTE    (Object.UseForLightmaps );
            FLOAT   (Object.InnerSpotCone   );
            FLOAT   (Object.OuterSpotCone   );
        }
        
        DEBUG_INFO("InnerSpotCone = " << Object.InnerSpotCone << ", OuterSpotCone = " << Object.OuterSpotCone)
        
        /* Read shader render-target, script and animation */
        READSAFE(ReadShaderRTObject         (Object.RTObject));
        READSAFE(ReadAnimScriptBaseObject   (Object         ));
    }
    
    return CatchLight(Object);
}

bool SpSceneImporter::ReadBoundVolume()
{
    SpBoundVolume Object;
    
    /* Read bounding volume object, script and animation */
    READSAFE(ReadBaseObject             (Object.BaseObject  ));
    READSAFE(ReadAnimScriptBaseObject   (Object             ));
    
    return CatchBoundVolume(Object);
}

bool SpSceneImporter::ReadSound()
{
    SpSound Object;
    
    /* Read sound object */
    READSAFE(ReadBaseObject(Object.BaseObject));
    
    STR     (Object.Filename);
    BYTE    (Object.Flags   );
    FLOAT   (Object.Volume  );
    FLOAT   (Object.Speed   );
    FLOAT   (Object.Balance );
    FLOAT   (Object.Radius  );
    
    /* Read script and animation */
    READSAFE(ReadAnimScriptBaseObject(Object));
    
    return CatchSound(Object);
}

bool SpSceneImporter::ReadSprite()
{
    SpSprite Object;
    
    /* Read sprite object */
    READSAFE(ReadBaseObject(Object.BaseObject));
    
    UINT    (Object.TexId);
    COLOR   (Object.Color);
    
    /* Read script and animation */
    READSAFE(ReadAnimScriptBaseObject(Object));
    
    return CatchSprite(Object);
}

bool SpSceneImporter::ReadAnimNode()
{
    SpAnimNode Object;
    
    /* Read animation node object */
    READSAFE(ReadBaseObject(Object.BaseObject));
    
    return CatchAnimNode(Object);
}

bool SpSceneImporter::ReadTexture()
{
    SpTexture Object;
    
    /* Read texture object */
    UINT(Object.Id          );
    STR (Object.Filename    );
    
    if (AtLeastVersion(FORMATVERSION_1_02) && Object.Filename.size() == 0)
    {
        DIM(Object.Size);
        
        /* Check if size is valid */
        if (Object.Size.w < 0 || Object.Size.h < 0)
            return ReturnWithError("Texture has invalid size", ERROR_TEXTURE);
    }
    
    /* Read texture configuration */
    File_.Read(&Object.Format, sizeof(int8)*9);
    
    COLOR   (Object.ColorKey    );
    BYTE    (Object.Dimension   );
    BYTE    (Object.RenderTarget);
    
    /* Check some values for correctness */
    if (Object.Dimension < 1 || Object.Dimension > 7)
        return ReturnWithError("Texture has invalid dimension", ERROR_TEXTURE);
    
    return CatchTexture(Object);
}

bool SpSceneImporter::ReadTextureClass()
{
    SpTextureClass Object;
    
    /* Read texture class object */
    UINT(Object.Id  );
    STR (Object.Name);
    
    /* Read texture class layers */
    UBYTE(uint8 LayerCount);
    
    Object.Layers.resize(LayerCount);
    
    FOREACH(SpTextureClassLayer, Object.Layers, it)
        READSAFE(ReadTextureClassLayer(*it));
    
    return CatchTextureClass(Object);
}

bool SpSceneImporter::ReadLightmap()
{
    SpLightmap Object;
    
    /* Read and check lightmap size */
    INT(Object.Size);
    
    if (Object.Size < 8 || RoundPow2(Object.Size) != Object.Size)
        ReturnWithError("Lightmap has invalid size (must be a power of 2 value and greater or equal to 8)", ERROR_LIGHTMAP);
    
    /* Read lightmap image buffer */
    Object.ImageBuffer.resize(Object.Size * Object.Size * 3);
    
    uint8* Buffer   = (&Object.ImageBuffer[0]);
    int32 Size      = Object.Size;
    uint8 BitSet    = 0;
    
    for (int32 y = 0, x; y < Size; ++y)
    {
        for (x = 0; x < Size; ++x)
        {
            /* Read next bit-set */
            if (x % 8 == 0)
                UBYTE(BitSet);
            
            /* Check if current texel has to be loaded in RGB format or not */
            if ((BitSet & 0x80) != 0)
            {
                /* Read RGB color */
                UBYTE(Buffer[0]);
                UBYTE(Buffer[1]);
                UBYTE(Buffer[2]);
            }
            else
            {
                /* Read gray color */
                UBYTE(uint8 Gray);
                
                Buffer[0] = Gray;
                Buffer[1] = Gray;
                Buffer[2] = Gray;
            }
            
            /* Get next bit in set */
            BitSet <<= 1;
            
            /* Get next texel */
            Buffer += 3;
        }
    }
    
    return CatchLightmap(Object);
}

bool SpSceneImporter::ReadLightmapScene()
{
    SpLightmapScene Object;
    
    /* Read lightmap scene object */
    STR(Object.Name);
    
    /* Read lightmap scene surfaces */
    UINT(uint32 SurfaceCount);
    
    Object.Surfaces.resize(SurfaceCount);
    
    FOREACH(SpLightmapSceneSurface, Object.Surfaces, it)
        READSAFE(ReadLightmapSceneSurface(*it));
    
    return CatchLightmapScene(Object);
}

bool SpSceneImporter::ReadShaderClass()
{
    SpShaderClass Object;
    
    /* Read shader class object */
    UINT(Object.Id      );
    STR (Object.Name    );
    BYTE(Object.Flags   );
    
    /* Read shaders */
    for (int32 i = 0; i < 5; ++i)
        READSAFE(ReadShader(Object.Shaders[i]));
    
    return CatchShaderClass(Object);
}


/*
 * ======= Private: Base Objects =======
 */

bool SpSceneImporter::ReadBaseObject(SpBaseObject &Object)
{
    /* Read object type */
    BYTE(Object.Type);
    
    /* Check if type is valid */
    if (Object.Type < 1 || Object.Type > 8)
        return ReturnWithError("Base object has invalid type", ERROR_BASEOBJECT);
    
    /* Read basic information */
    UINT(Object.Id      );
    UINT(Object.ParentId);
    STR (Object.Name    );
    
    DEBUG_INFO("Processing object \"" << Object.Name << "\" with Id " << Object.Id << " ...");
    
    /* Read extended flags */
    VERSION(1_04) BYTE  (Object.Visible );
    VERSION(1_01) STR   (Object.Flags   );
    
    /* Read transformation */
    VERSION(1_07)
    {
        VEC3    (Object.Position);
        MATRIX  (Object.Rotation);
        VEC3    (Object.Scaling );
    }
    else
    {
        SpMatrix4 Pos, Scl;
        
        MATRIX(Pos              );
        MATRIX(Object.Rotation  );
        MATRIX(Scl              );
        
        /* Convert old transformation */
        Object.Position.x = Pos.m[12];
        Object.Position.y = Pos.m[13];
        Object.Position.z = Pos.m[14];
        
        Object.Scaling.x = Scl.m[ 0];
        Object.Scaling.y = Scl.m[ 5];
        Object.Scaling.z = Scl.m[10];
    }
    
    /* Read further extensions */
    VERSION(1_05)
    {
        if (!ReadViewCulling(Object.ViewCulling))
            return false;
        
        NOT_VERSION(1_08)
        {
            if (!ReadScriptData(Object.ScriptData))
                return false;
        }
    }
    
    return true;
}

bool SpSceneImporter::ReadMaterial(SpMaterial &Material)
{
    /* Read material data */
    BUFFER(Material);
    
    /* Check some values for correctness */
    if (Material.WireframeFront < 0 || Material.WireframeFront > 2 || Material.WireframeBack < 0 || Material.WireframeBack > 2)
        return ReturnWithError("Material has invalid wireframe mode", ERROR_MATERIAL);
    if (Material.Shading < 0 || Material.Shading > 2)
        return ReturnWithError("Material has invalid shading mode", ERROR_MATERIAL);
    
    return true;
}

bool SpSceneImporter::ReadViewCulling(SpViewCulling &ViewCulling)
{
    /* Read material data */
    BUFFER(ViewCulling);
    
    /* Check some values for correctness */
    if (ViewCulling.BoundingType < 0 || ViewCulling.BoundingType > 2)
        return ReturnWithError("View culling has invalid bounding type", ERROR_VIEWCULLING);
    
    return true;
}

bool SpSceneImporter::ReadScriptTemplateData(SpScriptTemplateData &ScriptTemplate)
{
    /* Read script template name */
    STR(ScriptTemplate.TemplateName);
    
    if (ScriptTemplate.TemplateName.size() == 0)
        return true;
    
    /* Read script template parameters only if the template has a valid name */
    UINT(uint32 ParamCount);
    
    ScriptTemplate.Parameters.resize(ParamCount);
    
    FOREACH(std::string, ScriptTemplate.Parameters, it)
        STR(*it);
    
    return true;
}

bool SpSceneImporter::ReadScriptData(SpScriptData &ScriptData)
{
    VERSION(1_07)
    {
        /* Read all script templates */
        UINT(uint32 TemplateCount);
        
        ScriptData.ScriptTemplates.resize(TemplateCount);
        
        FOREACH(SpScriptTemplateData, ScriptData.ScriptTemplates, it)
            READSAFE(ReadScriptTemplateData(*it));
    }
    else
    {
        /* Read only one script template */
        ScriptData.ScriptTemplates.resize(1);
        
        READSAFE(ReadScriptTemplateData(*ScriptData.ScriptTemplates.begin()));
    }
    
    return true;
}

bool SpSceneImporter::ReadShaderRTObject(SpShaderRTObject &ShaderRTObject)
{
    /* Read shader render-target object */
    BUFFER(ShaderRTObject);
    return true;
}

bool SpSceneImporter::ReadAnimationKeyframe(SpAnimationKeyframe &Keyframe)
{
    /* Read animation keyframe data */
    BUFFER(Keyframe);
    return true;
}

bool SpSceneImporter::ReadAnimation(SpAnimation &Animation)
{
    /* Read animation data */
    STR     (Animation.Name             );
    BYTE    (Animation.Flags            );
    FLOAT   (Animation.SplineExpansion  );
    
    /* Read animation keyframes */
    UINT(uint32 KeyframeCount);
    
    Animation.Keyframes.resize(KeyframeCount);
    
    FOREACH(SpAnimationKeyframe, Animation.Keyframes, it)
        READSAFE(ReadAnimationKeyframe(*it));
    
    return true;
}

bool SpSceneImporter::ReadAnimationObject(SpAnimationObject &AnimObject)
{
    /* Read all animations */
    UINT(uint32 AnimCount);
    
    AnimObject.Animations.resize(AnimCount);
    
    FOREACH(SpAnimation, AnimObject.Animations, it)
        READSAFE(ReadAnimation(*it));
    
    return true;
}

bool SpSceneImporter::ReadSurfaceLayer(SpSurfaceLayer &SurfaceLayer)
{
    /* Read surface layer data */
    BUFFER(SurfaceLayer);
    return true;
}

bool SpSceneImporter::ReadSurface(SpSurface &Surface)
{
    STR(Surface.Name);
    
    VERSION(1_02)
    {
        /* Read mesh buffer description */
        BYTE(Surface.VertexFormat       );
        BYTE(Surface.IndexFormat        );
        BYTE(Surface.TangentVectorLayer );
        BYTE(Surface.BinormalVectorLayer);
        INT (Surface.InstanceCount      );
        
        /* Check some values for correctness */
        if (Surface.VertexFormat < 0 || Surface.VertexFormat > 3)
            return ReturnWithError("Surface has invalid vertex format", ERROR_SURFACE);
        if (Surface.IndexFormat < 5 || Surface.IndexFormat > 7)
            return ReturnWithError("Surface has invalid index format", ERROR_SURFACE);
        if (Surface.InstanceCount < 0)
            return ReturnWithError("Surface has invalid instance count", ERROR_SURFACE);
    }
    
    /* Read texture layer data */
    UINT(Surface.TexClassId);
    
    BYTE(int8 LayerCount);
    
    if (LayerCount < 0)
        return ReturnWithError("Surface has invalid texture layer count", ERROR_SURFACE);
    
    Surface.Layers.resize(LayerCount);
    
    FOREACH(SpSurfaceLayer, Surface.Layers, it)
    {
        if (!ReadSurfaceLayer(*it))
            return false;
    }
    
    return true;
}

bool SpSceneImporter::ReadCollision(SpCollision &Collision)
{
    /* Read surface layer data */
    BUFFER(Collision);
    
    /* Check some values for correctness */
    if (Collision.CollisionModel < 0)
        return ReturnWithError("Object has invalid collision model", ERROR_COLLISION);
    if (Collision.PickingModel < 0)
        return ReturnWithError("Object has invalid picking model", ERROR_COLLISION);
    
    return true;
}

bool SpSceneImporter::ReadMeshConstructionBasic(SpMeshConstructionBasic &Construct)
{
    /* Read surface layer data */
    BUFFER(Construct);
    
    /* Check some values for correctness */
    if (Construct.BasicMeshType < 0 || Construct.BasicMeshType > 16)
        return ReturnWithError("Unknown basic mesh type", ERROR_CONSTRUCTION);
    
    return true;
}

bool SpSceneImporter::ReadMeshConstructionProcStaircase(SpMeshConstructionProcStaircase &Construct)
{
    /* Read surface layer data */
    BUFFER(Construct);
    
    /* Check some values for correctness */
    if (Construct.StaircaseType < 0 || Construct.StaircaseType > 1)
        return ReturnWithError("Unknown procedural staircase type", ERROR_CONSTRUCTION);
    
    return true;
}

bool SpSceneImporter::ReadMeshConstructionProc(SpMeshConstructionProc &Construct)
{
    BYTE(Construct.ProcMeshType);
    
    switch (Construct.ProcMeshType)
    {
        case 0:     return ReadMeshConstructionProcStaircase(Construct.Staircase);
        default:    return ReturnWithError("Unknown procedural mesh type", ERROR_CONSTRUCTION);
    }
    
    return true;
}

bool SpSceneImporter::ReadMeshConstructionResource(SpMeshConstructionResource &Construct)
{
    STR(Construct.Filename);
    return true;
}

bool SpSceneImporter::ReadMeshConstruction(SpMeshConstruction &Construct)
{
    BYTE(Construct.BuildType);
    
    switch (Construct.BuildType)
    {
        case 0:     break; // Custom mesh is unused
        case 1:     return ReadMeshConstructionBasic    (Construct.BasicMesh    );
        case 2:     return ReadMeshConstructionProc     (Construct.ProcMesh     );
        case 3:     return ReadMeshConstructionResource (Construct.ResourceMesh );
        default:    return ReturnWithError("Unknown mesh construction type", ERROR_CONSTRUCTION);
    }
    
    return true;
}

bool SpSceneImporter::ReadAnimScriptBaseObject(SpAnimScriptBaseObject &Object)
{
    VERSION(1_08)
    {
        READSAFE(ReadScriptData     (Object.ScriptData));
        READSAFE(ReadAnimationObject(Object.AnimObject));
    }
    return true;
}

bool SpSceneImporter::ReadTextureClassLayer(SpTextureClassLayer &TexLayer)
{
    /* Read texture class layer */
    STR (TexLayer.Name          );
    BYTE(TexLayer.Type          );
    BYTE(TexLayer.Environment   );
    BYTE(TexLayer.MappingGen    );
    UINT(TexLayer.DefaultTexId  );
    
    /* Check some values for correctness */
    if (TexLayer.Type < 0 || TexLayer.Type > 2)
        return ReturnWithError("Texture class layer has invalid type", ERROR_TEXTURE);
    
    return true;
}

bool SpSceneImporter::ReadLightmapSceneVertex(SpLightmapSceneVertex &Vertex, uint8 LayerCount)
{
    /* Read vertex coordinate, normal and lightmap texture-coordinate */
    VEC3(Vertex.Coordinate      );
    VEC3(Vertex.Normal          );
    VEC2(Vertex.LightmapTexCoord);
    
    /* Read base texture-coordinates */
    for (uint8 i = 0; i < LayerCount; ++i)
        VEC2(Vertex.TexCoords[i]);
    
    return true;
}

bool SpSceneImporter::ReadLightmapSceneLayer(SpLightmapSceneLayer &Layer)
{
    BUFFER(Layer);
    return true;
}

bool SpSceneImporter::ReadLightmapSceneSurface(SpLightmapSceneSurface &Surface)
{
    /* Read lightmap scene surface */
    STR (Surface.Name           );
    BYTE(Surface.VertexFormat   );
    BYTE(Surface.IndexFormat    );
    
    /* Read and check surface layer */
    UBYTE(uint8 LayerCount);
    
    if (LayerCount > 7)
        return ReturnWithError("Lightmap scene surface layer count is invalid (must be smaller than 8)", ERROR_LIGHTMAP);
    
    /* Read surface layers */
    UINT(Surface.LightmapTexIndex);
    
    Surface.Layers.resize(LayerCount);
    
    FOREACH(SpLightmapSceneLayer, Surface.Layers, it)
        READSAFE(ReadLightmapSceneLayer(*it));
    
    /* Read surface vertices */
    UINT(uint32 VertexCount);
    
    Surface.Vertices.resize(VertexCount);
    
    FOREACH(SpLightmapSceneVertex, Surface.Vertices, it)
        READSAFE(ReadLightmapSceneVertex(*it, LayerCount));
    
    return true;
}

bool SpSceneImporter::ReadShader(SpShader &Shader)
{
    /* Read shader classification */
    BYTE(Shader.Version     );
    STR (Shader.EntryPoint  );
    STR (Shader.ShaderCode  );
    
    /* Read shader parameters */
    UINT(uint32 ParamCount);
    
    Shader.Parameters.resize(ParamCount);
    
    FOREACH(SpShaderParameter, Shader.Parameters, it)
        READSAFE(ReadShaderParameter(*it));
    
    return true;
}

bool SpSceneImporter::ReadShaderParameter(SpShaderParameter &Param)
{
    /* Read shader parameter */
    STR     (Param.Name );
    TMPSTR  (Value      );
    BYTE    (Param.Flags);
    
    /* Setup parameter value */
    if (Value.size() < 2 || Value[1] != ':')
        return true;
    
    switch (Value[0])
    {
        case 'f':
        {
            Param.Type      = SHADERPARAM_FLOAT;
            Param.ValueFlt  = ConvertString<float32>(Value.substr(2));
        }
        break;
        
        case 'i':
        {
            Param.Type      = SHADERPARAM_INT;
            Param.ValueInt  = ConvertString<int32>(Value.substr(2));
        }
        break;
        
        case 'b':
        {
            Param.Type      = SHADERPARAM_BOOL;
            Param.ValueBool = (ConvertString<int32>(Value.substr(2)) != 0);
        }
        break;
        
        case 'v':
        {
            /* Find all vector elements */
            int32 Count = 0;
            size_t Pos1 = 2, Pos2 = 0;
            
            float32 Vec[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
            
            while ( ( Pos2 = Value.find(';', Pos1) ) != std::string::npos && Count < 4 )
            {
                Vec[Count++] = ConvertString<float32>(Value.substr(Pos1, Pos2 - Pos1));
                Pos1 = Pos2 + 1;
            }
            
            /* Setup vector elements */
            switch (Count)
            {
                case 2:
                    Param.Type          = SHADERPARAM_VEC2;
                    Param.ValueVec2.x   = Vec[0];
                    Param.ValueVec2.y   = Vec[1];
                    break;
                    
                case 3:
                    Param.Type          = SHADERPARAM_VEC3;
                    Param.ValueVec3.x   = Vec[0];
                    Param.ValueVec3.y   = Vec[1];
                    Param.ValueVec3.z   = Vec[2];
                    break;
                    
                case 4:
                    Param.Type          = SHADERPARAM_VEC4;
                    Param.ValueVec4.x   = Vec[0];
                    Param.ValueVec4.y   = Vec[1];
                    Param.ValueVec4.z   = Vec[2];
                    Param.ValueVec4.w   = Vec[3];
                    break;
                    
                default:
                    Warning("Shader parameter vector has unsupported count of elements (only 2, 3 or 4 are supported)", WARNING_SHADER);
                    break;
            }
        }
        break;
        
        case 'm':
        {
            Param.Type          = SHADERPARAM_MATRIX;
            Param.ValueMatrix   = Value.substr(2);
        }
        break;
    }
    
    return true;
}


/*
 * Disable internal macros
 */

#undef BYTE
#undef SHORT
#undef INT

#undef UBYTE
#undef USHORT
#undef UINT

#undef FLOAT
#undef DOUBLE

#undef VEC2
#undef VEC3
#undef MATRIX
#undef COLOR
#undef DIM

#undef STR
#undef TMPSTR
#undef BUFFER

#undef VERSION
#undef NOT_VERSION

#undef FOREACH
#undef READSAFE
#undef READLUMP
#undef READLUMPS


} // /namespace sps



// ================================================================================
