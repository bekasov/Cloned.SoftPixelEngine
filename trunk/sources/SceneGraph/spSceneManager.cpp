/*
 * Scene manager file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "SceneGraph/spSceneManager.hpp"
#include "SceneGraph/spSceneMesh.hpp"
#include "SceneGraph/spSceneBillboard.hpp"
#include "SceneGraph/spSceneTerrain.hpp"
#include "SceneGraph/spSceneLight.hpp"
#include "SceneGraph/spSceneCamera.hpp"
#include "Base/spSharedObjects.hpp"
#include "Base/spBaseExceptions.hpp"
#include "Base/spBasicMeshGenerator.hpp"
#include "FileFormats/Mesh/spMeshFileFormats.hpp"
#include "RenderSystem/spRenderSystem.hpp"

#include <boost/foreach.hpp>


namespace sp
{

extern video::RenderSystem* __spVideoDriver;

namespace scene
{


const video::VertexFormat* SceneManager::DefaultVertexFormat_ = 0;
video::ERendererDataTypes SceneManager::DefaultIndexFormat_ = video::DATATYPE_UNSIGNED_SHORT;

bool SceneManager::TextureLoadingState_ = true;

SceneManager::SceneManager()
{
}
SceneManager::~SceneManager()
{
    /* Delete all animations and scene nodes */
    clearAnimations();
    clearScene();
}

/* === Meshes: === */

Mesh* SceneManager::createMesh()
{
    Mesh* NewMesh = MemoryManager::createMemory<Mesh>("scene::Mesh (Empty)");
    MeshList_.push_back(NewMesh);
    return NewMesh;
}

Mesh* SceneManager::createMesh(const EBasicMeshes Model, const SMeshConstruct &BuildConstruct)
{
    Mesh* NewMesh = createMesh();
    
    /* Create mesh buffer and construct 3D model */
    video::MeshBuffer* Surface = NewMesh->createMeshBuffer(SceneManager::getDefaultVertexFormat(), SceneManager::getDefaultIndexFormat());
    
    if (Surface)
        MeshGenerator::createMesh(*Surface, Model, BuildConstruct);
    
    return NewMesh;
}

Mesh* SceneManager::createSuperShape(const f32 (&ValueList)[12], s32 Detail)
{
    Mesh* NewMesh = createMesh();
    //!!!
    BasicMeshGenerator().createSuperShape(NewMesh, ValueList, Detail);
    return NewMesh;
}

Mesh* SceneManager::createSkyBox(video::Texture* (&TextureList)[6], f32 Radius)
{
    Mesh* NewMesh = createMesh();
    MeshGenerator::createSkyBox(*NewMesh, TextureList, Radius);
    return NewMesh;
}

Mesh* SceneManager::createHeightField(const video::Texture* TexHeightMap, const s32 Segments)
{
    //!!!
    Mesh* NewMesh = BasicMeshGenerator().createHeightField(TexHeightMap, Segments);
    MeshList_.push_back(NewMesh);
    return NewMesh;
}

Mesh* SceneManager::createMeshList(std::list<Mesh*> MergeList, bool isOldDelete)
{
    /* Allocate a new mesh */
    Mesh* NewMesh = MemoryManager::createMemory<Mesh>("scene::Mesh (List)");
    
    /* Temporary variables */
    u32 Indices[3];
    u32 s = 0, i = 0, k;
    dim::matrix4f NormalMatrix;
    
    std::list<video::Texture*> TexList;
    
    video::MeshBuffer* Surface = 0, * NewSurface = 0;
    
    // Loop all meshes in the list
    foreach (Mesh* Obj, MergeList)
    {
        /* Matrix transformation */
        spWorldMatrix = Obj->getTransformMatrix(true);
        
        NormalMatrix = spWorldMatrix;
        NormalMatrix.setPosition(0);
        
        /* Loop the surfaces of the current mesh */
        for (s = 0; s < Obj->getMeshBufferCount(); ++s)
        {
            Surface = Obj->getMeshBuffer(s);
            
            /* Create a new surface */
            NewSurface = NewMesh->createMeshBuffer(
                Surface->getVertexFormat(), Surface->getIndexFormat()->getDataType()
            );
            
            NewSurface->setName(Surface->getName().size() ? Surface->getName() : Obj->getName());
            
            /* Get the texture list */
            TexList = Surface->getTextureList();
            
            /* Add all textures */
            for (std::list<video::Texture*>::iterator itTex = TexList.begin(); itTex != TexList.end(); ++itTex)
                NewSurface->addTexture(*itTex);
            
            /* Add all vertices */
            for (i = 0; i < Surface->getVertexCount(); ++i)
            {
                k = NewSurface->addVertex(
                    spWorldMatrix       * Surface->getVertexCoord(i),
                    (NormalMatrix       * Surface->getVertexNormal(i)).normalize(),
                    spTextureMatrix[0]  * Surface->getVertexTexCoord(i),
                    Surface->getVertexColor(i),
                    Surface->getVertexFog(i)
                );
                
                /* Set texture coordinates */
                for (u8 j = 1; j < 8; ++j)
                {
                    NewSurface->setVertexTexCoord(
                        k, spTextureMatrix[j] * Surface->getVertexTexCoord(i, j), j
                    );
                }
            }
            
            /* Add all triangles */
            for (i = 0; i < Surface->getTriangleCount(); ++i)
            {
                Surface->getTriangleIndices(i, Indices);
                NewSurface->addTriangle(Indices[0], Indices[1], Indices[2]);
            }
        }
    }
    
    /* Delete the original meshs */
    if (isOldDelete)
    {
        foreach (Mesh* Obj, MergeList)
            deleteNode(Obj);
    }
    
    /* Update mesh buffer and add the mesh to the list */
    NewMesh->updateMeshBuffer();
    
    MeshList_.push_back(NewMesh);
    
    return NewMesh;
}

Mesh* SceneManager::createMeshSurface(Mesh* Model, u32 Surface)
{
    /* Check if surface is not out of range */
    if (!Model || Surface >= Model->getMeshBufferCount())
    {
        io::Log::warning("Invalid parameters for mesh/surface construction");
        return 0;
    }
    
    /* Allocate a new mesh */
    Mesh* NewMesh = MemoryManager::createMemory<Mesh>("scene::Mesh (Surface)");
    
    /* Temporary variables */
    u32 Indices[3];
    video::Texture* Tex = 0;
    
    /* Create a new surface */
    video::MeshBuffer* NewSurface = NewMesh->createMeshBuffer();
    
    /* Get current surface */
    video::MeshBuffer* OldSurface = Model->getMeshBuffer(Surface);
    
    /* Loop for all vertices of the specified surface */
    for (u32 i = 0; i < OldSurface->getVertexCount(); ++i)
    {
        NewSurface->addVertex(
            OldSurface->getVertexCoord(i),
            OldSurface->getVertexNormal(i),
            OldSurface->getVertexTexCoord(i),
            OldSurface->getVertexColor(i),
            OldSurface->getVertexFog(i)
        );
    }
    
    /* Loop for all triangles of the specified surface */
    for (u32 i = 0; i < OldSurface->getTriangleCount(); ++i)
    {
        OldSurface->getTriangleIndices(i, Indices);
        NewSurface->addTriangle(Indices[0], Indices[1], Indices[2]);
    }
    
    /* Add all textures of the specified surface */
    for (s32 i = 0; i < MAX_COUNT_OF_TEXTURES; ++i)
    {
        if ( ( Tex = OldSurface->getTexture(i) ) != 0 )
            NewSurface->addTexture(Tex);
    }
    
    /* Update mesh buffer and add the mesh to the list */
    NewMesh->updateMeshBuffer();
    
    MeshList_.push_back(NewMesh);
    
    return NewMesh;
}

Mesh* SceneManager::loadMesh(io::stringc Filename, io::stringc TexturePath, const EMeshFileFormats Format)
{
    /* Information message */
    io::Log::message("Load mesh: \"" + Filename + "\"");
    io::Log::ScopedTab UnusedTab;
    
    /* Temporary variables/ constants */
    if (TexturePath == video::TEXPATH_IGNORE)
        TexturePath = Filename.getPathPart();
    
    MeshLoader* Loader = 0;
    
    /* === Check if the file does exist === */
    
    if (!io::FileSystem().findFile(Filename))
    {
        /* Create an empty mesh and return with error */
        io::Log::error("Could not find mesh file");
        return createMesh();
    }
    
    /* === Select format === */
    
    const EMeshFileFormats FileFormat = SceneManager::getMeshFileFormat(Filename, Format);
    
    switch (FileFormat)
    {
        #ifdef SP_COMPILE_WITH_MESHLOADER_SPM
        case MESHFORMAT_SPM:
            Loader = MemoryManager::createMemory<MeshLoaderSPM>("scene::MeshLoaderSPM"); break;
        #endif
        #ifdef SP_COMPILE_WITH_MESHLOADER_3DS
        case MESHFORMAT_3DS:
            Loader = MemoryManager::createMemory<MeshLoader3DS>("scene::MeshLoader3DS"); break;
        #endif
        #ifdef SP_COMPILE_WITH_MESHLOADER_MS3D
        case MESHFORMAT_MS3D:
            Loader = MemoryManager::createMemory<MeshLoaderMS3D>("scene::MeshLoaderMS3D"); break;
        #endif
        #ifdef SP_COMPILE_WITH_MESHLOADER_X
        case MESHFORMAT_X:
            Loader = MemoryManager::createMemory<MeshLoaderX>("scene::MeshLoaderX"); break;
        #endif
        #ifdef SP_COMPILE_WITH_MESHLOADER_B3D
        case MESHFORMAT_B3D:
            Loader = MemoryManager::createMemory<MeshLoaderB3D>("scene::MeshLoaderB3D"); break;
        #endif
        #ifdef SP_COMPILE_WITH_MESHLOADER_MD2
        case MESHFORMAT_MD2:
            Loader = MemoryManager::createMemory<MeshLoaderMD2>("scene::MeshLoaderMD2"); break;
        #endif
        #ifdef SP_COMPILE_WITH_MESHLOADER_MD3
        case MESHFORMAT_MD3:
            Loader = MemoryManager::createMemory<MeshLoaderMD3>("scene::MeshLoaderMD3"); break;
        #endif
        #ifdef SP_COMPILE_WITH_MESHLOADER_OBJ
        case MESHFORMAT_OBJ:
            Loader = MemoryManager::createMemory<MeshLoaderOBJ>("scene::MeshLoaderOBJ"); break;
        #endif
        
        default:
            /* Create an empty mesh and return with error */
            io::Log::error("Mesh has unsupported file format");
            return createMesh();
    }
    
    /* Load the model with the determined model loader */
    Mesh* NewMesh = Loader->loadMesh(Filename, TexturePath);
    MeshList_.push_back(NewMesh);
    
    /* Delete the temporary mesh loader */
    delete Loader;
    
    return NewMesh;
}

bool SceneManager::saveMesh(Mesh* Model, io::stringc Filename, const EMeshFileFormats Format)
{
    /* Information message */
    io::Log::error("Save model: \"" + Filename + "\"");
    io::Log::ScopedTab UnusedTab;
    
    MeshSaver* Saver = 0;
    
    /* === Select format === */
    
    const EMeshFileFormats FileFormat = SceneManager::getMeshFileFormat(Filename, Format);
    
    switch (FileFormat)
    {
        #ifdef SP_COMPILE_WITH_MESHSAVER_SPM
        case MESHFORMAT_SPM:
            Saver = MemoryManager::createMemory<MeshSaverSPM>("scene::MeshSaverSPM"); break;
        #endif
        #ifdef SP_COMPILE_WITH_MESHSAVER_B3D
        case MESHFORMAT_B3D:
            Saver = MemoryManager::createMemory<MeshSaverB3D>("scene::MeshSaverB3D"); break;
        #endif
        #ifdef SP_COMPILE_WITH_MESHSAVER_OBJ
        case MESHFORMAT_OBJ:
            Saver = MemoryManager::createMemory<MeshSaverOBJ>("scene::MeshSaverOBJ"); break;
        #endif
        
        default:
            /* Exit with a failure */
            io::Log::error("Mesh has unsupported file format");
            return false;
    }
    
    if (!Saver)
        return false;
    
    /* Save the model with the determined model saver */
    bool Result = Saver->saveMesh(Model, Filename);
    
    /* Delete the temporary mesh saver */
    delete Saver;
    
    return Result;
}

Mesh* SceneManager::loadScene(io::stringc Filename, io::stringc TexturePath, const ESceneFileFormats Format, const s32 Flags)
{
    /* Information message */
    io::Log::message("Load scene: \"" + Filename + "\"");
    io::Log::ScopedTab UnusedTab;
    
    if (TexturePath == video::TEXPATH_IGNORE)
        TexturePath = Filename.getPathPart();
    
    SceneLoader* Loader = 0;
    
    /* === Select format === */
    
    const ESceneFileFormats FileFormat = SceneManager::getSceneFileFormat(Filename, Format);
    
    switch (FileFormat)
    {
        #ifdef SP_COMPILE_WITH_SCENELOADER_SPSB
        case SCENEFORMAT_SPSB:
            Loader = MemoryManager::createMemory<SceneLoaderSPSB>("scene::SceneLoaderSPSB"); break;
        #endif
        #ifdef SP_COMPILE_WITH_SCENELOADER_BSP1
        case SCENEFORMAT_BSP1:
            Loader = MemoryManager::createMemory<SceneLoaderBSP1>("scene::SceneLoaderBSP1"); break;
        #endif
        #ifdef SP_COMPILE_WITH_SCENELOADER_BSP3
        case SCENEFORMAT_BSP3:
            Loader = MemoryManager::createMemory<SceneLoaderBSP3>("scene::SceneLoaderBSP3"); break;
        #endif
        
        default:
            /* Exit with failure */
            io::Log::error("Scene has unsupported file format");
            return createMesh();
    }
    
    /* Load the scene with the determined scene loader */
    Mesh* NewMesh = Loader->loadScene(Filename, TexturePath, Flags);
    MeshList_.push_back(NewMesh);
    
    /* Delete the temporary scene loader */
    delete Loader;
    
    return NewMesh;
}

Mesh* SceneManager::getMesh(const io::stringc &Filename, io::stringc TexturePath, const EMeshFileFormats Format)
{
    Mesh* NewMesh = 0;
    
    /* Search for the filename and check if the model has already been loaded */
    const std::string FilenameLCase(Filename.lower().str());
    
    std::map<std::string, Mesh*>::iterator it = MeshMap_.find(FilenameLCase);
    
    if (it != MeshMap_.end())
    {
        /* Instantiate the mesh */
        Mesh* InstanceMesh = it->second;
        
        NewMesh = createMesh();
        NewMesh->setReference(InstanceMesh);
    }
    else
    {
        /* Load mesh and store it in the map */
        NewMesh = loadMesh(Filename, TexturePath, Format);
        
        MeshMap_[FilenameLCase] = NewMesh;
    }
    
    return NewMesh;
}

//!TODO! -> move this to "MeshGenerator" namespace
void SceneManager::createFurMesh(
    Mesh* Model, video::Texture* FurTexture, s32 LayerCount, f32 HairLength, s32 HairCloseness)
{
    if (!Model || !Model->getMeshBufferCount() || !FurTexture || !FurTexture->getImageBuffer() || LayerCount < 2)
    {
        #ifdef SP_DEBUGMODE
        io::Log::debug("SceneManager::createFurMesh");
        #endif
        return;
    }
    
    /* === Create the hair texture (for layers) === */
    
    /* Copy the fur texture */
    video::Texture* HairTexture = __spVideoDriver->copyTexture(FurTexture);
    
    /* Change the type for the alpha-channel */
    HairTexture->setFormat(video::PIXELFORMAT_RGBA);
    
    video::ImageBuffer* ImgBuffer = HairTexture->getImageBuffer();
    
    /* Loop for the image data */
    for (s32 y = 0, x; y < HairTexture->getSize().Height; ++y)
    {
        for (x = 0; x < HairTexture->getSize().Width; ++x)
        {
            if (math::Randomizer::randInt(HairCloseness) != 1)
            {
                /* Change the pixel transparency */
                video::color Color( ImgBuffer->getPixelColor(dim::point2di(x, y)) );
                Color.Alpha = 0;
                ImgBuffer->setPixelColor(dim::point2di(x, y), Color);
            }
        }
    }
    
    /* Update the image data */
    HairTexture->updateImageBuffer();
    
    /* === Create the hair layers === */
    
    /* Allocate a layer list */
    std::vector<Mesh*> Layer;
    Layer.resize(LayerCount);
    video::MeshBuffer* Surface = Model->getMeshBuffer(0), * LayerSurface = 0;
    
    /* Temporary variables */
    dim::vector3df Coord, Normal;
    
    /* Loop for all layers */
    for (s32 i = 0; i < LayerCount; ++i)
    {
        Layer[i]        = copyNode(Model);
        LayerSurface    = Layer[i]->getMeshBuffer(0);
        
        Layer[i]->setParent(Model);
        Layer[i]->setPosition(0);
        Layer[i]->setScale(1);
        
        LayerSurface->setTexture(0, HairTexture);
        
        Layer[i]->getMaterial()->setColorMaterial(false);
        Layer[i]->getMaterial()->getDiffuseColor().Alpha = 255 - 255 * (i+1) / LayerCount;
        
        for (u32 j = 0; j < LayerSurface->getVertexCount(); ++j)
        {
            Coord   = Surface->getVertexCoord(j);
            Normal  = Surface->getVertexNormal(j);
            
            Coord += Normal * (HairLength * static_cast<f32>(i+1) / LayerCount);
            
            LayerSurface->setVertexCoord(j, Coord);
        }
        
        Layer[i]->updateMeshBuffer();
    }
}

/* === Other scene nodes: === */

SceneNode* SceneManager::createNode()
{
    SceneNode* NewNode = new SceneNode(NODE_BASICNODE);
    NodeList_.push_back(NewNode);
    return NewNode;
}

Light* SceneManager::createLight(const ELightModels Type)
{
    try
    {
        Light* NewLight = new Light(Type);
        LightList_.push_back(NewLight);
        return NewLight;
    }
    catch (const std::exception &Err)
    {
        io::Log::error(Err.what());
    }
    return 0;
}

Billboard* SceneManager::createBillboard(video::Texture* BaseTexture)
{
    Billboard* NewBillboard = new Billboard(BaseTexture);
    BillboardList_.push_back(NewBillboard);
    return NewBillboard;
}

Camera* SceneManager::createCamera()
{
    return createCamera<Camera>();
}

Terrain* SceneManager::createTerrain(
    const video::SHeightMapTexture &TextureHeightMap, const dim::size2di &Resolution, s32 GeoMIPLevels)
{
    Terrain* NewTerrain = new Terrain(TextureHeightMap, Resolution, GeoMIPLevels);
    TerrainList_.push_back(NewTerrain);
    return NewTerrain;
}

/* === Animations: === */

void SceneManager::deleteAnimation(Animation* Anim)
{
    MemoryManager::removeElement(AnimationList_, Anim, true);
}

void SceneManager::updateAnimations()
{
    foreach (Animation* Anim, AnimationList_)
    {
        if (Anim->playing())
        {
            foreach (SceneNode* Node, Anim->getSceneNodeList())
                Anim->updateAnimation(Node);
        }
    }
}

void SceneManager::clearAnimations()
{
    MemoryManager::deleteList(AnimationList_);
}

/* === Other scene functions: === */

void SceneManager::clearScene(
    bool isDeleteNodes, bool isDeleteMeshes, bool isDeleteCameras,
    bool isDeleteLights, bool isDeleteBillboards, bool isDeleteTerrains)
{
    if (isDeleteNodes)
        MemoryManager::deleteList(NodeList_);
    if (isDeleteMeshes)
    {
        MemoryManager::deleteList(MeshList_);
        MeshMap_.clear();
    }
    if (isDeleteCameras)
        MemoryManager::deleteList(CameraList_);
    if (isDeleteLights)
        MemoryManager::deleteList(LightList_);
    if (isDeleteBillboards)
        MemoryManager::deleteList(BillboardList_);
    if (isDeleteTerrains)
        MemoryManager::deleteList(TerrainList_);
}

SceneNode* SceneManager::copyNode(const SceneNode* TemplateObject)
{
    return copySceneNode(NodeList_, TemplateObject);
}
Mesh* SceneManager::copyNode(const Mesh* TemplateObject)
{
    return copySceneNode(MeshList_, TemplateObject);
}
Light* SceneManager::copyNode(const Light* TemplateObject)
{
    return copySceneNode(LightList_, TemplateObject);
}
Billboard* SceneManager::copyNode(const Billboard* TemplateObject)
{
    return copySceneNode(BillboardList_, TemplateObject);
}
Camera* SceneManager::copyNode(const Camera* TemplateObject)
{
    return copySceneNode(CameraList_, TemplateObject);
}
Terrain* SceneManager::copyNode(const Terrain* TemplateObject)
{
    #ifdef SP_DEBUGMODE
    io::Log::debug("SceneGraph::copyNode", "Not yet implemented for terrain objects");
    #endif
    return 0;
}

bool SceneManager::deleteNode(SceneNode* Object)
{
    if (!Object)
        return false;
    
    switch (Object->getType())
    {
        case NODE_BASICNODE:    deleteSceneNode(NodeList_,      Object);
        case NODE_CAMERA:       deleteSceneNode(CameraList_,    static_cast<Camera*     >(Object));
        case NODE_LIGHT:        deleteSceneNode(LightList_,     static_cast<Light*      >(Object));
        case NODE_MESH:         deleteSceneNode(MeshList_,      static_cast<Mesh*       >(Object));
        case NODE_BILLBOARD:    deleteSceneNode(BillboardList_, static_cast<Billboard*  >(Object));
        case NODE_TERRAIN:      deleteSceneNode(TerrainList_,   static_cast<Terrain*    >(Object));
        default:
            break;
    }
    
    return false;
}

std::list<SceneNode*> SceneManager::findNodes(const io::stringc &Name) const
{
    std::list<SceneNode*> NodeList;
    
    filterNodeByName<SceneNode> (Name, NodeList, NodeList_      );
    filterNodeByName<Camera>    (Name, NodeList, CameraList_    );
    filterNodeByName<Light>     (Name, NodeList, LightList_     );
    filterNodeByName<Mesh>      (Name, NodeList, MeshList_      );
    filterNodeByName<Billboard> (Name, NodeList, BillboardList_ );
    filterNodeByName<Terrain>   (Name, NodeList, TerrainList_   );
    
    return NodeList;
}

SceneNode* SceneManager::findNode(const io::stringc &Name) const
{
    SceneNode* Object = 0;
    
    if ( ( Object = findNodeInList<SceneNode>   (Name, NodeList_        ) ) != 0 ) return Object;
    if ( ( Object = findNodeInList<Camera>      (Name, CameraList_      ) ) != 0 ) return Object;
    if ( ( Object = findNodeInList<Light>       (Name, LightList_       ) ) != 0 ) return Object;
    if ( ( Object = findNodeInList<Mesh>        (Name, MeshList_        ) ) != 0 ) return Object;
    if ( ( Object = findNodeInList<Billboard>   (Name, BillboardList_   ) ) != 0 ) return Object;
    if ( ( Object = findNodeInList<Terrain>     (Name, TerrainList_     ) ) != 0 ) return Object;
    
    return 0;
}

std::list<SceneNode*> SceneManager::findChildren(const SceneNode* ParentNode) const
{
    std::list<SceneNode*> NodeList;
    
    addChildToList<SceneNode>   (ParentNode, NodeList, NodeList_        );
    addChildToList<Camera>      (ParentNode, NodeList, CameraList_      );
    addChildToList<Light>       (ParentNode, NodeList, LightList_       );
    addChildToList<Mesh>        (ParentNode, NodeList, MeshList_        );
    addChildToList<Billboard>   (ParentNode, NodeList, BillboardList_   );
    addChildToList<Terrain>     (ParentNode, NodeList, TerrainList_     );
    
    return NodeList;
}

SceneNode* SceneManager::findChild(const SceneNode* ParentNode, const io::stringc &Name) const
{
    SceneNode* Child = 0;
    
    if ( ( Child = findChildInList<SceneNode>   (ParentNode, NodeList_,         Name) ) != 0 ) return Child;
    if ( ( Child = findChildInList<Camera>      (ParentNode, CameraList_,       Name) ) != 0 ) return Child;
    if ( ( Child = findChildInList<Light>       (ParentNode, LightList_,        Name) ) != 0 ) return Child;
    if ( ( Child = findChildInList<Mesh>        (ParentNode, MeshList_,         Name) ) != 0 ) return Child;
    if ( ( Child = findChildInList<Billboard>   (ParentNode, BillboardList_,    Name) ) != 0 ) return Child;
    if ( ( Child = findChildInList<Terrain>     (ParentNode, TerrainList_,      Name) ) != 0 ) return Child;
    
    return 0;
}

void SceneManager::removeTexture(const video::Texture* Tex)
{
    if (!Tex)
        return;
    
    video::MeshBuffer* Surface = 0;
    
    /* Search in each mesh */
    foreach (Mesh* Obj, MeshList_)
    {
        for (u32 i = 0; i < Obj->getMeshBufferCount(); ++i)
        {
            Surface = Obj->getMeshBuffer(i);
            
            for (s32 j = MAX_COUNT_OF_TEXTURES - 1; j >= 0; --j)
            {
                if (Surface->getTexture(j) == Tex)
                    Surface->removeTexture(j);
            }
        }
    }
    
    /* Search in each billboard */
    foreach (Billboard* Obj, BillboardList_)
    {
        if (Obj->getTexture() && Obj->getTexture() == Tex)
            Obj->setTexture(0);
    }
}

u32 SceneManager::getSceneMeshBufferCount() const
{
    u32 SurfaceCount = 0;
    
    foreach (Mesh* Obj, MeshList_)
        SurfaceCount += Obj->getMeshBufferCount();
    
    return SurfaceCount;
}

u32 SceneManager::getSceneVertexCount() const
{
    u32 VertexCount = 0;
    
    foreach (Mesh* Obj, MeshList_)
        VertexCount += Obj->getVertexCount();
    
    return VertexCount;
}

u32 SceneManager::getSceneTriangleCount() const
{
    u32 TriangleCount = 0;
    
    foreach (Mesh* Obj, MeshList_)
        TriangleCount += Obj->getTriangleCount();
    
    return TriangleCount;
}

u32 SceneManager::getSceneObjectsCount() const
{
    return NodeList_.size() + CameraList_.size() + LightList_.size() + MeshList_.size() + BillboardList_.size() + TerrainList_.size();
}

/* === Static functions: === */

void SceneManager::setDefaultVertexFormat(const video::VertexFormat* Format)
{
    if (Format)
        DefaultVertexFormat_ = Format;
    else
        DefaultVertexFormat_ = __spVideoDriver->getVertexFormatDefault();
}
const video::VertexFormat* SceneManager::getDefaultVertexFormat()
{
    return DefaultVertexFormat_;
}

void SceneManager::setDefaultIndexFormat(const video::ERendererDataTypes Format)
{
    if ( ( Format == video::DATATYPE_UNSIGNED_BYTE && __spVideoDriver->getRendererType() == video::RENDERER_OPENGL ) ||
         Format == video::DATATYPE_UNSIGNED_SHORT || Format == video::DATATYPE_UNSIGNED_INT )
    {
        DefaultIndexFormat_ = Format;
    }
}
video::ERendererDataTypes SceneManager::getDefaultIndexFormat()
{
    return DefaultIndexFormat_;
}

void SceneManager::setTextureLoadingState(bool AllowTextureLoading)
{
    TextureLoadingState_ = AllowTextureLoading;
}
bool SceneManager::getTextureLoadingState()
{
    return TextureLoadingState_;
}

EMeshFileFormats SceneManager::getMeshFileFormat(
    const io::stringc &Filename, const EMeshFileFormats DefaultFormat)
{
    const io::stringc FileExtension = Filename.getExtensionPart().upper();
    
    if ( ( DefaultFormat == MESHFORMAT_UNKNOWN && FileExtension == "SPM" ) || DefaultFormat == MESHFORMAT_SPM )
        return MESHFORMAT_SPM;
    else if ( ( DefaultFormat == MESHFORMAT_UNKNOWN && FileExtension == "3DS" ) || DefaultFormat == MESHFORMAT_3DS )
        return MESHFORMAT_3DS;
    else if ( ( DefaultFormat == MESHFORMAT_UNKNOWN && FileExtension == "MS3D" ) || DefaultFormat == MESHFORMAT_MS3D )
        return MESHFORMAT_MS3D;
    else if ( ( DefaultFormat == MESHFORMAT_UNKNOWN && FileExtension == "X" ) || DefaultFormat == MESHFORMAT_X )
        return MESHFORMAT_X;
    else if ( ( DefaultFormat == MESHFORMAT_UNKNOWN && FileExtension == "B3D" ) || DefaultFormat == MESHFORMAT_B3D )
        return MESHFORMAT_B3D;
    else if ( ( DefaultFormat == MESHFORMAT_UNKNOWN && FileExtension == "MD2" ) || DefaultFormat == MESHFORMAT_MD2 )
        return MESHFORMAT_MD2;
    else if ( ( DefaultFormat == MESHFORMAT_UNKNOWN && FileExtension == "MD3" ) || DefaultFormat == MESHFORMAT_MD3 )
        return MESHFORMAT_MD3;
    else if ( ( DefaultFormat == MESHFORMAT_UNKNOWN && FileExtension == "OBJ" ) || DefaultFormat == MESHFORMAT_OBJ )
        return MESHFORMAT_OBJ;
    
    return MESHFORMAT_UNKNOWN;
}

ESceneFileFormats SceneManager::getSceneFileFormat(
    const io::stringc &Filename, const ESceneFileFormats DefaultFormat)
{
    const io::stringc FileExtension = Filename.getExtensionPart().upper();
    
    if ( ( DefaultFormat == SCENEFORMAT_UNKNOWN && FileExtension == "SPSB" ) || DefaultFormat == SCENEFORMAT_SPSB )
        return SCENEFORMAT_SPSB;
    else if (DefaultFormat == SCENEFORMAT_UNKNOWN && FileExtension == "BSP")
    {
        // Get the magic number of the BSP file, because there are more than one types of this file format
        io::FileSystem TempLoader;
        io::File* TempFile = TempLoader.openFile(Filename);
        s32 MagicNumber = TempFile->readValue<s32>();
        TempLoader.closeFile(TempFile);
        
        // Check which BSP file version the file has
        if (MagicNumber == 0x17 || MagicNumber == 0x1E)
            return SCENEFORMAT_BSP1;
        else
            return SCENEFORMAT_BSP3;
    }
    else if (DefaultFormat == SCENEFORMAT_BSP1 || DefaultFormat == SCENEFORMAT_BSP3)
        return DefaultFormat;
    
    return SCENEFORMAT_UNKNOWN;
}


} // /namespace scene

} // /namespace sp



// ================================================================================
