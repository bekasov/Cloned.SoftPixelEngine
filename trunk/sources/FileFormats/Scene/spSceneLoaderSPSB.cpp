/*
 * SoftPixel Sandbox Scene loader file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "FileFormats/Scene/spSceneLoaderSPSB.hpp"
#include "Framework/Tools/spToolScriptLoader.hpp"

#ifdef SP_COMPILE_WITH_SCENELOADER_SPSB


#include "Platform/spSoftPixelDeviceOS.hpp"
#include "SceneGraph/spSceneMesh.hpp"
#include "SceneGraph/spSceneLight.hpp"
#include "SceneGraph/spSceneBillboard.hpp"
#include "SceneGraph/Animation/spNodeAnimation.hpp"

#include <boost/foreach.hpp>


namespace sp
{

extern video::RenderSystem* __spVideoDriver;
extern scene::SceneGraph* __spSceneManager;
extern audio::SoundDevice* __spSoundDevice;

namespace scene
{


#define FOREACH_LUMP(n)                             \
    if (!seekLump(Header_.##n))                     \
        return;                                     \
    for (u32 i = 0; i < Header_.##n##.Count; ++i)

const s32 SceneLoaderSPSB::MAGIC_NUMBER         = *(s32*)"SpSb";
const s32 SceneLoaderSPSB::VERSION_MIN_SUPPORT  = SceneLoaderSPSB::SPSB_VERSION_1_00;
const s32 SceneLoaderSPSB::VERSION_MAX_SUPPORT  = SceneLoaderSPSB::SPSB_VERSION_1_08;

SceneLoaderSPSB::SceneLoaderSPSB() :
    SceneLoader     (   ),
    FormatVersion_  (0  ),
    SkyBox_         (0  )
{
}
SceneLoaderSPSB::~SceneLoaderSPSB()
{
}

Mesh* SceneLoaderSPSB::loadScene(const io::stringc &Filename, const io::stringc &TexturePath, const s32 Flags)
{
    if (!Flags)
    {
        io::Log::warning("Loading an SPSB file with no flags has no effect");
        return 0;
    }
    
    Filename_       = Filename;
    TexturePath_    = TexturePath;
    Flags_          = Flags;
    
    /* Open file for reading */
    if (!openForReading())
        return 0;
    
    /* Read header */
    if (!readHeader())
        return 0;
    
    /* Read scene configuration, textures, shaders and lightmaps */
    if (Flags & SCENEFLAG_CONFIG)
        readSceneConfig();
    
    if (Flags & SCENEFLAG_TEXTURES)
    {
        readTextures();
        readTextureClasses();
    }
    
    if (FormatVersion_ >= SPSB_VERSION_1_02 && (Flags & SCENEFLAG_SHADERS))
        readShaders();
    
    if (Flags & SCENEFLAG_LIGHTMAPS)
    {
        readLightmaps();
        readLightmapScene();
    }
    
    /* Read scene objects */
    if (FormatVersion_ >= SPSB_VERSION_1_08 && (Flags & SCENEFLAG_ANIMNODES))
        readAnimNodes();
    
    if (Flags & SCENEFLAG_MESHES)
        readMeshes();
    if (Flags & SCENEFLAG_LIGHTS)
        readLights();
    if (Flags & SCENEFLAG_CAMERAS)
        readCameras();
    if (Flags & SCENEFLAG_WAYPOINTS)
        readWayPoints();
    
    if (FormatVersion_ >= SPSB_VERSION_1_06)
    {
        if ((Flags & SCENEFLAG_SOUNDS) && __spSoundDevice)
            readSounds();
        if (Flags & SCENEFLAG_SPRITES)
            readSprites();
        if (Flags & SCENEFLAG_BOUNDVOLUMES)
            readBoundVolumes();
    }
    
    /* Apply queues */
    applyQueues();
    
    return 0;
}


/*
 * ======= Protected: =======
 */

bool SceneLoaderSPSB::readHeader()
{
    /* Read header buffer */
    readBuffer(&Header_, sizeof(SHeader));
    
    /* Check magic and version number */
    if (Header_.Magic != SceneLoaderSPSB::MAGIC_NUMBER)
    {
        io::Log::error("Scene file has invalid magic number");
        return false;
    }
    
    if (Header_.Version < SceneLoaderSPSB::VERSION_MIN_SUPPORT)
    {
        io::Log::error("Scene file version is too old to be loaded");
        return false;
    }
    
    if (Header_.Version > SceneLoaderSPSB::VERSION_MAX_SUPPORT)
    {
        io::Log::error("Scene loader is too old to load this scene file");
        return false;
    }
    
    FormatVersion_ = Header_.Version;
    
    /* Print format version */
    const s32 Major = FormatVersion_ / 100;
    const s32 Minor = FormatVersion_ % 100;
    
    io::Log::message(
        "Format version " + io::stringc(Major) + "." +
        (Minor < 10 ? io::stringc("0") + io::stringc(Minor) : io::stringc(Minor))
    );
    
    return true;
}

bool SceneLoaderSPSB::seekLump(const SLump &Lump)
{
    if (Lump.Offset > 0 && Lump.Size > 0)
    {
        File_->setSeek(Lump.Offset);
        return true;
    }
    return false;
}


/*
 * ======= Reading functions for main lumps: =======
 */

void SceneLoaderSPSB::readSceneConfig()
{
    if (!seekLump(Header_.SceneConfig))
        return;
    
    /* Read resource path */
    if (FormatVersion_ >= SPSB_VERSION_1_03)
        ResourcePath_ = readString();
    if (FormatVersion_ >= SPSB_VERSION_1_05)
        ScriptTemplateFilename_ = readString();
    
    /* Read skybox texture filenames */
    video::Texture* SkyboxTextures[6] = { 0 };
    bool hasSkyBox = true;
    
    __spVideoDriver->setTextureGenFlags(video::TEXGEN_WRAP, video::TEXWRAP_CLAMP);
    
    for (s32 i = 0; i < 6; ++i)
    {
        const io::stringc Filename = readString();
        
        if (Filename.size())
            SkyboxTextures[i] = __spVideoDriver->loadTexture(getAbsolutePath(Filename));
        else
            hasSkyBox = false;
    }
    
    __spVideoDriver->setTextureGenFlags(video::TEXGEN_WRAP, video::TEXWRAP_REPEAT);
    
    /* Create skybox */
    if (hasSkyBox)
    {
        SkyBox_ = __spSceneManager->createSkyBox(SkyboxTextures);
        SkyBox_->setName("skybox");
    }
}

void SceneLoaderSPSB::readMeshes()
{
    SBaseObject BaseObject;
    
    FOREACH_LUMP(Meshes)
    {
        /* Read base object data */
        readBaseObject(BaseObject);
        
        scene::Mesh* Object = 0;
        
        const EMeshBuildTypes BuildType = static_cast<EMeshBuildTypes>(readValue<s8>());
        
        switch (BuildType)
        {
            case MESHBUILD_BASICMESH:
            {
                SBasicMesh BasicMesh;
                readBuffer(&BasicMesh, sizeof(SBasicMesh));
                
                Object = applyBasicMesh(
                    static_cast<scene::EBasicMeshes>(BasicMesh.BasicMeshType),
                    scene::SMeshConstruct(
                        BasicMesh.SegsVert,
                        BasicMesh.SegsHorz,
                        BasicMesh.InnerRadius,
                        BasicMesh.OuterRadius,
                        BasicMesh.RotationDeg,
                        BasicMesh.RotationDist,
                        BasicMesh.HasCap,
                        static_cast<video::EShadingTypes>(BasicMesh.Shading)
                    )
                );
            }
            break;
            
            case MESHBUILD_PROCMESH:
            {
                const EProceduralMeshes ProcMeshType = static_cast<EProceduralMeshes>(readValue<s8>());
                
                switch (ProcMeshType)
                {
                    case PROCMESH_STAIRCASE:
                    {
                        SProcMeshStaircase ProcMeshStaircase;
                        readBuffer(&ProcMeshStaircase, sizeof(SProcMeshStaircase));
                        
                        //...
                    }
                    break;
                    
                    default:
                        break;
                }
                
                Object = __spSceneManager->createMesh();
            }
            break;
            
            case MESHBUILD_MESHFILE:
            {
                const io::stringc Filename = readString();
                
                Object = applyResourceMesh(getAbsolutePath(Filename));
            }
            break;
            
            default:
                break;
        }
        
        if (!Object)
        {
            io::Log::error("Unexpected failure while loading meshes");
            return;
        }
        
        /* Setup object name and location */
        applyBaseObject(Object, BaseObject);
        
        /* Read and apply shader class */
        if (FormatVersion_ >= SPSB_VERSION_1_02)
        {
            u32 ShaderClassId = readValue<u32>();
            
            //...
        }
        
        /* Read and apply material */
        SMaterial Material;
        readBuffer(&Material, sizeof(SMaterial));
        
        applyMaterial(Material, Object->getMaterial());
        
        if (Material.GetShadow && (Flags_ & SCENEFLAG_LIGHTMAPS) && Header_.LightmapScene.Count > 0)
            Object->setVisible(false);
        
        if (Material.Shading < 2)
            Object->updateNormals();
        
        /* Read and setup surfaces */
        const u32 SurfaceCount = readValue<u32>();
        
        for (u32 s = 0; s < SurfaceCount; ++s)
        {
            video::MeshBuffer* Surface = Object->getMeshBuffer(s);
            
            const io::stringc SurfName(readString());
            
            if (Surface)
                Surface->setName(SurfName);
            
            if (FormatVersion_ >= SPSB_VERSION_1_02)
            {
                /* Read hardware buffer information */
                SHardwareBuffer HardwareBuffer;
                readBuffer(&HardwareBuffer, sizeof(SHardwareBuffer));
                
                video::VertexFormat* VertFormat = 0;
                
                switch (HardwareBuffer.VertexFormat)
                {
                    case 0: VertFormat = __spVideoDriver->getVertexFormatDefault(); break;
                    case 1: VertFormat = __spVideoDriver->getVertexFormatReduced(); break;
                    case 2: VertFormat = __spVideoDriver->getVertexFormatExtended(); break;
                    case 3: VertFormat = __spVideoDriver->getVertexFormatFull(); break;
                }
                
                if (Surface)
                {
                    /* Apply hardware mesh buffer format and instancing */
                    applyMeshBufferFormat(Surface, VertFormat, static_cast<video::ERendererDataTypes>(HardwareBuffer.IndexFormat));
                    
                    Surface->setHardwareInstancing(HardwareBuffer.InstanceCount);
                }
            }
            
            /* Read texture information */
            const u32 TexClassId = readValue<u32>();
            const s8 LayerCount = readValue<s8>();
            
            /* Read and get texture class */
            const STextureClass* TexClass = 0;
            
            if (TexClassId > 0)
            {
                std::map<u32, STextureClass>::iterator itTexClass = TextureClasses_.find(TexClassId);
                
                if (itTexClass != TextureClasses_.end())
                    TexClass = &(itTexClass->second);
                else
                    io::Log::error("Wrong ID number for texture class");
            }
            
            /* Read texture layers */
            for (s8 l = 0; l < LayerCount; ++l)
            {
                SSurfaceLayer Layer;
                readBuffer(&Layer, sizeof(SSurfaceLayer));
                
                if (!Surface)
                    continue;
                
                /* Setup texture coordinate generation */
                if (Layer.AutoMapMode != AUTOMAP_DISABLE)
                    Object->textureAutoMap(l, Layer.Density, s, Layer.AutoMapMode == AUTOMAP_GLOBAL);
                
                if (Layer.TexId > 0)
                {
                    /* Apply texture */
                    video::Texture* Tex = Textures_[Layer.TexId];
                    
                    if (Tex)
                        applyTextureMapping(Surface, Tex, Layer.TexId, l);
                    else
                        io::Log::error("Wrong ID number for mesh texture");
                }
                
                if (TexClass && l < TexClass->Layers.size())
                {
                    /* Setup texture configuration from texture class */
                    const STextureClassLayer* ClassLayer = &(TexClass->Layers[l]);
                    
                    Surface->setTextureEnv(l, ClassLayer->Environment);
                    Surface->setMappingGen(l, ClassLayer->Mapping);
                    
                    if (!Layer.TexId && ClassLayer->DefaultTexture)
                        applyTextureMapping(Surface, ClassLayer->DefaultTexture, 0, l);
                }
            }
        }
        
        if (FormatVersion_ >= SPSB_VERSION_1_01)
        {
            /* Read collision information */
            const ECollisionModels CollModel = static_cast<ECollisionModels>(readValue<s8>());
            const scene::EPickingTypes PickModel = static_cast<scene::EPickingTypes>(readValue<s8>());
            
            /* Apply collision */
            applyCollision(Object, CollModel, PickModel);
            
            /* Read script and animation data */
            if (FormatVersion_ >= SPSB_VERSION_1_08)
            {
                readScriptTemplates(BaseObject.ScriptData);
                readAnimationObject(Object);
            }
        }
        
        /* Complete mesh construction */
        completeMeshConstruction(Object, BaseObject);
        applyScriptTemplates(BaseObject);
    }
}

void SceneLoaderSPSB::readLights()
{
    SBaseObject BaseObject;
    
    FOREACH_LUMP(Lights)
    {
        /* Read base object data */
        readBaseObject(BaseObject);
        
        /* Read light settings */
        const s8 LightModel         = readValue<s8>();
        const s8 Volumetric         = readValue<s8>();
        const f32 VolumetricRadius  = readValue<f32>();
        
        /* Create light source object */
        scene::Light* Object = __spSceneManager->createLight(
            static_cast<scene::ELightModels>(LightModel)
        );
        
        Object->setVolumetric(Volumetric != 0);
        Object->setVolumetricRadius(VolumetricRadius);
        
        bool UseForLightmaps = true;
        
        if (FormatVersion_ >= SPSB_VERSION_1_04)
        {
            /* Read light color, lightmap usage and spot cone angles */
            const video::color LightColor = readValue<video::color>();
            UseForLightmaps = static_cast<bool>(readValue<s8>());
            const f32 InnerSpotCone = readValue<f32>();
            const f32 OuterSpotCone = readValue<f32>();
            
            Object->setLightingColor(LightColor);
            Object->setSpotCone(InnerSpotCone, OuterSpotCone);
        }
        
        /* Setup object name and location */
        applyBaseObject(Object, BaseObject);
        
        if (FormatVersion_ >= SPSB_VERSION_1_02)
        {
            /* Read shader render target object */
            SShaderRTObject ShaderRTObject;
            readShaderRTObject(ShaderRTObject);
            
            /* Read script and animation data */
            if (FormatVersion_ >= SPSB_VERSION_1_08)
            {
                readScriptTemplates(BaseObject.ScriptData);
                readAnimationObject(Object);
            }
        }
        
        observeLight(Object, BaseObject, UseForLightmaps);
        applyScriptTemplates(BaseObject);
    }
}

void SceneLoaderSPSB::readCameras()
{
    SBaseObject BaseObject;
    
    FOREACH_LUMP(Cameras)
    {
        /* Read base object data */
        readBaseObject(BaseObject);
        
        /* Read camera settings */
        const f32 FieldOfView   = readValue<f32>();
        const f32 NearPlane     = readValue<f32>();
        const f32 FarPlane      = readValue<f32>();
        
        if (FormatVersion_ >= SPSB_VERSION_1_04)
        {
            const u32 TexId = readValue<u32>();
            //...
        }
        
        /* Create camera object */
        scene::Camera* Object = __spSceneManager->createCamera();
        
        Object->setRange(NearPlane, FarPlane);
        Object->setFOV(FieldOfView);
        
        #if 0
        if (FormatVersion_ >= SPSB_VERSION_1_03)
        {
            /* Read render target texture */
            const u32 TexId = readValue<u32>();
            
            if (TexId > 0)
            {
                video::Texture* Tex = Textures_[TexId];
                
                if (Tex)
                    observeCameraRenderTarget(Object, Tex);
                else
                    io::Log::error("Wrong ID number for camera's render target texture");
            }
        }
        #endif
        
        /* Read script and animation data */
        if (FormatVersion_ >= SPSB_VERSION_1_08)
        {
            readScriptTemplates(BaseObject.ScriptData);
            readAnimationObject(Object);
        }
        
        /* Setup object name and location */
        applyBaseObject(Object, BaseObject);
        applyScriptTemplates(BaseObject);
    }
}

void SceneLoaderSPSB::readWayPoints()
{
    FOREACH_LUMP(WayPoints)
    {
        SWayPoint WayPoint;
        
        /* Read base object data */
        readBaseObject(WayPoint.BaseObject);
        
        /* Read neighbor ID list */
        const u32 NeighborCount = readValue<u32>();
        
        WayPoint.NeighborIdList.resize(NeighborCount);
        
        for (u32 j = 0; j < NeighborCount; ++j)
            WayPoint.NeighborIdList[j] = readValue<u32>();
        
        WayPoints_.push_back(WayPoint);
    }
}

void SceneLoaderSPSB::readBoundVolumes()
{
    SBaseObject BaseObject;
    
    FOREACH_LUMP(BoundVolumes)
    {
        /* Read base object data and apply bounding volume */
        readBaseObject(BaseObject);
        
        /* Read script and animation data */
        if (FormatVersion_ >= SPSB_VERSION_1_08)
        {
            readScriptTemplates(BaseObject.ScriptData);
            readAnimationObject(0);//!!!
        }
        
        applyBoundingVolume(BaseObject);
        applyScriptTemplates(BaseObject);
    }
}

void SceneLoaderSPSB::readSounds()
{
    SSound Sound;
    
    FOREACH_LUMP(Sounds)
    {
        /* Read base object data */
        readBaseObject(Sound.BaseObject);
        
        /* Read sound settings */
        Sound.Filename  = readString();
        Sound.Flags     = readValue<s8>();
        Sound.Volume    = readValue<f32>();
        Sound.Speed     = readValue<f32>();
        Sound.Balance   = readValue<f32>();
        Sound.Radius    = readValue<f32>();
        
        /* Read script and animation data */
        if (FormatVersion_ >= SPSB_VERSION_1_08)
        {
            readScriptTemplates(Sound.BaseObject.ScriptData);
            readAnimationObject(0);//!!!
        }
        
        /* Apply sound object */
        applySound(Sound);
        applyScriptTemplates(Sound.BaseObject);
    }
}

void SceneLoaderSPSB::readSprites()
{
    SSprite Sprite;
    
    FOREACH_LUMP(Sprites)
    {
        /* Read base object data */
        readBaseObject(Sprite.BaseObject);
        
        /* Read sprite settings */
        const u32 TexId = readValue<u32>();
        
        Sprite.Tex = (TexId > 0 ? Textures_[TexId] : 0);
        
        Sprite.Color = readValue<video::color>();
        
        /* Read script and animation data */
        if (FormatVersion_ >= SPSB_VERSION_1_08)
        {
            readScriptTemplates(Sprite.BaseObject.ScriptData);
            readAnimationObject(0);//!!!
        }
        
        /* Apply sprite object */
        applySprite(Sprite);
        applyScriptTemplates(Sprite.BaseObject);
    }
}

void SceneLoaderSPSB::readAnimNodes()
{
    SBaseObject BaseObject;
    
    FOREACH_LUMP(AnimNodes)
    {
        /* Read base object data and apply animation node */
        readBaseObject(BaseObject);
        applyAnimNode(BaseObject);
    }
}

void SceneLoaderSPSB::readTextures()
{
    FOREACH_LUMP(Textures)
    {
        /* Create new texture */
        const u32 Id = readValue<u32>();
        const io::stringc Filename = readString();
        
        video::Texture* Tex = 0;
        
        if (Filename.size())
            Tex = __spVideoDriver->loadTexture(getAbsolutePath(Filename));
        else
        {
            s32 Width = 1, Height = 1;
            
            if (FormatVersion_ >= SPSB_VERSION_1_02)
            {
                Width = readValue<s32>();
                Height = readValue<s32>();
            }
            
            Tex = __spVideoDriver->createTexture(dim::size2di(Width, Height));
        }
        
        /* Read texture information */
        STextureConfig TexConfig;
        readBuffer(&TexConfig, sizeof(STextureConfig));
        
        applyTexture(Tex, TexConfig, Id);
        
        /* Add texture to the map */
        Textures_[Id] = Tex;
    }
}

void SceneLoaderSPSB::readTextureClasses()
{
    FOREACH_LUMP(TextureClasses)
    {
        /* Create new texture class */
        const u32 Id = readValue<u32>();
        
        STextureClass* TexClass = &(TextureClasses_[Id]);
        
        /* Read unused class name */
        readString();
        
        /* Read layers */
        const u8 LayerCount = readValue<u8>();
        
        for (u8 l = 0; l < LayerCount; ++l)
        {
            STextureClassLayer Layer;
            
            /* Read unused layer name */
            readString();
            
            /* Read layer information */
            Layer.Type          = static_cast<ETextureClassLayerTypes>(readValue<s8>());
            Layer.Environment   = static_cast<video::ETextureEnvTypes>(readValue<s8>());
            Layer.Mapping       = static_cast<video::EMappingGenTypes>(readValue<s8>());
            
            /* Read and get default texture */
            const u32 TexId = readValue<u32>();
            
            if (TexId > 0)
            {
                video::Texture* Tex = Textures_[TexId];
                
                if (Tex)
                    Layer.DefaultTexture = Tex;
                else
                    io::Log::error("Wrong ID number for texture class layer");
            }
            
            TexClass->Layers.push_back(Layer);
        }
    }
}

void SceneLoaderSPSB::readLightmaps()
{
    /* Temporary memory */
    video::color Texel;
    video::Texture* Tex = 0;
    
    FOREACH_LUMP(Lightmaps)
    {
        /* Read size and create texture */
        const s32 Size = readValue<s32>();
        
        Tex = __spVideoDriver->createTexture(Size);
        video::ImageBuffer* ImgBuffer = Tex->getImageBuffer();
        
        dim::point2di Pos;
        
        /* Read lightmap texel blocks */
        for (s32 j = 0, c = Size * Size / 8; j < c; ++j)
        {
            u8 BitSet = readValue<u8>();
            
            for (s32 k = 0; k < 8; ++k)
            {
                /* Read texel color */
                if (math::getBitR2L(BitSet, k))
                {
                    Texel.Red   = readValue<u8>();
                    Texel.Green = readValue<u8>();
                    Texel.Blue  = readValue<u8>();
                }
                else
                    Texel = video::color(readValue<u8>());
                
                /* Set texel color */
                ImgBuffer->setPixelColor(Pos, Texel);
                
                /* Boost texel coordinate */
                if (++Pos.X >= Size)
                {
                    Pos.X = 0;
                    ++Pos.Y;
                }
            }
        }
        
        /* Update image buffer and add texture to the list */
        Tex->updateImageBuffer();
        
        LightmapTextures_.push_back(Tex);
    }
}

void SceneLoaderSPSB::readLightmapScene()
{
    FOREACH_LUMP(LightmapScene)
    {
        /* Create mesh and set first mesh as main lightmap scene object */
        scene::Mesh* Mesh = __spSceneManager->createMesh();
        
        Mesh->setName("LM:" + readString());
        const u32 SurfaceCount = readValue<u32>();
        
        Mesh->getMaterial()->setLighting(false);
        
        /* Read all surfaces */
        for (u32 s = 0; s < SurfaceCount; ++s)
        {
            const io::stringc Name(readString());
            
            const s8 VertexFormat       = readValue<s8>(); //!!! todo
            const s8 IndexFormat        = readValue<s8>();
            const u8 LayerCount         = readValue<u8>();
            const u32 LightmapTexIndex  = readValue<u32>();
            
            /* Create new surface */
            video::MeshBuffer* Surface = Mesh->createMeshBuffer();
            
            applyMeshBufferFormat(
                Surface, __spVideoDriver->getVertexFormatDefault(), static_cast<video::ERendererDataTypes>(IndexFormat)
            );
            
            Surface->setName(Name);
            
            /* Read surface layers */
            for (u8 l = 0; l < LayerCount; ++l)
            {
                const u32 TexId         = readValue<u32>();
                const s8 Environment    = readValue<s8>();
                const s8 MappingGen     = readValue<s8>();
                
                if (TexId > 0)
                {
                    video::Texture* Tex = Textures_[TexId];
                    
                    if (Tex)
                    {
                        applyTextureMapping(Surface, Tex, TexId, l);
                        
                        Surface->setTextureEnv(l, static_cast<video::ETextureEnvTypes>(Environment));
                        Surface->setMappingGen(l, static_cast<video::EMappingGenTypes>(MappingGen));
                    }
                    else
                        io::Log::error("Wrong ID number for lightmap scene's texture");
                }
            }
            
            /* Apply lightmap texture */
            if (LightmapTexIndex < LightmapTextures_.size())
                Surface->addTexture(LightmapTextures_[LightmapTexIndex]);
            
            /* Read all vertices */
            const u32 VertexCount = readValue<u32>();
            
            for (u32 j = 0; j < VertexCount; ++j)
            {
                const dim::vector3df Coord(readValue<dim::vector3df>());
                const dim::vector3df Normal(readValue<dim::vector3df>());
                
                Surface->addVertex(Coord, Normal, readValue<dim::point2df>());
                
                for (u8 l = 0; l < LayerCount; ++l)
                    Surface->setVertexTexCoord(j, readValue<dim::point2df>(), l);
                
                /* Create new triangle */
                if ((j + 1) % 3 == 0)
                {
                    Surface->addTriangle(0, 1, 2);
                    Surface->addIndexOffset(3);
                }
            }
        }
        
        Mesh->updateMeshBuffer();
        
        #if 1
        break; //!!! -> currently only read the main lightmap scene object for debugging
        #endif
    }
}

void SceneLoaderSPSB::readShaders()
{
    if (!seekLump(Header_.Shaders))
        return;
    
    
}


/*
 * ======= Base reading functions: =======
 */

void SceneLoaderSPSB::readBaseObject(SBaseObject &Object)
{
    /* Read basic object data */
    Object.Type     = readValue<s8>();
    Object.Id       = readValue<u32>();
    Object.ParentId = readValue<u32>();
    Object.Name     = readString();
    
    if (FormatVersion_ >= SPSB_VERSION_1_04)
        Object.Visible = static_cast<bool>(readValue<s8>());
    else
        Object.Visible = true;
    
    if (FormatVersion_ >= SPSB_VERSION_1_01)
        Object.Flags = readString();
    else
        Object.Flags = 0;
    
    /* Read object transformation */
    if (FormatVersion_ >= SPSB_VERSION_1_07)
    {
        Object.Position = dim::getPositionMatrix(readValue<dim::vector3df>());
        Object.Rotation = readMatrix();
        Object.Scaling  = dim::getScaleMatrix(readValue<dim::vector3df>());
    }
    else
    {
        Object.Position = readMatrix();
        Object.Rotation = readMatrix();
        Object.Scaling  = readMatrix();
    }
    
    /* Read view culling and script data */
    if (FormatVersion_ >= SPSB_VERSION_1_05)
    {
        readViewCulling(Object.ViewCulling);
        if (FormatVersion_ < SPSB_VERSION_1_08)
            readScriptTemplates(Object.ScriptData);
    }
}

void SceneLoaderSPSB::readViewCulling(SViewCulling &ViewCulling)
{
    /* Read view culling */
    ViewCulling.BoundingType    = static_cast<scene::EBoundingVolumes>(readValue<s8>());
    ViewCulling.SphereRadius    = readValue<f32>();
    ViewCulling.BoxSize         = readValue<f32>();
}

void SceneLoaderSPSB::readScriptTemplates(std::vector<SScriptData> &ScriptDataList)
{
    if (FormatVersion_ >= SPSB_VERSION_1_07)
    {
        /* Read all script templates */
        const u32 ScriptCount = readValue<u32>();
        
        ScriptDataList.resize(ScriptCount);
        
        for (u32 i = 0; i < ScriptCount; ++i)
            readScriptData(ScriptDataList[i]);
    }
    else
    {
        /* Read only one script template */
        SScriptData ScriptData;
        readScriptData(ScriptData);
        
        if (ScriptData.TemplateName.size())
            ScriptDataList.push_back(ScriptData);
    }
}

void SceneLoaderSPSB::readScriptData(SScriptData &ScriptData)
{
    /* Read script template */
    ScriptData.TemplateName = readString();
    
    if (ScriptData.TemplateName.size())
    {
        /* Read script parameters */
        const u32 ParamCount = readValue<u32>();
        
        ScriptData.Parameters.resize(ParamCount);
        
        for (u32 i = 0; i < ParamCount; ++i)
            ScriptData.Parameters[i] = readString();
    }
    else
        ScriptData.Parameters.clear();
}

void SceneLoaderSPSB::readShaderRTObject(SShaderRTObject &Object)
{
    readBuffer(&Object, sizeof(SShaderRTObject));
}

void SceneLoaderSPSB::readAnimationObject(scene::SceneNode* Node)
{
    if (FormatVersion_ < SPSB_VERSION_1_08)
        return;
    
    /* Read animations */
    const u32 AnimCount = readValue<u32>();
    
    for (u32 i = 0; i < AnimCount; ++i)
    {
        /* Read node-animation */
        const io::stringc Name      = readString();
        const s8 Flags              = readValue<s8>();
        const f32 SplineExpansion   = readValue<f32>();
        
        /* Create node-animation */
        scene::NodeAnimation* NodeAnim = 0;
        
        if (Node)
        {
            NodeAnim = __spSceneManager->createAnimation<scene::NodeAnimation>(Name);
            Node->addAnimation(NodeAnim);
        }
        
        /* Read animation keyframes */
        const u32 KeyframeCount = readValue<u32>();
        
        for (u32 j = 0; j < KeyframeCount; ++j)
        {
            /* Read keyframe */
            const u32 AnimNodeId    = readValue<u32>();
            const u32 Duration      = readValue<u32>();
            
            /* Add keyframe */
            if (NodeAnim && AnimNodeId > 0)
            {
                /* Find animation node */
                std::map<u32, dim::matrix4f>::iterator it = AnimNodeMatrixMap_.find(AnimNodeId);
                
                if (it != AnimNodeMatrixMap_.end())
                    NodeAnim->addKeyframe(KeyframeTransformation(it->second), Duration);
                else
                    io::Log::error("Invalid ID number for animation-node object (" + io::stringc(AnimNodeId) + ")");
            }
        }
        
        /* Finish animation settings */
        if (NodeAnim)
        {
            NodeAnim->setSplineTranslation(Flags & ANIMFlAG_USESPLINES);
            NodeAnim->setSplineExpansion(SplineExpansion);
        }
    }
}


/*
 * ======= Other functions: =======
 */

void SceneLoaderSPSB::applyQueues()
{
    /* Apply parent queue */
    foreach (const SParentQueue &Queue, QueueParents_)
    {
        if (Queue.Object)
        {
            scene::SceneNode* Parent = ObjectIdMap_[Queue.ParentId];
            
            if (Parent)
                Queue.Object->setParent(Parent, true);
        }
    }
}

void SceneLoaderSPSB::applyMaterial(const SMaterial &Material, video::MaterialStates* MatStates)
{
    /* Apply material settings */
    MatStates->setDiffuseColor(Material.DiffuseColor);
    MatStates->setAmbientColor(Material.AmbientColor);
    MatStates->setSpecularColor(Material.SpecularColor);
    MatStates->setEmissionColor(Material.EmissionColor);
    
    MatStates->setPolygonOffset(
        Material.PolygonOffset != 0, Material.PolygonOffsetFactor, Material.PolygonOffsetUnits
    );
    MatStates->setWireframe(
        static_cast<video::EWireframeTypes>(Material.WireframeFront),
        static_cast<video::EWireframeTypes>(Material.WireframeBack)
    );
    
    MatStates->setShininess(Material.Shininess);
    MatStates->setAlphaReference(Material.AlphaReference);
    MatStates->setDepthMethod(static_cast<video::ESizeComparisionTypes>(Material.DepthMethod));
    MatStates->setAlphaMethod(static_cast<video::ESizeComparisionTypes>(Material.AlphaMethod));
    MatStates->setBlendSource(static_cast<video::EBlendingTypes>(Material.BlendSource));
    MatStates->setBlendTarget(static_cast<video::EBlendingTypes>(Material.BlendTarget));
    MatStates->setRenderFace(static_cast<video::EFaceTypes>(Material.RenderFace));
    
    MatStates->setColorMaterial(Material.ColorMaterial != 0);
    MatStates->setLighting(Material.Lighting != 0);
    MatStates->setBlending(Material.Blending != 0);
    MatStates->setDepthBuffer(Material.DepthBuffer != 0);
    MatStates->setFog(Material.Fog != 0);
    
    if (Material.Shading < 2)
        MatStates->setShading(static_cast<video::EShadingTypes>(Material.Shading));
}

void SceneLoaderSPSB::applyMeshBufferFormat(
    video::MeshBuffer* Surface, video::VertexFormat* VertexFormat, const video::ERendererDataTypes IndexFormat)
{
    Surface->setVertexFormat(VertexFormat);
    Surface->setIndexFormat(IndexFormat);
}

void SceneLoaderSPSB::applyTextureMapping(video::MeshBuffer* Surface, video::Texture* Tex, u32 TexId, u8 Layer)
{
    if (Surface->getTexture(Layer))
        Surface->setTexture(Layer, Tex);
    else
        Surface->addTexture(Tex);
}

void SceneLoaderSPSB::applyBaseObject(scene::SceneNode* Node, const SBaseObject &Object)
{
    /* Set name and transformation */
    Node->setName(Object.Name);
    Node->setPositionMatrix(Object.Position);
    Node->setRotationMatrix(Object.Rotation);
    Node->setScaleMatrix(Object.Scaling);
    
    /* Set visible */
    Node->setVisible(Object.Visible);
    
    /* Setup bounding volume */
    Node->setBoundingType(Object.ViewCulling.BoundingType);
    
    if (Object.ViewCulling.BoundingType == scene::BOUNDING_BOX)
    {
        if (Node->getType() == scene::NODE_MESH)
        {
            scene::Mesh* Mesh = static_cast<scene::Mesh*>(Node);
            dim::aabbox3df BoundBox(Mesh->getMeshBoundingBox());
            
            BoundBox.Min *= Object.ViewCulling.BoxSize;
            BoundBox.Max *= Object.ViewCulling.BoxSize;
            
            Node->setBoundingBox(BoundBox);
        }
        else
            Node->setBoundingBox(dim::aabbox3df(-0.5f, 0.5f));
    }
    else if (Object.ViewCulling.BoundingType == scene::BOUNDING_SPHERE)
        Node->setBoundingSphere(Object.ViewCulling.SphereRadius);
    
    /* Map object ID */
    ObjectIdMap_[Object.Id] = Node;
    
    /* Add parent queue */
    if (Object.ParentId)
    {
        SParentQueue Queue;
        {
            Queue.Object    = Node;
            Queue.ParentId  = Object.ParentId;
        }
        QueueParents_.push_back(Queue);
    }
}

void SceneLoaderSPSB::applyCollision(
    scene::Mesh* Object, const ECollisionModels CollModel, const scene::EPickingTypes PickModel)
{
    // do nothing
}

void SceneLoaderSPSB::applyScriptTemplates(const SBaseObject &Object)
{
    // do nothing
}

scene::Mesh* SceneLoaderSPSB::applyBasicMesh(const scene::EBasicMeshes Type, const scene::SMeshConstruct &Construct)
{
    return __spSceneManager->createMesh(Type, Construct);
}

scene::Mesh* SceneLoaderSPSB::applyResourceMesh(const io::stringc &AbsolutePath)
{
    return __spSceneManager->loadMesh(AbsolutePath);
}

void SceneLoaderSPSB::applyBoundingVolume(const SBaseObject &BaseObject)
{
    // do nothing
}

audio::Sound* SceneLoaderSPSB::applySound(const SSound &Sound)
{
    if (!__spSoundDevice)
        return 0;
    
    /* Load sound file */
    audio::Sound* Object = __spSoundDevice->loadSound(getAbsolutePath(Sound.Filename));
    
    /* Setup sound settings */
    Object->setVolume           (Sound.Volume   );
    Object->setSpeed            (Sound.Speed    );
    Object->setBalance          (Sound.Balance  );
    Object->setVolumetricRadius (Sound.Radius   );
    
    Object->setVolumetric(!(Sound.Flags & SOUNDFLAG_BACKGROUND));
    Object->setLoop(Sound.Flags & SOUNDFLAG_LOOP);
    
    Object->setPosition(Sound.BaseObject.Position.getPosition());
    
    if (Sound.Flags & SOUNDFLAG_STARTUP)
        Object->play();
    
    return Object;
}

scene::Billboard* SceneLoaderSPSB::applySprite(const SSprite &Sprite)
{
    /* Create sprite billboard and setup transformation */
    scene::Billboard* Object = __spSceneManager->createBillboard(Sprite.Tex);
    applyBaseObject(Object, Sprite.BaseObject);
    
    /* Setup material */
    video::MaterialStates* Material = Object->getMaterial();
    
    Material->setDiffuseColor(0);
    Material->setAmbientColor(Sprite.Color);
    
    return Object;
}

void SceneLoaderSPSB::applyAnimNode(const SBaseObject &BaseObject)
{
    /* Store animation node matrix */
    AnimNodeMatrixMap_[BaseObject.Id] = BaseObject.Position * BaseObject.Rotation * BaseObject.Scaling;
}

void SceneLoaderSPSB::observeCameraRenderTarget(scene::Camera* Cam, video::Texture* Tex)
{
    // do nothing
}
void SceneLoaderSPSB::observeLight(scene::Light* Object, const SBaseObject &BaseObject, bool UseForLightmaps)
{
    // do nothing
}

void SceneLoaderSPSB::completeMeshConstruction(scene::Mesh* Object, const SBaseObject &BaseObject)
{
    // do nothing
}

void SceneLoaderSPSB::applyTexture(video::Texture* Tex, const STextureConfig &TexConfig, u32 TexId)
{
    if (!Tex)
        return;
    
    /* Update image buffer  */
    video::ImageBuffer* ImgBuffer = Tex->getImageBuffer();
    {
        ImgBuffer->setFormat(static_cast<video::EPixelFormats>(TexConfig.Format));
        
        if (TexConfig.ColorKey.Alpha < 255)
            ImgBuffer->setColorKey(TexConfig.ColorKey);
    }
    Tex->updateImageBuffer();
    
    /* Update texture settings */
    Tex->setHardwareFormat(static_cast<video::EHWTextureFormats>(TexConfig.HWFormat));
    Tex->setFilter(static_cast<video::ETextureFilters>(TexConfig.Filter));
    Tex->setMipMapFilter(static_cast<video::ETextureMipMapFilters>(TexConfig.MipMapFilter));
    Tex->setAnisotropicSamples(TexConfig.Anisotropy);
    Tex->setMipMapping(TexConfig.HasMipMaps != 0);
    Tex->setWrapMode(
        static_cast<video::ETextureWrapModes>(TexConfig.WrapModeX),
        static_cast<video::ETextureWrapModes>(TexConfig.WrapModeY),
        static_cast<video::ETextureWrapModes>(TexConfig.WrapModeZ)
    );
    Tex->setDimension(static_cast<video::ETextureDimensions>(TexConfig.Dimension));
    Tex->setRenderTarget(TexConfig.RenderTarget != 0);
}

io::stringc SceneLoaderSPSB::getAbsolutePath(const io::stringc &Path) const
{
    if (!Path.size() || !ResourcePath_.size() || !(Flags_ & SCENEFLAG_ABSOLUTEPATH))
        return Path;
    
    const io::stringc AbsolutePath(ResourcePath_ + Path);
    
    if (!io::FileSystem().findFile(AbsolutePath))
        return Path;
    
    return AbsolutePath;
}

#undef FOREACH_LUMP


} // /namespace scene

} // /namespace sp


#endif



// ================================================================================
