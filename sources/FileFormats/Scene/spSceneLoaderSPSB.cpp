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
extern audio::SoundDevice* __spSoundDevice;

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
        Warning("Mesh object is invalid");
        return true;
    }
    
    /* Setup object */
    setupBaseObject(MeshObj, Object.BaseObject);
    setupMaterial(MeshObj->getMaterial(), Object.Material);
    
    if (Object.Material.GetShadow && (Flags_ & SCENEFLAG_LIGHTMAPS) && HasLightmaps_)
        MeshObj->setVisible(false);
    if (Object.Material.Shading < 2)
        MeshObj->updateNormals();
    
    /* Setup surfaces and their texture layers */
    for (u32 s = 0; s < Object.Surfaces.size(); ++s)
        setupSurface(MeshObj, MeshObj->getMeshBuffer(s), Object.Surfaces[s], s);
    
    /* Setup collision and picking models */
    setupMeshCollision(
        MeshObj,
        static_cast<ECollisionModels>(Object.Collision.CollisionModel),
        static_cast<EPickingTypes>(Object.Collision.PickingModel)
    );
    
    /* Setup script templates, animation and complete mesh construction */
    setupScriptTemplates(MeshObj, Object.BaseObject, Object.ScriptData);
    setupAnimation(MeshObj, Object.AnimObject);
    
    completeMeshConstruct(MeshObj, Object);
    
    return true;
}

bool SceneLoaderSPSB::CatchCamera(const SpCamera &Object)
{
    /* Create camera object */
    Camera* CameraObj = __spSceneManager->createCamera();
    
    CameraObj->setFOV(Object.FieldOfView);
    CameraObj->setRange(Object.NearPlane, Object.FarPlane);
    
    /* Setup object */
    setupBaseObject(CameraObj, Object.BaseObject);
    setupScriptTemplates(CameraObj, Object.BaseObject, Object.ScriptData);
    setupAnimation(CameraObj, Object.AnimObject);
    
    completeCameraConstruct(CameraObj, Object);
    
    return true;
}

bool SceneLoaderSPSB::CatchWayPoint(const SpWayPoint &Object)
{
    
    return true;
}

bool SceneLoaderSPSB::CatchLight(const SpLight &Object)
{
    /* Create light object */
    Light* LightObj = __spSceneManager->createLight(static_cast<ELightModels>(Object.LightModel));
    
    LightObj->setVolumetric(Object.Volumetric != 0);
    LightObj->setVolumetricRadius(Object.VolumetricRadius);
    LightObj->setLightingColor(*(const video::color*)(&Object.LightColor.r));
    LightObj->setSpotCone(Object.InnerSpotCone, Object.OuterSpotCone);
    
    /* Setup object */
    setupBaseObject(LightObj, Object.BaseObject);
    setupScriptTemplates(LightObj, Object.BaseObject, Object.ScriptData);
    setupAnimation(LightObj, Object.AnimObject);
    
    completeLightConstruct(LightObj, Object);
    
    return true;
}

bool SceneLoaderSPSB::CatchBoundVolume(const SpBoundVolume &Object)
{
    
    return true;
}

bool SceneLoaderSPSB::CatchSound(const SpSound &Object)
{
    
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
    setupBaseObject(SpriteObj, Object.BaseObject);
    setupAnimation(SpriteObj, Object.AnimObject);
    
    completeSpriteConstruct(SpriteObj, Object);
    
    return true;
}

bool SceneLoaderSPSB::CatchAnimNode(const SpAnimNode &Object)
{
    /* Create animation node object */
    AnimNodeTransMap_[Object.BaseObject.Id] = KeyframeTransformation(
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
        Tex = __spVideoDriver->loadTexture(Object.Filename);
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
    
    return true;
}

bool SceneLoaderSPSB::CatchLightmapScene(const SpLightmapScene &Object)
{
    
    return true;
}

bool SceneLoaderSPSB::CatchShaderClass(const SpShaderClass &Object)
{
    
    return true;
}

io::stringc SceneLoaderSPSB::getFinalPath(const io::stringc &Path) const
{
    if (!Path.size() || !ResourcePath_.size() || !(Flags_ & SCENEFLAG_ABSOLUTEPATH))
        return Path;
    
    const io::stringc AbsolutePath(ResourcePath_ + Path);
    
    if (!io::FileSystem().findFile(AbsolutePath))
        return Path;
    
    return AbsolutePath;
}


/*
 * ======= Protected: setup/ notification functions =======
 */

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
    if (Id > 0)
    {
        std::map<u32, video::Texture*>::iterator it = Textures_.find(Id);
        if (it != Textures_.end())
            return it->second;
        else
            Error(("Wrong ID number for texture (" + io::stringc(Id) + ")").str());
    }
    return 0;
}

KeyframeTransformation SceneLoaderSPSB::findAnimNodeTransformation(u32 Id)
{
    if (Id > 0)
    {
        std::map<u32, KeyframeTransformation>::iterator it = AnimNodeTransMap_.find(Id);
        if (it != AnimNodeTransMap_.end())
            return it->second;
        else
            Error(("Wrong ID number for animation node (" + io::stringc(Id) + ")").str());
    }
    return KeyframeTransformation();
}

void SceneLoaderSPSB::setupBaseObject(SceneNode* Node, const SpBaseObject &Object)
{
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
}

void SceneLoaderSPSB::setupViewCulling(SceneNode* Node, const SpViewCulling &Object)
{
    Node->setBoundingType(static_cast<EBoundingVolumes>(Object.BoundingType));
    
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
                
                Node->setBoundingBox(BoundBox);
            }
            else
                Node->setBoundingBox(dim::aabbox3df(-0.5f, 0.5f));
        }
        break;
        
        case BOUNDING_SPHERE:
        {
            Node->setBoundingSphere(Object.SphereRadius);
        }
        break;
    }
}

void SceneLoaderSPSB::setupAnimation(SceneNode* Node, const SpAnimationObject &Object)
{
    if (!Node)
        return;
    
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
            KeyframeTransformation Trans(findAnimNodeTransformation(Keyframe.AnimNodeId));
            
            if (DisableTranslation)
                Trans.setPosition(Node->getPosition());
            if (DisableRotation)
                Trans.setRotation(Node->getRotationMatrix());
            if (DisableScaling)
                Trans.setScale(Node->getScale());
            
            AnimObj->addKeyframe(Trans, Keyframe.Duration);
        }
    }
}

void SceneLoaderSPSB::setupMaterial(video::MaterialStates* Material, const SpMaterial &Object)
{
    if (!Material)
        return;
    
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
}

void SceneLoaderSPSB::setupSurface(
    Mesh* MeshObj,video::MeshBuffer* Surface, const SpSurface &Object, u32 Index)
{
    if (!MeshObj || !Surface)
        return;
    
    /* Setup surface attribtue */
    Surface->setName                (Object.Name            );
    Surface->setHardwareInstancing  (Object.InstanceCount   );
    
    /* Setup mesh buffer format */
    video::VertexFormat* VxFormat = 0;
    
    switch (Object.VertexFormat)
    {
        case 0: VxFormat = __spVideoDriver->getVertexFormatDefault();   break;
        case 1: VxFormat = __spVideoDriver->getVertexFormatReduced();   break;
        case 2: VxFormat = __spVideoDriver->getVertexFormatExtended();  break;
        case 3: VxFormat = __spVideoDriver->getVertexFormatFull();      break;
    }
    
    setupMeshBufferFormat(
        Surface, VxFormat, static_cast<video::ERendererDataTypes>(Object.IndexFormat)
    );
    
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
}

void SceneLoaderSPSB::setupSurfaceTexture(video::MeshBuffer* Surface, video::Texture* Tex, u32 TexId, u8 Layer)
{
    if (Surface)
    {
        if (Surface->getTexture(Layer))
            Surface->setTexture(Layer, Tex);
        else
            Surface->addTexture(Tex, Layer);
    }
}

void SceneLoaderSPSB::setupSurfaceTextureClass(
    video::MeshBuffer* Surface, const SpTextureClassLayer &TexClassLayer, bool NeedDefaultTex, u8 Layer)
{
    if (!Surface)
        return;
    
    /* Setup texture configuration from texture class */
    Surface->setTextureEnv(Layer, static_cast<video::ETextureEnvTypes>(TexClassLayer.Environment));
    Surface->setMappingGen(Layer, static_cast<video::EMappingGenTypes>(TexClassLayer.MappingGen ));
    
    /* Setup default texture */
    if (NeedDefaultTex)
    {
        video::Texture* Tex = findTexture(TexClassLayer.DefaultTexId);
        
        if (Tex)
            setupSurfaceTexture(Surface, Tex, 0, Layer);
    }
}

void SceneLoaderSPSB::setupMeshBufferFormat(
    video::MeshBuffer* Surface, video::VertexFormat* VxFormat, const video::ERendererDataTypes IxFormat)
{
    if (Surface)
    {
        Surface->setVertexFormat(VxFormat);
        Surface->setIndexFormat(IxFormat);
    }
}

void SceneLoaderSPSB::setupMeshCollision(Mesh* MeshObj, const ECollisionModels CollModel, const EPickingTypes PickModel)
{
    // todo
}

void SceneLoaderSPSB::setupScriptTemplates(SceneNode* Node, const SpBaseObject &Object, const SpScriptData &Script)
{
    // todo
}

void SceneLoaderSPSB::setupTexture(video::Texture* Tex, const SpTexture &Object)
{
    if (!Tex)
        return;
    
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
}

void SceneLoaderSPSB::completeMeshConstruct(Mesh* MeshObj, const SpMesh &Object)
{
    // do noghting
}
void SceneLoaderSPSB::completeCameraConstruct(Camera* CameraObj, const SpCamera &Object)
{
    // do noghting
}
void SceneLoaderSPSB::completeLightConstruct(Light* LightObj, const SpLight &Object)
{
    // do noghting
}
void SceneLoaderSPSB::completeSpriteConstruct(Billboard* SpriteObj, const SpSprite&Object)
{
    // do noghting
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


} // /namespace scene

} // /namespace sp


#endif



// ================================================================================
