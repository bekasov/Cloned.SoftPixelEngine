/*
 * Exporter file
 * 
 * This file is part of the "SoftPixel Scene Importer/Exporter" (Copyright (c) 2012 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "spsExporter.h"

#include <sstream>


namespace sps
{


/*
 * Internal macros
 */

#define BYTE(m)     File_.Write<int8>(static_cast<int8>(m))
#define SHORT(m)    File_.Write<int16>(static_cast<int16>(m))
#define INT(m)      File_.Write<int32>(static_cast<int32>(m))

#define UBYTE(m)    File_.Write<uint8>(static_cast<uint8>(m))
#define USHORT(m)   File_.Write<uint16>(static_cast<uint16>(m))
#define UINT(m)     File_.Write<uint32>(static_cast<uint32>(m))

#define FLOAT(m)    File_.Write<float32>(static_cast<float32>(m))
#define DOUBLE(m)   File_.Write<float64>(static_cast<float64>(m))

#define VEC2(m)     File_.Write<SpVector2>(m)
#define VEC3(m)     File_.Write<SpVector3>(m)
#define MATRIX(m)   File_.Write<SpMatrix4>(m)
#define COLOR(m)    File_.Write<SpColor>(m)
#define DIM(m)      File_.Write<SpDimension>(m)

#define STR(m)      File_.WriteString(m)
#define BUFFER(m)   File_.Write(&m, sizeof(m))

#define VERSION(v)      if (AtLeastVersion(FORMATVERSION_##v))
#define NOT_VERSION(v)  if (!AtLeastVersion(FORMATVERSION_##v))

#define FOREACH(t, l, i) \
    for (std::vector<t>::const_iterator i = l.begin(); i != l.end(); ++i)

#define WRITESAFE(f) if (!f) return false

#define COMMIT_OBJECT(n)    \
    Sp##n Object;           \
    if (!Commit##n(Object)) \
        return false;

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
 * SpSceneExporter class
 */

const SpSceneExporter::WriteObjectProc SpSceneExporter::WriteObjectProcList[SpSceneFormatHandler::LUMP_COUNT] = {
    &SpSceneExporter::WriteSceneConfig,
    
    &SpSceneExporter::WriteMesh,
    &SpSceneExporter::WriteLight,
    &SpSceneExporter::WriteCamera,
    &SpSceneExporter::WriteWayPoint,
    
    &SpSceneExporter::WriteTexture,
    &SpSceneExporter::WriteTextureClass,
    
    &SpSceneExporter::WriteLightmap,
    &SpSceneExporter::WriteLightmapScene,
    
    &SpSceneExporter::WriteShaderClass,
    
    &SpSceneExporter::WriteBoundVolume,
    &SpSceneExporter::WriteSound,
    &SpSceneExporter::WriteSprite,
    &SpSceneExporter::WriteAnimNode
};

SpSceneExporter::SpSceneExporter()
{
}
SpSceneExporter::~SpSceneExporter()
{
}

bool SpSceneExporter::ExportScene(const std::string &Filename, const EFormatVersions Version)
{
    if (!SpSceneFormatHandler::IsVersionSupported(Version))
        return ReturnWithError("Unsupported format version", ERROR_DEFAULT);
    
    /* Open file and write header first */
    if (!File_.Open(Filename))
        return ReturnWithError("Could not create file: \"" + Filename + "\"", ERROR_FILE);
    
    if (!WriteHeader(Version))
        return ReturnWithError();
    
    /* Write lump directories */
    for (uint32 i = LUMP_SCENECONFIG; i < LUMP_COUNT; ++i)
    {
        if (!WriteLump(static_cast<ELumpDirectories>(i), Header_.Lumps[i], SpLumpNames[i], SpSceneExporter::WriteObjectProcList[i]))
            return ReturnWithError();
    }
    
    File_.Close();
    
    return true;
}


/*
 * ======= Protected: =======
 */

#define DEFINE_COMMIT_DUMMY(n)                      \
    bool SpSceneExporter::Commit##n(Sp##n  &Unused) \
    {                                               \
        return false;                               \
    }

DEFINE_COMMIT_DUMMY(SceneConfig     )
DEFINE_COMMIT_DUMMY(Mesh            )
DEFINE_COMMIT_DUMMY(Camera          )
DEFINE_COMMIT_DUMMY(WayPoint        )
DEFINE_COMMIT_DUMMY(Light           )
DEFINE_COMMIT_DUMMY(BoundVolume     )
DEFINE_COMMIT_DUMMY(Sound           )
DEFINE_COMMIT_DUMMY(Sprite          )
DEFINE_COMMIT_DUMMY(AnimNode        )
DEFINE_COMMIT_DUMMY(Texture         )
DEFINE_COMMIT_DUMMY(TextureClass    )
DEFINE_COMMIT_DUMMY(Lightmap        )
DEFINE_COMMIT_DUMMY(LightmapScene   )
DEFINE_COMMIT_DUMMY(ShaderClass     )

#undef DEFINE_CATCH_DUMMY


/*
 * ======= Private: =======
 */

bool SpSceneExporter::ReturnWithError()
{
    File_.Close();
    return false;
}
bool SpSceneExporter::ReturnWithError(const std::string &Message, const EErrorTypes Type)
{
    Error(Message, Type);
    return false;
}

bool SpSceneExporter::WriteHeader(int32 Version)
{
    /* Write magic number and version */
    INT(SpSceneFormatHandler::GetMagicNumber()  );
    INT(Header_.Version = Version               );
    
    /* Get lump directory information */
    const uint32 LumpCount = GetLumpCount();
    
    for (uint32 i = 0; i < LumpCount; ++i)
        Header_.Lumps[i].Count = GetLumpElementCount(static_cast<ELumpDirectories>(i));
    
    /* Write pre-setting for lump directories */
    File_.Write(&Header_.Lumps[0].Offset, sizeof(SpLump)*LumpCount);
    
    return true;
}


bool SpSceneExporter::WriteLump(
    const ELumpDirectories Type, const SpLump &Lump, const std::string &Name, WriteObjectProc Proc)
{
    /* Check if lump is used */
    if (!Lump.IsUsed())
        return true;
    
    const uint32 LumpOffset = File_.GetPosition();
    
    for (uint32 i = 0; i < Lump.Count; ++i)
    {
        /* Call lump procedure */
        WRITESAFE((*this.*Proc)());
    }
    
    /* Write lump information (offset and size) after writing the lump directory it self */
    const uint32 Pos = File_.GetPosition();
    File_.Seek(8 + 12 * Type);
    
    UINT(LumpOffset);
    UINT(Pos - LumpOffset);
    
    File_.Seek(Pos);
    
    return true;
}


/*
 * ======= Private: Main Objects =======
 */

bool SpSceneExporter::WriteSceneConfig()
{
    COMMIT_OBJECT(SceneConfig);
    
    /* Write scene configuration */
    VERSION(1_03)
    {
        STR(Object.ResourcePath);
        
        VERSION(1_05)
            STR(Object.ScriptTemplateFile);
    }
    
    for (int32 i = 0; i < 6; ++i)
        STR(Object.SkyBoxTexFilenames[i]);
    
    return true;
}

bool SpSceneExporter::WriteMesh()
{
    COMMIT_OBJECT(Mesh);
    
    /* Write mesh object */
    WRITESAFE(WriteBaseObject       (Object.BaseObject  ));
    WRITESAFE(WriteMeshConstruction (Object.Construct   ));
    
    VERSION(1_02)
        UINT(Object.ShaderClassId);
    
    WRITESAFE(WriteMaterial(Object.Material));
    
    /* Write surfaces */
    UINT(Object.Surfaces.size());
    
    FOREACH(SpSurface, Object.Surfaces, it)
        WRITESAFE(WriteSurface(*it));
    
    /* Write collision, script and animation */
    VERSION(1_01)
    {
        WRITESAFE(WriteCollision            (Object.Collision   ));
        WRITESAFE(WriteAnimScriptBaseObject (Object             ));
    }
    
    return true;
}

bool SpSceneExporter::WriteCamera()
{
    //todo
    return true;
}

bool SpSceneExporter::WriteWayPoint()
{
    //todo
    return true;
}

bool SpSceneExporter::WriteLight()
{
    //todo
    return true;
}

bool SpSceneExporter::WriteBoundVolume()
{
    //todo
    return true;
}

bool SpSceneExporter::WriteSound()
{
    //todo
    return true;
}

bool SpSceneExporter::WriteSprite()
{
    //todo
    return true;
}

bool SpSceneExporter::WriteAnimNode()
{
    //todo
    return true;
}

bool SpSceneExporter::WriteTexture()
{
    //todo
    return true;
}

bool SpSceneExporter::WriteTextureClass()
{
    //todo
    return true;
}

bool SpSceneExporter::WriteLightmap()
{
    //todo
    return true;
}

bool SpSceneExporter::WriteLightmapScene()
{
    //todo
    return true;
}

bool SpSceneExporter::WriteShaderClass()
{
    //todo
    return true;
}


/*
 * ======= Private: Base Objects =======
 */

bool SpSceneExporter::WriteBaseObject(const SpBaseObject &Object)
{
    
    return true;
}

bool SpSceneExporter::WriteMaterial(const SpMaterial &Material)
{
    
    return true;
}

bool SpSceneExporter::WriteViewCulling(const SpViewCulling &ViewCulling)
{
    
    return true;
}

bool SpSceneExporter::WriteScriptTemplateData(const SpScriptTemplateData &ScriptTemplate)
{
    
    return true;
}

bool SpSceneExporter::WriteScriptData(const SpScriptData &ScriptData)
{
    
    return true;
}

bool SpSceneExporter::WriteShaderRTObject(const SpShaderRTObject &ShaderRTObject)
{
    
    return true;
}

bool SpSceneExporter::WriteAnimationKeyframe(const SpAnimationKeyframe &Keyframe)
{
    
    return true;
}

bool SpSceneExporter::WriteAnimation(const SpAnimation &Animation)
{
    
    return true;
}

bool SpSceneExporter::WriteAnimationObject(const SpAnimationObject &AnimObject)
{
    
    return true;
}

bool SpSceneExporter::WriteSurfaceLayer(const SpSurfaceLayer &SurfaceLayer)
{
    
    return true;
}

bool SpSceneExporter::WriteSurface(const SpSurface &Surface)
{
    
    return true;
}

bool SpSceneExporter::WriteCollision(const SpCollision &Collision)
{
    
    return true;
}

bool SpSceneExporter::WriteMeshConstructionBasic(const SpMeshConstructionBasic &Construct)
{
    
    return true;
}

bool SpSceneExporter::WriteMeshConstructionProcStaircase(const SpMeshConstructionProcStaircase &Construct)
{
    
    return true;
}

bool SpSceneExporter::WriteMeshConstructionProc(const SpMeshConstructionProc &Construct)
{
    
    return true;
}

bool SpSceneExporter::WriteMeshConstructionResource(const SpMeshConstructionResource &Construct)
{
    
    return true;
}

bool SpSceneExporter::WriteMeshConstruction(const SpMeshConstruction &Construct)
{
    
    return true;
}

bool SpSceneExporter::WriteAnimScriptBaseObject(const SpAnimScriptBaseObject &Object)
{
    
    return true;
}

bool SpSceneExporter::WriteTextureClassLayer(const SpTextureClassLayer &TexLayer)
{
    
    return true;
}

bool SpSceneExporter::WriteLightmapSceneVertex(const SpLightmapSceneVertex &Vertex, uint8 LayerCount)
{
    
    return true;
}

bool SpSceneExporter::WriteLightmapSceneLayer(const SpLightmapSceneLayer &Layer)
{
    
    return true;
}

bool SpSceneExporter::WriteLightmapSceneSurface(const SpLightmapSceneSurface &Surface)
{
    
    return true;
}

bool SpSceneExporter::WriteShader(const SpShader &Shader)
{
    
    return true;
}

bool SpSceneExporter::WriteShaderParameter(const SpShaderParameter &Param)
{
    
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
#undef BUFFER

#undef VERSION
#undef NOT_VERSION

#undef FOREACH
#undef WRITESAFE

#undef COMMIT_OBJECT


} // /namespace sps



// ================================================================================
