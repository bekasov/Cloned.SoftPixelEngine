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


using namespace sps;

namespace sp
{

extern video::RenderSystem* __spVideoDriver;
extern scene::SceneGraph* __spSceneManager;

namespace scene
{


SceneLoaderSPSB::SceneLoaderSPSB() :
    SceneLoader     (       ),
    SpSceneImporter (       ),
    HasLightmaps_   (false  )
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
    
    Flags_ = Flags;
    
    if (!ImportScene(Filename.str()))
        return 0;
    
    /* Apply queues */
    applyQueues();
    
    return 0;
}


/*
 * ======= Protected: callback functions =======
 */

void SceneLoaderSPSB::Error(const std::string &Message, const EErrorTypes Type)
{
    io::Log::error(Message);
}
void SceneLoaderSPSB::Warning(const std::string &Message, const EWarningType Type)
{
    io::Log::warning(Message);
}

bool SceneLoaderSPSB::CatchHeader(const SpHeader &Header)
{
    io::Log::message("Format version " + io::stringc(SpSceneFormatHandler::GetVersionString(Header.Version)));
    
    HasLightmaps_ = (Header.Lumps[LUMP_LIGHTMAPSCENE].IsUsed());
    
    return true;
}

bool SceneLoaderSPSB::CatchSceneConfig(const SpSceneConfig &Object)
{
    /* Copy object attributes */
    ResourcePath_           = Object.ResourcePath;
    ScriptTemplateFilename_ = Object.ScriptTemplateFile;
    
    createSkyBox(Object.SkyBoxTexFilenames);
    
    return true;
}

bool SceneLoaderSPSB::CatchMesh(const SpMesh &Object)
{
    /* Create mesh object */
    Mesh* MeshObj = 0;
    
    switch (Object.Construct.BuildType)
    {
        case 1:     MeshObj = createMeshBasic   (Object.Construct.BasicMesh     ); break;
        case 3:     MeshObj = createMeshResource(Object.Construct.ResourceMesh  ); break;
        default:    return true;
    }
    
    if (!MeshObj)
    {
        Error("Mesh object is invalid");
        return true;
    }
    
    /* Setup object */
    if ( !setupBaseObject(MeshObj, Object.BaseObject) ||
         !setupMaterial(MeshObj->getMaterial(), Object.Material) )
    {
        return false;
    }
    
    if (Object.Material.GetShadow && (Flags_ & SCENEFLAG_LIGHTMAPS) && HasLightmaps_)
        MeshObj->setVisible(false);
    if (Object.Material.Shading < 2)
        MeshObj->updateNormals();
    
    /* Setup surfaces and their texture layers */
    for (u32 s = 0; s < Object.Surfaces.size(); ++s)
    {
        if (!setupSurface(MeshObj, MeshObj->getMeshBuffer(s), Object.Surfaces[s], s))
            return false;
    }
    
    /* Setup collision- and picking models, shader class, script templates, animation and complete construction */
    return (
        setupMeshCollision(
            MeshObj,
            static_cast<ECollisionModels>(Object.Collision.CollisionModel),
            static_cast<EPickingTypes>(Object.Collision.PickingModel)
        ) &&
        
        setupMeshShader(MeshObj, Object.ShaderClassId) &&
        setupScriptTemplates(MeshObj, Object.BaseObject, Object.ScriptData) &&
        setupAnimation(MeshObj, Object.AnimObject) &&
        
        completeMeshConstruct(MeshObj, Object)
    );
}

bool SceneLoaderSPSB::CatchCamera(const SpCamera &Object)
{
    /* Create camera object */
    Camera* CameraObj = __spSceneManager->createCamera();
    
    CameraObj->setFOV(Object.FieldOfView);
    CameraObj->setRange(Object.NearPlane, Object.FarPlane);
    
    /* Setup object */
    return (
        setupBaseObject(CameraObj, Object.BaseObject) &&
        setupScriptTemplates(CameraObj, Object.BaseObject, Object.ScriptData) &&
        setupAnimation(CameraObj, Object.AnimObject) &&
        completeCameraConstruct(CameraObj, Object)
    );
}

bool SceneLoaderSPSB::CatchWayPoint(const SpWayPoint &Object)
{
    //todo
    return true;
}

bool SceneLoaderSPSB::CatchLight(const SpLight &Object)
{
    /* Create light object */
    Light* LightObj = __spSceneManager->createLight(static_cast<ELightModels>(Object.LightModel));
    
    LightObj->setVolumetric(Object.Volumetric != 0);
    LightObj->setVolumetricRadius(Object.VolumetricRadius);
    LightObj->setLightingColor(convert(Object.LightColor));
    LightObj->setSpotCone(Object.InnerSpotCone, Object.OuterSpotCone);
    
    /* Setup object */
    return (
        setupBaseObject(LightObj, Object.BaseObject) &&
        setupScriptTemplates(LightObj, Object.BaseObject, Object.ScriptData) &&
        setupAnimation(LightObj, Object.AnimObject) &&
        completeLightConstruct(LightObj, Object)
    );
}

bool SceneLoaderSPSB::CatchBoundVolume(const SpBoundVolume &Object)
{
    //todo
    return true;
}

bool SceneLoaderSPSB::CatchSound(const SpSound &Object)
{
    //todo
    return true;
}

bool SceneLoaderSPSB::CatchSprite(const SpSprite &Object)
{
    /* Create sprite object */
    Billboard* SpriteObj = __spSceneManager->createBillboard();
    
    /* Setup sprite texture */
    video::Texture* Tex = findTexture(Object.TexId);
    
    if (Tex)
        SpriteObj->setTexture(Tex);
    
    /* Setup sprite material */
    video::MaterialStates* Material = SpriteObj->getMaterial();
    
    Material->setDiffuseColor(0);
    Material->setAmbientColor(convert(Object.Color));
    
    /* Setup object */
    return (
        setupBaseObject(SpriteObj, Object.BaseObject) &&
        setupAnimation(SpriteObj, Object.AnimObject) &&
        completeSpriteConstruct(SpriteObj, Object)
    );
}

bool SceneLoaderSPSB::CatchAnimNode(const SpAnimNode &Object)
{
    /* Create animation node object */
    AnimNodeTransMap_[Object.BaseObject.Id] = Transformation(
        convert(Object.BaseObject.Position  ),
        convert(Object.BaseObject.Rotation  ),
        convert(Object.BaseObject.Scaling   )
    );
    return true;
}

bool SceneLoaderSPSB::CatchTexture(const SpTexture &Object)
{
    /* Create texture object */
    video::Texture* Tex = 0;
    
    if (Object.Filename.size() > 0)
        Tex = __spVideoDriver->loadTexture(getFinalPath(Object.Filename));
    else
        Tex = __spVideoDriver->createTexture(dim::size2di(Object.Size.w, Object.Size.h));
    
    /* Setup object and store texture in hash-map */
    setupTexture(Tex, Object);
    
    Textures_[Object.Id] = Tex;
    
    return true;
}

bool SceneLoaderSPSB::CatchTextureClass(const SpTextureClass &Object)
{
    /* Create texture class object */
    TextureClasses_[Object.Id] = Object;
    return true;
}

bool SceneLoaderSPSB::CatchLightmap(const SpLightmap &Object)
{
    /* Create lightmap texture object */
    if (Object.Size <= 0 || Object.ImageBuffer.size() < static_cast<u32>(Object.Size*Object.Size*3))
    {
        Error(("Unexpected image buffer size for lightmap texture #" + io::stringc(LightmapTextures_.size())).str());
        return false;
    }
    
    video::Texture* Tex = __spVideoDriver->createTexture(
        dim::size2di(Object.Size), video::PIXELFORMAT_RGB, &Object.ImageBuffer[0]
    );
    
    /* Add lightmap texture to the list */
    LightmapTextures_.push_back(Tex);
    
    return true;
}

bool SceneLoaderSPSB::CatchLightmapScene(const SpLightmapScene &Object)
{
    /* Create lightmap scene object */
    if (!(Flags_ & SCENEFLAG_LIGHTMAPS))
        return true;
    
    scene::Mesh* MeshObj = __spSceneManager->createMesh();
    
    MeshObj->setName(Object.Name);
    MeshObj->getMaterial()->setLighting(false);
    
    foreach (const SpLightmapSceneSurface &Surface, Object.Surfaces)
    {
        if (!setupLightmapSceneSurface(MeshObj->createMeshBuffer(), Surface))
            return false;
    }
    
    return setupMeshCollision(MeshObj, scene::COLLISION_MESH, scene::PICKMODE_POLYGON);
}

bool SceneLoaderSPSB::CatchShaderClass(const SpShaderClass &Object)
{
    /* Create shader class object */
    if (!Object.Shaders[0].ShaderCode.size())
        return true;
    
    video::ShaderClass* ShaderClassObj = __spVideoDriver->createShaderClass();
    
    ShaderClassObj->setName(Object.Name);
    
    video::Shader* VertShd = createShader(Object.Shaders[0], ShaderClassObj, video::SHADER_VERTEX    );
    video::Shader* FragShd = createShader(Object.Shaders[1], ShaderClassObj, video::SHADER_PIXEL     );
    video::Shader* GeomShd = createShader(Object.Shaders[2], ShaderClassObj, video::SHADER_GEOMETRY  );
    
    /* Setup object and store shader class in hash-map */
    if (ShaderClassObj->link())
    {
        ShaderClasses_[Object.Id] = ShaderClassObj;
        
        setupShaderConstants(VertShd, Object.Shaders[0]);
        setupShaderConstants(FragShd, Object.Shaders[1]);
        setupShaderConstants(GeomShd, Object.Shaders[2]);
    }
    
    return true;
}


/*
 * ======= Protected: setup/ notification functions =======
 */

io::stringc SceneLoaderSPSB::getFinalPath(const io::stringc &Path) const
{
    if (!Path.size() || !ResourcePath_.size() || !(Flags_ & SCENEFLAG_ABSOLUTEPATH))
        return Path;
    
    const io::stringc AbsolutePath(ResourcePath_ + Path);
    
    if (!io::FileSystem().findFile(AbsolutePath))
        return Path;
    
    return AbsolutePath;
}

void SceneLoaderSPSB::applyQueues()
{
    /* Apply parent queue */
    foreach (const SParentQueue &Queue, QueueParents_)
    {
        if (Queue.Object)
        {
            std::map<u32, SceneNode*>::iterator it = ObjectIdMap_.find(Queue.ParentId);
            
            if (it != ObjectIdMap_.end())
                Queue.Object->setParent(it->second, true);
        }
    }
}

void SceneLoaderSPSB::addObjectToParentQueue(SceneNode* Node, u32 ParentId)
{
    if (ParentId)
    {
        SParentQueue Queue;
        {
            Queue.Object    = Node;
            Queue.ParentId  = ParentId;
        }
        QueueParents_.push_back(Queue);
    }
}

SpSceneFormatHandler::SpTextureClass* SceneLoaderSPSB::findTextureClass(u32 Id)
{
    if (Id > 0)
    {
        std::map<u32, SpTextureClass>::iterator it = TextureClasses_.find(Id);
        if (it != TextureClasses_.end())
            return &(it->second);
        else
            Error(("Wrong ID number for texture class (" + io::stringc(Id) + ")").str());
    }
    return 0;
}

video::Texture* SceneLoaderSPSB::findTexture(u32 Id)
{
    return findObjectById<video::Texture>(Id, Textures_, "texture");
}

video::ShaderClass* SceneLoaderSPSB::findShaderClass(u32 Id)
{
    return findObjectById<video::ShaderClass>(Id, ShaderClasses_, "shader class");
}

Transformation SceneLoaderSPSB::findAnimNodeTransformation(u32 Id)
{
    if (Id > 0)
    {
        std::map<u32, Transformation>::iterator it = AnimNodeTransMap_.find(Id);
        if (it != AnimNodeTransMap_.end())
            return it->second;
        else
            Error(("Wrong ID number for animation node (" + io::stringc(Id) + ")").str());
    }
    return Transformation();
}

bool SceneLoaderSPSB::setupBaseObject(SceneNode* Node, const SpBaseObject &Object)
{
    if (!Node)
    {
        Error("Invalid arguments for base object");
        return false;
    }
    
    /* Setup name and visibility */
    Node->setName(Object.Name);
    Node->setVisible(Object.Visible != 0);
    
    /* Setup transformation */
    Node->setPosition       (convert(Object.Position));
    Node->setRotationMatrix (convert(Object.Rotation));
    Node->setScale          (convert(Object.Scaling ));
    
    /* Setup view culling */
    setupViewCulling(Node, Object.ViewCulling);
    
    /* Store object ID in hash-map */
    ObjectIdMap_[Object.Id] = Node;
    
    /* Add parent queue */
    addObjectToParentQueue(Node, Object.ParentId);
    
    return true;
}

bool SceneLoaderSPSB::setupViewCulling(SceneNode* Node, const SpViewCulling &Object)
{
    if (!Node)
    {
        Error("Invalid arguments for view culling object");
        return false;
    }
    
    BoundingVolume* BoundVolume = &(Node->getBoundingVolume());
    
    BoundVolume->setType(static_cast<EBoundingVolumes>(Object.BoundingType));
    
    switch (Object.BoundingType)
    {
        case BOUNDING_BOX:
        {
            if (Node->getType() == NODE_MESH)
            {
                Mesh* MeshObj = static_cast<Mesh*>(Node);
                dim::aabbox3df BoundBox(MeshObj->getMeshBoundingBox());
                
                BoundBox.Min *= Object.BoxSize;
                BoundBox.Max *= Object.BoxSize;
                
                BoundVolume->setBox(BoundBox);
            }
        }
        break;
        
        case BOUNDING_SPHERE:
        {
            BoundVolume->setRadius(Object.SphereRadius);
        }
        break;
    }
    
    return true;
}

bool SceneLoaderSPSB::setupAnimation(SceneNode* Node, const SpAnimationObject &Object)
{
    if (!Node)
    {
        Error("Invalid arguments for animation");
        return false;
    }
    
    foreach (const SpAnimation &Anim, Object.Animations)
    {
        /* Create node animation */
        NodeAnimation* AnimObj = __spSceneManager->createAnimation<NodeAnimation>(Anim.Name);
        Node->addAnimation(AnimObj);
        
        /* Setup animation flags */
        #if 0
        if (Anim.Flags & 0x01)
            AnimObj->setSplineTranslation(true);
        #endif
        
        bool DisableTranslation = ((Anim.Flags & 0x02) != 0);
        bool DisableRotation    = ((Anim.Flags & 0x04) != 0);
        bool DisableScaling     = ((Anim.Flags & 0x08) != 0);
        
        AnimObj->setSplineExpansion(Anim.SplineExpansion);
        
        /* Create animation keyframes */
        foreach (const SpAnimationKeyframe &Keyframe, Anim.Keyframes)
        {
            Transformation Trans(findAnimNodeTransformation(Keyframe.AnimNodeId));
            
            if (DisableTranslation)
                Trans.setPosition(Node->getPosition());
            if (DisableRotation)
                Trans.setRotation(Node->getRotationMatrix());
            if (DisableScaling)
                Trans.setScale(Node->getScale());
            
            AnimObj->addKeyframe(Trans, Keyframe.Duration);
        }
    }
    
    return true;
}

bool SceneLoaderSPSB::setupMaterial(video::MaterialStates* Material, const SpMaterial &Object)
{
    if (!Material)
    {
        Error("Invalid arguments for material");
        return false;
    }
    
    /* Setup material attributes */
    Material->setDiffuseColor(*(const video::color*)(&Object.DiffuseColor.r));
    Material->setAmbientColor(*(const video::color*)(&Object.AmbientColor.r));
    Material->setSpecularColor(*(const video::color*)(&Object.SpecularColor.r));
    Material->setEmissionColor(*(const video::color*)(&Object.EmissionColor.r));
    
    Material->setPolygonOffset(
        Object.PolygonOffset != 0, Object.PolygonOffsetFactor, Object.PolygonOffsetUnits
    );
    Material->setWireframe(
        static_cast<video::EWireframeTypes>(Object.WireframeFront),
        static_cast<video::EWireframeTypes>(Object.WireframeBack)
    );
    
    Material->setShininess(Object.Shininess);
    Material->setAlphaReference(Object.AlphaReference);
    Material->setDepthMethod(static_cast<video::ESizeComparisionTypes>(Object.DepthMethod));
    Material->setAlphaMethod(static_cast<video::ESizeComparisionTypes>(Object.AlphaMethod));
    Material->setBlendSource(static_cast<video::EBlendingTypes>(Object.BlendSource));
    Material->setBlendTarget(static_cast<video::EBlendingTypes>(Object.BlendTarget));
    Material->setRenderFace(static_cast<video::EFaceTypes>(Object.RenderFace));
    
    Material->setColorMaterial(Object.ColorMaterial != 0);
    Material->setLighting(Object.Lighting != 0);
    Material->setBlending(Object.Blending != 0);
    Material->setDepthBuffer(Object.DepthBuffer != 0);
    Material->setFog(Object.Fog != 0);
    
    if (Object.Shading < 2)
        Material->setShading(static_cast<video::EShadingTypes>(Object.Shading));
    
    return true;
}

bool SceneLoaderSPSB::setupSurface(
    Mesh* MeshObj,video::MeshBuffer* Surface, const SpSurface &Object, u32 Index)
{
    if (!MeshObj || !Surface)
    {
        Error("Invalid arguments for mesh surface");
        return false;
    }
    
    /* Setup surface attribtue */
    Surface->setName                (Object.Name            );
    Surface->setHardwareInstancing  (Object.InstanceCount   );
    
    /* Setup mesh buffer format */
    if (!setupMeshBufferFormat(Surface, getVertexFormat(Object.VertexFormat), static_cast<video::ERendererDataTypes>(Object.IndexFormat)))
        return false;
    
    /* Setup surface texture layers */
    SpTextureClass* TexClass = findTextureClass(Object.TexClassId);
    
    u8 l = 0;
    foreach (const SpSurfaceLayer &Layer, Object.Layers)
    {
        /* Setup texture coordinate generation */
        if (Layer.AutoMapMode)
            MeshObj->textureAutoMap(l, Layer.Density, Index, Layer.AutoMapMode == 2);
        
        /* Setup texture mapping */
        video::Texture* Tex = findTexture(Layer.TexId);
        
        if (Tex)
            setupSurfaceTexture(Surface, Tex, Layer.TexId, l);
        
        if (TexClass && static_cast<u32>(l) < TexClass->Layers.size())
            setupSurfaceTextureClass(Surface, TexClass->Layers[l], Layer.TexId == 0, l);
        
        ++l;
    }
    
    return true;
}

bool SceneLoaderSPSB::setupSurfaceTexture(video::MeshBuffer* Surface, video::Texture* Tex, u32 TexId, u8 Layer)
{
    if (!Surface)
    {
        Error("Invalid arguments for surface texture");
        return false;
    }
    
    if (Surface->getTexture(Layer))
        Surface->setTexture(Layer, Tex);
    else
        Surface->addTexture(Tex, Layer);
    
    return true;
}

bool SceneLoaderSPSB::setupSurfaceTextureClass(
    video::MeshBuffer* Surface, const SpTextureClassLayer &TexClassLayer, bool NeedDefaultTex, u8 Layer)
{
    if (!Surface)
    {
        Error("Invalid arguments for surface texture class");
        return false;
    }
    
    /* Setup default texture */
    if (NeedDefaultTex)
    {
        video::Texture* Tex = findTexture(TexClassLayer.DefaultTexId);
        
        if (Tex)
            setupSurfaceTexture(Surface, Tex, 0, Layer);
    }
    
    /* Setup texture configuration from texture class */
    Surface->setTextureEnv(Layer, static_cast<video::ETextureEnvTypes>(TexClassLayer.Environment));
    Surface->setMappingGen(Layer, static_cast<video::EMappingGenTypes>(TexClassLayer.MappingGen ));
    
    return true;
}

bool SceneLoaderSPSB::setupMeshBufferFormat(
    video::MeshBuffer* Surface, video::VertexFormat* VxFormat, const video::ERendererDataTypes IxFormat)
{
    if (!Surface)
    {
        Error("Invalid arguments for mesh buffer format");
        return false;
    }
    
    Surface->setVertexFormat(VxFormat);
    Surface->setIndexFormat(IxFormat);
    
    return true;
}

bool SceneLoaderSPSB::setupMeshCollision(Mesh* MeshObj, const ECollisionModels CollModel, const EPickingTypes PickModel)
{
    return true; // todo
}

bool SceneLoaderSPSB::setupMeshShader(Mesh* MeshObj, u32 ShaderClassId)
{
    if (ShaderClassId > 0)
    {
        if (!MeshObj)
        {
            Error("Invalid arguments for mesh shader");
            return false;
        }
        MeshObj->setShaderClass(findShaderClass(ShaderClassId));
    }
    return true;
}

bool SceneLoaderSPSB::setupScriptTemplates(SceneNode* Node, const SpBaseObject &Object, const SpScriptData &Script)
{
    return true; // todo
}

bool SceneLoaderSPSB::setupTexture(video::Texture* Tex, const SpTexture &Object)
{
    if (!Tex)
    {
        Error("Invalid arguments for texture");
        return false;
    }
    
    /* Setup image buffer */
    video::ImageBuffer* ImgBuffer = Tex->getImageBuffer();
    {
        ImgBuffer->setFormat(static_cast<video::EPixelFormats>(Object.Format));
        
        if (Object.ColorKey.a < 255)
            ImgBuffer->setColorKey(convert(Object.ColorKey));
    }
    Tex->updateImageBuffer();
    
    /* Setup texture settings */
    Tex->setHardwareFormat(static_cast<video::EHWTextureFormats>(Object.HWFormat));
    Tex->setFilter(static_cast<video::ETextureFilters>(Object.Filter));
    Tex->setMipMapFilter(static_cast<video::ETextureMipMapFilters>(Object.MipMapFilter));
    Tex->setAnisotropicSamples(Object.Anisotropy);
    Tex->setMipMapping(Object.HasMipMaps != 0);
    Tex->setWrapMode(
        static_cast<video::ETextureWrapModes>(Object.WrapModeX),
        static_cast<video::ETextureWrapModes>(Object.WrapModeY),
        static_cast<video::ETextureWrapModes>(Object.WrapModeZ)
    );
    Tex->setDimension(static_cast<video::ETextureDimensions>(Object.Dimension));
    Tex->setRenderTarget(Object.RenderTarget != 0);
    
    return true;
}

bool SceneLoaderSPSB::setupLightmapSceneSurface(video::MeshBuffer* Surface, const SpLightmapSceneSurface &Object)
{
    if (!Surface)
    {
        Error("Invalid arguments for lightmap scene surface");
        return false;
    }
    
    /* Setup surface settings */
    Surface->setName(Object.Name);
    Surface->setIndexBufferEnable(false);
    
    /* Setup mesh buffer format */
    if (!setupMeshBufferFormat(Surface, getVertexFormat(Object.VertexFormat), static_cast<video::ERendererDataTypes>(Object.IndexFormat)))
        return false;
    
    /* Build mesh buffer */
    u8 LayerCount = math::Min<u8>(static_cast<u8>(Object.Layers.size()), 7);
    u32 i = 0;
    
    foreach (const SpLightmapSceneVertex &Vert, Object.Vertices)
    {
        Surface->addVertex(convert(Vert.Coordinate), convert(Vert.Normal), convert(Vert.LightmapTexCoord));
        
        for (u8 l = 0; l < LayerCount; ++l)
            Surface->setVertexTexCoord(i, convert(Vert.TexCoords[l]), l);
        
        ++i;
    }
    
    Surface->updateVertexBuffer();
    
    /* Setup textures */
    u8 l = 0;
    
    foreach (const SpLightmapSceneLayer &Layer, Object.Layers)
    {
        video::Texture* Tex = findTexture(Layer.TexId);
        
        if (Tex)
        {
            setupSurfaceTexture(Surface, Tex, Layer.TexId, l);
            
            /* Setup texture configuration from texture class */
            Surface->setTextureEnv(l, static_cast<video::ETextureEnvTypes>(Layer.Environment));
            Surface->setMappingGen(l, static_cast<video::EMappingGenTypes>(Layer.MappingGen ));
        }
        
        ++l;
    }
    
    /* Setup lightmap texture */
    if (Object.LightmapTexIndex < LightmapTextures_.size())
        Surface->addTexture(LightmapTextures_[Object.LightmapTexIndex]);
    else
    {
        Error("Lightmap texture index out of range");
        return false;
    }
    
    return true;
}

bool SceneLoaderSPSB::setupShaderConstants(video::Shader* ShaderObj, const SpShader &Object)
{
    if (!ShaderObj)
        return true;
    
    /* Setup shader constants */
    foreach (const SpShaderParameter &Param, Object.Parameters)
    {
        switch (Param.Type)
        {
            case SHADERPARAM_FLOAT:
                ShaderObj->setConstant(Param.Name, Param.ValueFlt); break;
            case SHADERPARAM_INT:
                ShaderObj->setConstant(Param.Name, Param.ValueInt); break;
            case SHADERPARAM_BOOL:
                ShaderObj->setConstant(Param.Name, Param.ValueBool); break;
            case SHADERPARAM_VEC2:
                ShaderObj->setConstant(Param.Name, convert(Param.ValueVec2)); break;
            case SHADERPARAM_VEC3:
                ShaderObj->setConstant(Param.Name, convert(Param.ValueVec3)); break;
            case SHADERPARAM_VEC4:
                ShaderObj->setConstant(Param.Name, convert(Param.ValueVec4)); break;
            case SHADERPARAM_MATRIX:
            default:
                break;
        }
    }
    
    return true;
}

bool SceneLoaderSPSB::completeMeshConstruct(Mesh* MeshObj, const SpMesh &Object)
{
    return true; // do noghting
}
bool SceneLoaderSPSB::completeCameraConstruct(Camera* CameraObj, const SpCamera &Object)
{
    return true; // do noghting
}
bool SceneLoaderSPSB::completeLightConstruct(Light* LightObj, const SpLight &Object)
{
    return true; // do noghting
}
bool SceneLoaderSPSB::completeSpriteConstruct(Billboard* SpriteObj, const SpSprite&Object)
{
    return true; // do noghting
}


/*
 * ======= Protected: creation functions =======
 */

Mesh* SceneLoaderSPSB::createSkyBox(const std::string (&SkyBoxTexFilenames)[6])
{
    /* Check if skybox is used */
    for (s32 i = 0; i < 6; ++i)
    {
        if (!SkyBoxTexFilenames[i].size())
            return 0;
    }
    
    /* Read skybox texture filenames */
    video::Texture* TexList[6] = { 0 };
    
    __spVideoDriver->setTextureGenFlags(video::TEXGEN_WRAP, video::TEXWRAP_CLAMP);
    
    for (s32 i = 0; i < 6; ++i)
        TexList[i] = __spVideoDriver->loadTexture(getFinalPath(SkyBoxTexFilenames[i]));
    
    __spVideoDriver->setTextureGenFlags(video::TEXGEN_WRAP, video::TEXWRAP_REPEAT);
    
    /* Create skybox */
    Mesh* SkyBox = __spSceneManager->createSkyBox(TexList);
    SkyBox->setName("skybox");
    
    return SkyBox;
}

Mesh* SceneLoaderSPSB::createMeshBasic(const SpMeshConstructionBasic &Construct)
{
    return __spSceneManager->createMesh(
        static_cast<EBasicMeshes>(Construct.BasicMeshType),
        SMeshConstruct(
            Construct.SegsVert,
            Construct.SegsHorz,
            Construct.InnerRadius,
            Construct.OuterRadius,
            Construct.RotationDeg,
            Construct.RotationDist,
            Construct.HasCap != 0,
            static_cast<video::EShadingTypes>(Construct.Shading)
        )
    );
}

Mesh* SceneLoaderSPSB::createMeshResource(const SpMeshConstructionResource &Construct)
{
    return __spSceneManager->loadMesh(getFinalPath(Construct.Filename));
}

video::Shader* SceneLoaderSPSB::createShader(
    const SpShader &Object, video::ShaderClass* ShaderClassObj, const video::EShaderTypes Type)
{
    if (!ShaderClassObj || !Object.ShaderCode.size())
        return 0;
    
    /* Create shader object */
    std::list<io::stringc> ShaderBuffer;
    ShaderBuffer.push_back(Object.ShaderCode);
    
    return __spVideoDriver->createShader(
        ShaderClassObj,
        Type,
        static_cast<video::EShaderVersions>(Object.Version),
        ShaderBuffer,
        Object.EntryPoint
    );
}

video::VertexFormat* SceneLoaderSPSB::getVertexFormat(s8 VertexFormat)
{
    video::VertexFormat* VxFormat = 0;
    
    switch (VertexFormat)
    {
        case 0: VxFormat = __spVideoDriver->getVertexFormatDefault();   break;
        case 1: VxFormat = __spVideoDriver->getVertexFormatReduced();   break;
        case 2: VxFormat = __spVideoDriver->getVertexFormatExtended();  break;
        case 3: VxFormat = __spVideoDriver->getVertexFormatFull();      break;
    }
    
    return VxFormat;
}


} // /namespace scene

} // /namespace sp


#endif



// ================================================================================
