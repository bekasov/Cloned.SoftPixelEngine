/*
 * Scene graph file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "SceneGraph/spSceneGraph.hpp"
#include "SceneGraph/spSceneManager.hpp"
#include "Platform/spSoftPixelDeviceOS.hpp"
#include "Base/spInternalDeclarations.hpp"
#include "Base/spSharedObjects.hpp"

#include <boost/foreach.hpp>


namespace sp
{

extern SoftPixelDevice* __spDevice;
extern io::InputControl* __spInputControl;
extern video::RenderSystem* __spVideoDriver;
extern scene::SceneGraph* __spSceneManager;

namespace scene
{


/*
 * Internal functions
 */

bool cmpObjectLights(Light* &obj1, Light* &obj2)
{
    /* Compare visiblity */
    //if (obj1->getVisible() != obj2->getVisible())
    //    return static_cast<s32>(obj1->getVisible()) > static_cast<s32>(obj2->getVisible());
    
    /* Compare light model */
    /*if (obj1->getLightModel() == LIGHT_DIRECTIONAL && obj2->getLightModel() != LIGHT_DIRECTIONAL)
        return true;
    if (obj1->getLightModel() != LIGHT_DIRECTIONAL && obj2->getLightModel() == LIGHT_DIRECTIONAL)
        return false;*/
    
    /* Compare distance to camera */
    const dim::vector3df CamPos(
        __spSceneManager->getActiveCamera() ? __spSceneManager->getActiveCamera()->getPosition(true) : 0.0f
    );
    
    return
        math::getDistanceSq(obj1->getPosition(true), CamPos) <
        math::getDistanceSq(obj2->getPosition(true), CamPos);
}

bool compareSceneNodes(SceneNode* &ObjA, SceneNode* &ObjB)
{
    /* Compare visiblity */
    if (ObjA->getVisible() != ObjB->getVisible())
        return ObjA->getVisible();
    
    /* Compare material nodes */
    if (ObjA->getType() >= NODE_MESH && ObjB->getType() >= NODE_MESH)
        return static_cast<MaterialNode*>(ObjA)->compare(static_cast<MaterialNode*>(ObjB));
    
    /* Compare types */
    return ObjA->getType() > ObjB->getType();
}

static bool compareRenderNodesDepthDistance(RenderNode* &ObjA, RenderNode* &ObjB)
{
    /* Compare visiblity */
    if (ObjA->getVisible() != ObjB->getVisible())
        return ObjA->getVisible();
    
    /* Compare material nodes */
    if (ObjA->getType() >= NODE_MESH && ObjB->getType() >= NODE_MESH)
        return static_cast<MaterialNode*>(ObjA)->compare(static_cast<MaterialNode*>(ObjB));
    
    /* Compare types */
    return ObjA->getType() > ObjB->getType();
}

static bool compareRenderNodesMeshBuffer(RenderNode* &ObjA, RenderNode* &ObjB)
{
    /* Compare visiblity */
    if (ObjA->getVisible() != ObjB->getVisible())
        return ObjA->getVisible();
    
    /* Compare mesh nodes */
    if (ObjA->getType() == NODE_MESH && ObjB->getType() == NODE_MESH)
        return static_cast<Mesh*>(ObjA)->compareMeshBuffers(static_cast<Mesh*>(ObjB));
    
    /* Compare types */
    return ObjA->getType() > ObjB->getType();
}


/*
 * SceneGraph class
 */

bool SceneGraph::ReverseDepthSorting_ = false;

SceneGraph::SceneGraph(const ESceneGraphs Type) :
    RenderNode      (NODE_SCENEGRAPH        ),
    GraphType_      (Type                   ),
    hasChildTree_   (false                  ),
    ActiveCamera_   (0                      ),
    ActiveMesh_     (0                      ),
    WireframeFront_ (video::WIREFRAME_SOLID ),
    WireframeBack_  (video::WIREFRAME_SOLID ),
    DepthSorting_   (true                   )
{
}
SceneGraph::~SceneGraph()
{
}

void SceneGraph::addSceneNode(SceneNode* Object)
{
    if (Object)
        NodeList_.push_back(Object);
}
void SceneGraph::removeSceneNode(SceneNode* Object)
{
    MemoryManager::removeElement(NodeList_, Object);
}

void SceneGraph::addSceneNode(Camera* Object)
{
    if (Object)
        CameraList_.push_back(Object);
}
void SceneGraph::removeSceneNode(Camera* Object)
{
    MemoryManager::removeElement(CameraList_, Object);
}

void SceneGraph::addSceneNode(Light* Object)
{
    if (Object)
        LightList_.push_back(Object);
}
void SceneGraph::removeSceneNode(Light* Object)
{
    MemoryManager::removeElement(LightList_, Object);
}

void SceneGraph::addSceneNode(RenderNode* Object)
{
    if (Object)
        RenderList_.push_back(Object);
}
void SceneGraph::removeSceneNode(RenderNode* Object)
{
    MemoryManager::removeElement(RenderList_, Object);
}

void SceneGraph::addRootNode(SceneNode* Object)
{
    // do nothing
}
void SceneGraph::removeRootNode(SceneNode* Object)
{
    // do nothing
}

SceneNode* SceneGraph::createNode()
{
    SceneNode* NewSceneNode = gSharedObjects.SceneMngr->createNode();
    addSceneNode(NewSceneNode);
    return NewSceneNode;
}

Mesh* SceneGraph::createMesh()
{
    return integrateNewMesh(gSharedObjects.SceneMngr->createMesh());
}
Mesh* SceneGraph::createMesh(const EBasicMeshes Model, const SMeshConstruct &BuildConstruct)
{
    return integrateNewMesh(gSharedObjects.SceneMngr->createMesh(Model, BuildConstruct));
}
Mesh* SceneGraph::createSuperShape(const f32 (&ValueList)[12], s32 Detail)
{
    return integrateNewMesh(gSharedObjects.SceneMngr->createSuperShape(ValueList, Detail));
}
Mesh* SceneGraph::createSkyBox(video::Texture* (&TextureList)[6], f32 Radius)
{
    return integrateNewMesh(gSharedObjects.SceneMngr->createSkyBox(TextureList, Radius));
}
Mesh* SceneGraph::createHeightField(const video::Texture* TexHeightMap, const s32 Segments)
{
    return integrateNewMesh(gSharedObjects.SceneMngr->createHeightField(TexHeightMap, Segments));
}
Mesh* SceneGraph::createMeshList(std::list<Mesh*> MergeList, bool isOldDelete)
{
    return integrateNewMesh(gSharedObjects.SceneMngr->createMeshList(MergeList, isOldDelete));
}
Mesh* SceneGraph::createMeshSurface(Mesh* Model, u32 Surface)
{
    return integrateNewMesh(gSharedObjects.SceneMngr->createMeshSurface(Model, Surface));
}

Mesh* SceneGraph::loadMesh(const io::stringc &Filename, const io::stringc &TexturePath, const EMeshFileFormats Format)
{
    return integrateNewMesh(gSharedObjects.SceneMngr->loadMesh(Filename, TexturePath, Format));
}
bool SceneGraph::saveMesh(Mesh* Model, const io::stringc &Filename, const EMeshFileFormats Format)
{
    return gSharedObjects.SceneMngr->saveMesh(Model, Filename, Format);
}
Mesh* SceneGraph::loadScene(
    const io::stringc &Filename, const io::stringc &TexturePath, const ESceneFileFormats Format, const s32 Flags)
{
    return integrateNewMesh(gSharedObjects.SceneMngr->loadScene(Filename, TexturePath, Format, Flags));
}
Mesh* SceneGraph::getMesh(const io::stringc &Filename, const io::stringc &TexturePath, const EMeshFileFormats Format)
{
    return integrateNewMesh(gSharedObjects.SceneMngr->getMesh(Filename, TexturePath, Format));
}

Light* SceneGraph::createLight(const ELightModels Type)
{
    Light* NewLight = gSharedObjects.SceneMngr->createLight(Type);
    addSceneNode(NewLight);
    return NewLight;
}

Billboard* SceneGraph::createBillboard(video::Texture* BaseTexture)
{
    Billboard* NewBillboard = gSharedObjects.SceneMngr->createBillboard(BaseTexture);
    addSceneNode(NewBillboard);
    return NewBillboard;
}

Camera* SceneGraph::createCamera()
{
    Camera* NewCamera = gSharedObjects.SceneMngr->createCamera();
    addSceneNode(NewCamera);
    return NewCamera;
}

Terrain* SceneGraph::createTerrain(
    const video::SHeightMapTexture &TextureHeightMap, const dim::size2di &Resolution, s32 GeoMIPLevels)
{
    Terrain* NewTerrain = gSharedObjects.SceneMngr->createTerrain(TextureHeightMap, Resolution, GeoMIPLevels);
    addSceneNode(NewTerrain);
    return NewTerrain;
}

void SceneGraph::renderScene()
{
    foreach (Camera* Cam, CameraList_)
    {
        if (Cam->getVisible())
            renderScene(Cam);
    }
}

void SceneGraph::renderScene(Camera* ActiveCamera)
{
    /* Configure view to the current active camera */
    setActiveCamera(ActiveCamera);
    
    if (ActiveCamera)
        ActiveCamera->setupRenderView();
    
    spWorldMatrix.reset();
    
    /* Render the scene graph */
    render();
    
    /* Finish rendering the scene */
    SceneGraph::finishRenderScene();
}

void SceneGraph::renderScenePlain(Camera* ActiveCamera)
{
    if (ActiveCamera)
        renderScene(ActiveCamera);
    else
        renderScene();
}


/*
Renders the whole scene as a stereo image
You can use '3d glaces' because this scene will renderd two times in a red and a green color mask
*/
void SceneGraph::renderSceneStereoImage(Camera* ActiveCamera, f32 CamDegree, f32 CamDist)
{
    if (!ActiveCamera)
        return;
    
    /* Temporary variables and static members */
    
    static video::Texture* StereoImageA, * StereoImageB;
    
    const dim::size2di ScrSize(gSharedObjects.ScreenWidth, gSharedObjects.ScreenHeight);
    
    video::Texture* CurRenderTarget = __spVideoDriver->getRenderTarget();
    
    if (!StereoImageA)
    {
        StereoImageA = __spVideoDriver->createTexture(ScrSize);
        StereoImageA->setMipMapping(false);
        StereoImageA->setFilter(video::FILTER_LINEAR);
        StereoImageA->setRenderTarget(true);
        
        StereoImageB = __spVideoDriver->createTexture(ScrSize);
        StereoImageB->setMipMapping(false);
        StereoImageB->setFilter(video::FILTER_LINEAR);
        StereoImageB->setRenderTarget(true);
    }
    
    /* Red color scene */
    
    __spVideoDriver->setRenderTarget(StereoImageA);
    
    __spVideoDriver->setColorMask(true, true, true, true);
    __spVideoDriver->clearBuffers();
    __spVideoDriver->setColorMask(false, true, false, true);
    {
        ActiveCamera->move(dim::vector3df(CamDist, 0, 0));
        ActiveCamera->turn(dim::vector3df(0, CamDegree, 0));
    }
    renderScene(ActiveCamera);
    
    /* Green color scene */
    
    __spVideoDriver->setRenderTarget(StereoImageB);
    
    __spVideoDriver->setColorMask(true, true, true, true);
    __spVideoDriver->clearBuffers();
    __spVideoDriver->setColorMask(true, false, false, true);
    {
        ActiveCamera->turn(dim::vector3df(0, -CamDegree, 0));
        ActiveCamera->move(dim::vector3df(-CamDist*2, 0, 0));
        ActiveCamera->turn(dim::vector3df(0, -CamDegree, 0));
    }
    renderScene(ActiveCamera);
    
    __spVideoDriver->setRenderTarget(CurRenderTarget);
    
    __spVideoDriver->setColorMask(true, true, true, true);
    __spVideoDriver->clearBuffers();
    {
        ActiveCamera->turn(dim::vector3df(0, CamDegree, 0));
        ActiveCamera->move(dim::vector3df(CamDist, 0, 0));
    }
    
    /* Drawing */
    
    __spVideoDriver->beginDrawing2D();
    {
        const dim::rect2df Clipping(0, 0, 1, 1);
        
        __spVideoDriver->draw2DImage(
            StereoImageA, dim::rect2di(0, 0, ScrSize.Width, ScrSize.Height), Clipping
        );
        __spVideoDriver->draw2DImage(
            StereoImageB, dim::rect2di(0, 0, ScrSize.Width, ScrSize.Height), Clipping, video::color(255, 255, 255, 128)
        );
    }
    __spVideoDriver->endDrawing2D();
}

void SceneGraph::clearScene(
    bool isRemoveNodes, bool isRemoveMeshes, bool isRemoveCameras,
    bool isRemoveLights, bool isRemoveBillboards, bool isRemoveTerrains)
{
    if (isRemoveNodes)
        NodeList_.clear();
    if (isRemoveCameras)
        CameraList_.clear();
    if (isRemoveLights)
        LightList_.clear();
    
    if (isRemoveMeshes && isRemoveBillboards && isRemoveTerrains)
        RenderList_.clear();
    else
    {
        //todo
        RenderList_.clear();
        #ifdef SP_DEBUGMODE
        io::Log::debug("SceneGraph::clearScene", "TODO (isRemoveMeshes || isRemoveBillboards || isRemoveTerrains)");
        #endif
    }
}

SceneNode* SceneGraph::copyNode(const SceneNode* Object)
{
    SceneNode* NewObject = gSharedObjects.SceneMngr->copyNode(Object);
    addSceneNode(NewObject);
    return NewObject;
}
Mesh* SceneGraph::copyNode(const Mesh* Object)
{
    Mesh* NewObject = gSharedObjects.SceneMngr->copyNode(Object);
    addSceneNode(NewObject);
    return NewObject;
}
Light* SceneGraph::copyNode(const Light* Object)
{
    Light* NewObject = gSharedObjects.SceneMngr->copyNode(Object);
    addSceneNode(NewObject);
    return NewObject;
}
Billboard* SceneGraph::copyNode(const Billboard* Object)
{
    Billboard* NewObject = gSharedObjects.SceneMngr->copyNode(Object);
    addSceneNode(NewObject);
    return NewObject;
}
Camera* SceneGraph::copyNode(const Camera* Object)
{
    Camera* NewObject = gSharedObjects.SceneMngr->copyNode(Object);
    addSceneNode(NewObject);
    return NewObject;
}
Terrain* SceneGraph::copyNode(const Terrain* Object)
{
    /*Terrain* NewObject = gSharedObjects.SceneMngr->copyNode(Object);
    addSceneNode(NewObject);
    return NewObject;*/
    return 0;
}

bool SceneGraph::deleteNode(SceneNode* Object)
{
    if (Object)
    {
        switch (Object->getType())
        {
            case NODE_CAMERA:
                removeSceneNode(static_cast<Camera*>(Object));
                break;
            case NODE_LIGHT:
                removeSceneNode(static_cast<Light*>(Object));
                break;
            case NODE_MESH:
            case NODE_BILLBOARD:
            case NODE_TERRAIN:
            case NODE_SCENEGRAPH:
                removeSceneNode(static_cast<RenderNode*>(Object));
                break;
            default:
                removeSceneNode(Object);
                break;
        }
        
        gSharedObjects.SceneMngr->deleteNode(Object);
    }
    return false;
}

std::list<SceneNode*> SceneGraph::findNodes(const io::stringc &Name) const
{
    std::list<SceneNode*> NodeList;
    
    addNodeToList<SceneNode ,SceneNode> (Name, NodeList, NodeList_  );
    addNodeToList<Camera, Camera>       (Name, NodeList, CameraList_);
    addNodeToList<Light, Light>         (Name, NodeList, LightList_ );
    addNodeToList<Mesh, RenderNode>     (Name, NodeList, RenderList_);
    addNodeToList<Billboard, RenderNode>(Name, NodeList, RenderList_);
    addNodeToList<Terrain, RenderNode>  (Name, NodeList, RenderList_);
    
    return NodeList;
}

SceneNode* SceneGraph::findNode(const io::stringc &Name) const
{
    SceneNode* Object = 0;
    
    if ( ( Object = findNodeInList<SceneNode>   (Name, NodeList_    ) ) != 0 ) return Object;
    if ( ( Object = findNodeInList<Camera>      (Name, CameraList_  ) ) != 0 ) return Object;
    if ( ( Object = findNodeInList<Light>       (Name, LightList_   ) ) != 0 ) return Object;
    if ( ( Object = findNodeInList<RenderNode>  (Name, RenderList_  ) ) != 0 ) return Object;
    if ( ( Object = findNodeInList<RenderNode>  (Name, RenderList_  ) ) != 0 ) return Object;
    if ( ( Object = findNodeInList<RenderNode>  (Name, RenderList_  ) ) != 0 ) return Object;
    
    return 0;
}

std::vector<SceneNode*> SceneGraph::findChildren(const SceneNode* ParentNode) const
{
    std::vector<SceneNode*> NodeList;
    
    addChildToList<SceneNode>   (ParentNode, NodeList, NodeList_    );
    addChildToList<Camera>      (ParentNode, NodeList, CameraList_  );
    addChildToList<Light>       (ParentNode, NodeList, LightList_   );
    addChildToList<RenderNode>  (ParentNode, NodeList, RenderList_  );
    
    return NodeList;
}

SceneNode* SceneGraph::findChild(const SceneNode* ParentNode, const io::stringc &Name) const
{
    SceneNode* Child = 0;
    
    if ( ( Child = findChildInList<SceneNode>   (ParentNode, NodeList_,     Name) ) != 0 ) return Child;
    if ( ( Child = findChildInList<Camera>      (ParentNode, CameraList_,   Name) ) != 0 ) return Child;
    if ( ( Child = findChildInList<Light>       (ParentNode, LightList_,    Name) ) != 0 ) return Child;
    if ( ( Child = findChildInList<RenderNode>  (ParentNode, RenderList_,   Name) ) != 0 ) return Child;
    if ( ( Child = findChildInList<RenderNode>  (ParentNode, RenderList_,   Name) ) != 0 ) return Child;
    if ( ( Child = findChildInList<RenderNode>  (ParentNode, RenderList_,   Name) ) != 0 ) return Child;
    
    return 0;
}

std::list<Mesh*> SceneGraph::getMeshList() const
{
    return filterRenderNodeList<Mesh>(scene::NODE_MESH);
}
std::list<Billboard*> SceneGraph::getBillboardList() const
{
    return filterRenderNodeList<Billboard>(scene::NODE_BILLBOARD);
}
std::list<Terrain*> SceneGraph::getTerrainList() const
{
    return filterRenderNodeList<Terrain>(scene::NODE_TERRAIN);
}

void SceneGraph::setWireframe(const video::EWireframeTypes Type)
{
    WireframeFront_ = WireframeBack_ = Type;
    
    foreach (RenderNode* Obj, RenderList_)
    {
        if (Obj->getType() == NODE_MESH || Obj->getType() == NODE_BILLBOARD || Obj->getType() == NODE_TERRAIN)
            static_cast<MaterialNode*>(Obj)->getMaterial()->setWireframe(Type);
    }
}

void SceneGraph::setWireframe(const video::EWireframeTypes TypeFront, const video::EWireframeTypes TypeBack)
{
    WireframeFront_ = TypeFront;
    WireframeBack_  = TypeBack;
    
    foreach (Mesh* Obj, getMeshList())
        Obj->getMaterial()->setWireframe(WireframeFront_, WireframeBack_);
}

void SceneGraph::setRenderFace(const video::EFaceTypes Face)
{
    foreach (Mesh* Obj, getMeshList())
        Obj->getMaterial()->setRenderFace(Face);
}

void SceneGraph::setLighting(bool isLighting)
{
    //!TODO! -> make this a member variable and not a global state
    __isLighting = isLighting;
}
bool SceneGraph::getLighting() const
{
    return __isLighting;
}

/* === Static functions: === */

void SceneGraph::setReverseDepthSorting(bool Enable)
{
    ReverseDepthSorting_ = Enable;
}
bool SceneGraph::getReverseDepthSorting()
{
    return ReverseDepthSorting_;
}

u32 SceneGraph::getSceneMeshBufferCount() const
{
    u32 SurfaceCount = 0;
    
    foreach (Mesh* Obj, getMeshList())
        SurfaceCount += Obj->getMeshBufferCount();
    
    return SurfaceCount;
}

u32 SceneGraph::getSceneVertexCount() const
{
    u32 VertexCount = 0;
    
    foreach (Mesh* Obj, getMeshList())
        VertexCount += Obj->getVertexCount();
    
    return VertexCount;
}

u32 SceneGraph::getSceneTriangleCount() const
{
    u32 TriangleCount = 0;
    
    foreach (Mesh* Obj, getMeshList())
        TriangleCount += Obj->getTriangleCount();
    
    return TriangleCount;
}

u32 SceneGraph::getSceneObjectsCount() const
{
    return NodeList_.size() + CameraList_.size() + LightList_.size() + RenderList_.size();
}

void SceneGraph::sortRenderList(const ERenderListSortMethods Method, std::vector<RenderNode*> &ObjectList)
{
    switch (Method)
    {
        case RENDERLIST_SORT_DEPTHDISTANCE:
            std::sort(ObjectList.begin(), ObjectList.end(), compareRenderNodesDepthDistance);
            break;
        case RENDERLIST_SORT_MESHBUFFER:
            std::sort(ObjectList.begin(), ObjectList.end(), compareRenderNodesMeshBuffer);
            break;
        default:
            break;
    }
}

void SceneGraph::sortRenderList(const ERenderListSortMethods Method)
{
    sortRenderList(Method, RenderList_);
}


/*
 * ======= Protected: =======
 */

Mesh* SceneGraph::integrateNewMesh(Mesh* NewMesh)
{
    if (NewMesh)
    {
        NewMesh->getMaterial()->setWireframe(WireframeFront_, WireframeBack_);
        addSceneNode(dynamic_cast<RenderNode*>(NewMesh));
    }
    return NewMesh;
}

void SceneGraph::arrangeRenderList(std::vector<RenderNode*> &ObjectList, const dim::matrix4f &BaseMatrix)
{
    if (ActiveCamera_)
        ActiveCamera_->updateTransformation();
    
    foreach (RenderNode* Obj, ObjectList)
    {
        if (Obj->getVisible())
            Obj->updateTransformationBase(BaseMatrix);
    }
    
    if (DepthSorting_)
        sortRenderList(RENDERLIST_SORT_DEPTHDISTANCE, ObjectList);
}

void SceneGraph::arrangeLightList(std::vector<Light*> &ObjectList)
{
    const u32 MaxLightCount = static_cast<u32>(__spVideoDriver->getMaxLightCount());
    
    if (ObjectList.size() <= MaxLightCount)
        return;
    
    /* Sort light list */
    std::sort(ObjectList.begin(), ObjectList.end(), cmpObjectLights);
    
    /* Update renderer lights for the first [MaxLightCount] objects */
    u32 LightID = 0;
    video::color Diffuse, Ambient, Specular;
    
    foreach (Light* Obj, ObjectList)
    {
        Obj->LightID_ = LightID;
        
        /* Update light colors */
        Obj->getLightingColor(Diffuse, Ambient, Specular);
        __spVideoDriver->setLightColor(LightID, Diffuse, Ambient, Specular);
        
        /* Update light status */
        __spVideoDriver->setLightStatus(LightID, Obj->getVisible());
        
        if (++LightID >= MaxLightCount)
            break;
    }
}

void SceneGraph::renderLightsDefault(const dim::matrix4f &BaseMatrix)
{
    arrangeLightList(LightList_);
    
    s32 LightIndex = 0;
    
    foreach (Light* Node, LightList_)
    {
        if (!Node->getVisible())
            continue;
        if (++LightIndex > MAX_COUNT_OF_LIGHTS)
            break;
        
        spWorldMatrix = BaseMatrix;
        Node->render();
    }
}

void SceneGraph::finishRenderScene()
{
    __spVideoDriver->endSceneRendering();
}


} // /namespace scene

} // /namespace sp



// ================================================================================
