/*
 * Scene graph header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_SCENEGRAPH_H__
#define __SP_SCENEGRAPH_H__


#include "Base/spStandard.hpp"
#include "Base/spBasicMeshGenerator.hpp"
#include "Base/spInputOutputString.hpp"
#include "Base/spStencilManager.hpp"
#include "Base/spVertexFormat.hpp"
#include "Base/spTreeBuilder.hpp"
#include "SceneGraph/spSceneMesh.hpp"
#include "SceneGraph/spSceneCamera.hpp"
#include "SceneGraph/spSceneLight.hpp"
#include "SceneGraph/spSceneBillboard.hpp"
#include "SceneGraph/spSceneTerrain.hpp"
#include "SceneGraph/spCameraFirstPerson.hpp"
#include "SceneGraph/spCameraBlender.hpp"
#include "SceneGraph/spCameraTracking.hpp"
#include "SceneGraph/Collision/spCollisionDetector.hpp"
#include "SceneGraph/Animation/spNodeAnimation.hpp"
#include "SceneGraph/Animation/spMorphTargetAnimation.hpp"
#include "SceneGraph/Animation/spSkeletalAnimation.hpp"
#include "FileFormats/Mesh/spMeshFileFormats.hpp"
#include "RenderSystem/spShaderProgram.hpp"

#include <list>


namespace sp
{
namespace scene
{


/*
 * Enumerations
 */

//! Mesh file formats
enum EMeshFileFormats
{
    MESHFORMAT_UNKNOWN, //!< Unknown file format.
    
    /* Supported formats */
    MESHFORMAT_SPM,     //!< SoftPixelMesh.
    MESHFORMAT_3DS,     //!< 3D Studio.
    MESHFORMAT_MS3D,    //!< Milkshape3D.
    MESHFORMAT_X,       //!< DirectX.
    MESHFORMAT_B3D,     //!< Blitz3D.
    MESHFORMAT_MD2,     //!< Quake II model.
    MESHFORMAT_MD3,     //!< Quake III model.
    MESHFORMAT_OBJ,     //!< Wavefront Object.
    
    /* Unsupported formats (just for identification) */
    MESHFORMAT_OGRE,    //!< OGRE Mesh (unsupported yet).
    MESHFORMAT_LWO,     //!< LightWave Object (unsupported yet).
};

//! Scene file formats
enum ESceneFileFormats
{
    SCENEFORMAT_UNKNOWN,    //!< Unknown file format.
    
    /* Supported formats */
    SCENEFORMAT_SPSB,       //!< SoftPixel Sandbox Scene.
    SCENEFORMAT_BSP1,       //!< BinarySpacePartition v.1 (Quake map).
    SCENEFORMAT_BSP3,       //!< BinarySpacePartition v.3 (Quake III Arena map).
};

//! Supported scene managers
enum ESceneGraphs
{
    SCENEGRAPH_CUSTOM,          //!< Custom scene graph (user defined).
    SCENEGRAPH_SIMPLE,          //!< Simple default scene graph.
    SCENEGRAPH_SIMPLE_STREAM,   //!< Simple scene graph with streaming (used for multi-threading).
    SCENEGRAPH_FAMILY_TREE,     //!< Scene graph with child tree hierarchy.
    //SCENEGRAPH_KD_TREE,         //!< Scene graph with kd-Tree hierarchy. Practial for deferred renderer but not for render systems with transparency objects.
};


/**
This is the basic scene manager with all the basic functions like loading meshes, creating cameras and other objects.
To render a scene you will need one of the abstract scene manager classes like "SimpleSceneManager" or "ExpansiveSceneManager".
To know which type is the best for you take a look at their documentation sites. You also can create your own scene manager
by abstracting it from this class. Then you can handle scene rendering by yourself.
*/
class SP_EXPORT SceneGraph : public RenderNode
{
    
    public:
        
        virtual ~SceneGraph();
        
        //! Returns the type of the scene graph.
        inline ESceneGraphs getGraphType() const
        {
            return GraphType_;
        }
        inline bool hasChildTree() const
        {
            return hasChildTree_;
        }
        
        //! Returns pointer of the StencilManager object.
        StencilManager* getStencilManager();
        
        //! Adds the sepcified node to the scene node list.
        virtual void addSceneNode(SceneNode*    Object);
        virtual void addSceneNode(Camera*       Object);
        virtual void addSceneNode(Light*        Object);
        virtual void addSceneNode(RenderNode*   Object);
        
        //! Removes the sepcified node from the scene node list.
        virtual void removeSceneNode(SceneNode*     Object);
        virtual void removeSceneNode(Camera*        Object);
        virtual void removeSceneNode(Light*         Object);
        virtual void removeSceneNode(RenderNode*    Object);
        
        //! Adds a root SceneNode object. This is basically used for child tree scene managers.
        virtual void addRootNode(SceneNode* Object);
        
        //! Removes a root SceneNode object. This is basically used for child tree scene managers.
        virtual void removeRootNode(SceneNode* Object);
        
        /**
        Creates a simple Node.
        \return Pointer to a SceneNode object.
        */
        virtual SceneNode* createNode();
        
        /**
        Creates an empty Mesh object.
        With this you can build your own 3d models at the beginning.
        Look at the "Mesh" class reference to learn more about building models.
        \return Pointer to an Mesh object which represents the 3D models.
        */
        virtual Mesh* createMesh();
        
        /**
        Creates a standard primitive.
        \param Type: Type of the standard primitve (e.g. Cube, Sphere etc.).
        \param BuildContruct: Structure which holds the model contrstruction data (e.g. Segments, Radius etc.).
        */
        virtual Mesh* createMesh(const EBasicMeshes Model, const SMeshConstruct &BuildConstruct = SMeshConstruct());
        
        /**
        Creates a "SuperShape". This is a 'sphere-like' model which can look very strange with the right parameter values ;-).
        \param ValueList: List of the 12 values which describe the construction of the super-shape.
        Play with it to learn how the values affect the final model.
        \param Detail: Segments which defines the detail of the model.
        */
        virtual Mesh* createSuperShape(const f32 ValueList[12], s32 Detail = DEF_MESH_SEGMENTS);
        
        /**
        Creates a skybox with 6 surfaces.
        \param TextureList: Array of 6 textures.
        \param Radius: Radius (or rather size) of the box. Needs to be greater than the near-clipping plane of the camera.
        \param Type: Mapping type which defines the method on how the textures are mapped on the model.
        The two methods are similiar to these like in the "Half-Life 1" game.
        */
        virtual Mesh* createSkyBox(video::Texture* TextureList[6], f32 Radius = 50.0f);
        
        /**
        Creates a height field. It is an alternate to a dynamic terrain.
        \param HeightMapTexture: Texture which holds the height-map data
        \param Detail: Segments for the height-field to be created
        */
        virtual Mesh* createHeightField(const video::Texture* TexHeightMap, const s32 Segments = DEF_MESH_SEGMENTS);
        
        //! Creates a mesh out of the specified meshes.
        virtual Mesh* createMeshList(std::list<Mesh*> MergeList, bool isOldDelete);
        
        //! Creates a mesh out of the specified mesh's surface.
        virtual Mesh* createMeshSurface(Mesh* Model, u32 Surface);
        
        /**
        Loads a model from the disk. The supported file formats are listed in the "EModelTypes" enumeration
        and at the "SoftPixel Engine"'s homepage under "features".
        \param Filename: Filename/ path of the model to be loaded.
        \param TexturePath: Directory path in which the model's textures are stored.
        When a texture filename is saved in the model file (e.g. "Texture1.bmp") the "TexturePath" is pushed to the front
        and loaded. e.g. [TexturePath + "Texture1.bmp"]. By default the path where the model is saved is used.
        \param FileType: Specifies which file format the model has. By default the engine can determine it self.
        */
        virtual Mesh* loadMesh(
            io::stringc Filename, io::stringc TexturePath = video::TEXPATH_IGNORE, const EMeshFileFormats Format = MESHFORMAT_UNKNOWN
        );
        
        /**
        Loads a model or instanciates it when it has already been loaded.
        \see SceneGraph::loadMesh
        \note When the root mesh will be deleted you also need to delete all derived meshes, too!
        */
        virtual Mesh* getMesh(
            const io::stringc &Filename, io::stringc TexturePath = video::TEXPATH_IGNORE, const EMeshFileFormats Format = MESHFORMAT_UNKNOWN
        );
        
        /**
        Saves a model to the disk.
        \param Model: 3D model which is to be saved.
        \param Filename: Filename/ path where the file shall be created.
        \param FileType: Specifies which file format the file shall get.
        By default the engine can decide it by the filename's extension
        */
        virtual bool saveMesh(Mesh* Model, io::stringc Filename, const EMeshFileFormats Format = MESHFORMAT_UNKNOWN);
        
        /**
        Loads a scene (or rather game map).
        \param Filename: Filename/ path of the scene to be loaded.
        \param TexturePath: Directory path in which the scene's textures are stored.
        When loading a BSP1 (Quake 1/ Half-Life 1) map file you can set the WAD (texture storage file)
        file directly.
        \param FileType: Specifies which file format the scene has. By default the engine can determine it self.
        \param Flags: Specifies some possible options. This has been added particular for the SPSB (SoftPixel Sandbox Scene)
        file format which has been integrated since version 3.1.
        \return Pointer to the main Mesh object and 0 (Null) when loading an SPSB file (SoftPixel Sandbox Scene).
        \note When loading an SPSB file (SoftPixel Sandbox Scene) the return value is always 0 (Null).
        It is anyway the best idea to write an own class and derive from the SceneLoaderSPSB class to have more
        access of how the scene will be loaded.
        */
        virtual Mesh* loadScene(
            io::stringc Filename, io::stringc TexturePath = video::TEXPATH_IGNORE,
            const ESceneFileFormats Format = SCENEFORMAT_UNKNOWN, const s32 Flags = DEF_SCENE_FLAGS
        );
        
        /**
        Creates a fur mesh. This functions copies the model several times and creates a fur effect.
        Normally fur effects can be achieved better using shaders but for small meshes this function can be used.
        \param Model: 3D model which shall get a fur.
        \param FurTexture: Texture which is to be used as the fur surface.
        \param LayerCount: Count of mesh copies which represent the fur layers.
        \param HairLength: Length of hairs which specifies the distance from the first to the last layer.
        \param HairCloseness: Random value for ploting hairs in the fur texture. Higher values cater for less hairs.
        */
        virtual void createFurMesh(
            Mesh* Model, video::Texture* FurTexture,
            s32 LayerCount = 25, f32 HairLength = 0.2f, s32 HairCloseness = 2
        );
        
        /**
        Creates a Camera. Without cameras you cannot see the scene which shall be rendered.
        \return Pointer to the specified Camera or Camera derived class object.
        */
        template <class T> T* createCamera()
        {
            T* NewCamera = MemoryManager::createMemory<T>("scene::Camera");
            addSceneNode(NewCamera);
            setActiveCamera(NewCamera);
            return NewCamera;
        }
        
        /**
        Creates a Camera. Without cameras you cannot see the scene which shall be rendered.
        \return Pointer to a Camera object.
        */
        virtual Camera* createCamera();
        
        /**
        Creates a dynamic light source.
        \param Type: Light's type. Supported are three types: Directional (by default), Point, Spot.
        Point and Spot lights can be volumetric using "Light::setVolumetric".
        \return Pointer to a Light object.
        */
        virtual Light* createLight(const ELightModels Type = LIGHT_DIRECTIONAL);
        
        /**
        Creates a Billboard. Billboards (or Sprites) are mostly used for effects like Fire, Sparks, Lense flares etc.
        \param hTexture: Billboard's base texture. 0 is also allowed but then you only have a white quad.
        \return Pointer to a Billboard object.
        */
        virtual Billboard* createBillboard(video::Texture* BaseTexture = 0);
        
        /**
        Creates a Terrain. Terrains are mostly used for mountains and large areas. These terrains can be used only for
        height-fields. Voxels are not supported.
        \param TextureHeightMap: Heightmap data object. You can also use a normal Texture because the SHeightMapData
        structure accepts this parameter. But if you want to create a height field with more detail (e.g. 32 bit height data)
        you can create a height field using floating-point arrays. With this feature you can save and load your own
        height-field texture data file formats. i.e. when using a 16 bit height field data type you can save memory
        and anyhow you have much more precise for your terrain.
        \param Resolution: Specifies the grid resolution for the terrain. If the resolution is e.g. 8*8 each
        quad-tree-node will have a mesh with 8*8*2 triangles (or 8*8 quads).
        \param GeoMIPLevels: Count of levels for geo-MIP-mapping which specifies the forks count of the quad-tree.
        */
        virtual Terrain* createTerrain(
            const video::SHeightMapTexture &TextureHeightMap, const dim::size2di &Resolution, s32 GeoMIPLevels = DEF_GEOMIP_LEVELS
        );
        
        /**
        Copies the specified obejct
        \return Pointer to the new copied object
        */
        virtual SceneNode* copyNode(const SceneNode* Object);
        virtual Mesh* copyNode(const Mesh* Object);
        virtual Light* copyNode(const Light* Object);
        virtual Billboard* copyNode(const Billboard* Object);
        virtual Camera* copyNode(const Camera* Object);
        virtual Terrain* copyNode(const Terrain* Object);
        
        /**
        Deletes the specified object. All renderer resources are also released.
        \return True if the object could be deleted.
        Otherwise the object does not exist in the SceneManager's list or an other error has been occured.
        */
        virtual bool deleteNode(SceneNode* Object);
        
        /**
        Tries to find each node with the specified name.
        \param Name: Name of the wanted nodes.
        \return List with each Node object which could found.
        */
        virtual std::list<SceneNode*> findNodes(const io::stringc &Name) const;
        
        /**
        Tries to find the specified node.
        \param Name: Name of the wanted node.
        \return Pointer to the Node object which could found. If no object could found the return value is 0.
        */
        virtual SceneNode* findNode(const io::stringc &Name) const;
        
        /**
        Tries to find each child of the specified parent.
        \param ParentNode: Pointer to the Node object which is the parent of the wanted children.
        \return List with each Node object which could found.
        */
        virtual std::list<SceneNode*> findChildren(const SceneNode* Parent) const;
        
        /**
        Tries to find the specified child of the specified parent.
        \param ParentNode: Pointer to the Node object which is the parent of the wanted child.
        \param Name: Name of the wanted child.
        \return Pointer to the child Node object if the search was successful. Otherwise 0.
        */
        virtual SceneNode* findChild(const SceneNode* Parent, const io::stringc &Name) const;
        
        /**
        Renders the whole scene without using a camera object.
        This is particular used when a scene manager is a child of another one.
        */
        virtual void render() = 0;
        
        /**
        Renders the whole scene for the specified camera.
        \param ActiveCamera: Pointer to the camera object which gets the views location, projection etc.
        */
        virtual void renderScene(Camera* ActiveCamera);
        
        //! Renders the whole scene for each camera with their specified viewports
        virtual void renderScene();
        
        /**
        Renders the scene as a stero image. Use 3D glasses (red and green) to see the effect correctly.
        \param hCamera: Pointer to the camera object which gets the views location, projection etc.
        \param CamDegree: Degree or angle between to two points of view. With this parameter
        you can set the angle to concentrate the two eyes on a special point.
        \param CamDist: Distance between the two points of view or the two eyes.
        */
        virtual void renderSceneStereoImage(Camera* ActiveCamera, f32 CamDegree, f32 CamDist = 0.25f);
        
        //! Creates a new animation. Use the NodeAnimation, MorphTargetAnimation and SkeletalAnimation classes.
        template <class T> T* createAnimation(const io::stringc &Name = "")
        {
            T* NewAnim = MemoryManager::createMemory<T>(Name.size() ? Name : "Animation");
            NewAnim->setName(Name);
            AnimationList_.push_back(NewAnim);
            return NewAnim;
        }
        
        //! Deletes the specified animation.
        virtual void deleteAnimation(Animation* Anim);
        
        //! Deletes all animations.
        virtual void clearAnimations();
        
        //! Updates all animations.
        virtual void updateAnimations();
        
        //! Clears the whole scene from the specified objects.
        virtual void clearScene(
            bool isDeleteNodes = true, bool isDeleteMeshes = true,
            bool isDeleteCameras = true, bool isDeleteLights = true,
            bool isDeleteBillboards = true, bool isDeleteTerrains = true
        );
        
        /* ===== Extra functions ===== */
        
        /**
        Sets the wireframe mode of each "Mesh" and "Terrain" object (Billboards does not have wireframe mode yet).
        \param Type: Wireframe mode/ type (supported are: Solid, Lines, Points).
        */
        virtual void setWireframe(const video::EWireframeTypes Type);
        virtual void setWireframe(const video::EWireframeTypes TypeFront, const video::EWireframeTypes TypeBack);
        virtual void setRenderFace(const video::EFaceTypes Face);
        
        /**
        Removes the specified texture from each "Mesh" and "Terrain" object which use this texture.
        \param hTexture: Texture which is to be removed.
        */
        virtual void removeTexture(const video::Texture* Tex);
        
        /**
        Enables or disables the lighting system. By default lighting is disabled.
        If lighting is disbaled the whole scene is full bright and now shadows (or better no shading) is proceeded.
        \param isLighting: Specifies if lighting shall be enabled or disabled (true/false)
        */
        virtual void setLighting(bool isLighting = true);
        virtual bool getLighting() const;
        
        //! Enables or disables the stencil effects. If true stencil shadow volumes are processed and visible.
        inline void setStencilEffects(bool isStencilEffects = true)
        {
            isStencilEffects_ = isStencilEffects;
        }
        inline bool getStencilEffects() const
        {
            return isStencilEffects_;
        }
        
        /* Object lists */
        virtual std::list<Mesh*> getMeshList() const;
        virtual std::list<Billboard*> getBillboardList() const;
        virtual std::list<Terrain*> getTerrainList() const;
        
        virtual inline std::list<Light*> getLightList() const
        {
            return LightList_;
        }
        virtual inline std::list<Camera*> getCameraList() const
        {
            return CameraList_;
        }
        virtual inline std::list<SceneNode*> getNodeList() const
        {
            return NodeList_;
        }
        virtual inline std::list<Animation*> getAnimationList() const
        {
            return AnimationList_;
        }
        
        /**
        Sets the current active camera.
        \param ActiveCamera: Camera which is to be set to the current active one.
        This is used in all 3D drawing functions such as "draw3DLine", "draw3DTriangle" etc.
        in the VideoDriver.
        */
        inline void setActiveCamera(Camera* ActiveCamera)
        {
            ActiveCamera_ = ActiveCamera;
        }
        inline Camera* getActiveCamera() const
        {
            return ActiveCamera_;
        }
        
        /**
        Sets the current active mesh.
        \param ActiveMesh: Mesh which is to be set to the current active one.
        This is set before a mesh is rendered. Particular used for the default
        basic shader for the Direct3D11 video driver.
        */
        inline void setActiveMesh(Mesh* ActiveMesh)
        {
            ActiveMesh_ = ActiveMesh;
        }
        inline Mesh* getActiveMesh() const
        {
            return ActiveMesh_;
        }
        
        /**
        Sets the default vertex format which will be used when loading or creating a new mesh.
        \param Format: Specifies the new default vertex format. By default RenderSystem::getVertexFormatDefault().
        If 0 the initial default vertex format will be used again.
        */
        static void setDefaultVertexFormat(const video::VertexFormat* Format);
        
        //! Returns the default vertex format.
        static const video::VertexFormat* getDefaultVertexFormat();
        
        /**
        Sets the default index format which will be used when loading or creating a new mesh.
        \param Format: Specifies the new default index format. By default ATTRIBUTE_UNSIGNED_SHORT.
        Must be one of the following values: ATTRIBUTE_UNSIGNED_BYTE (only for OpenGL), ATTRIBUTE_UNSIGNED_SHORT
        or ATTRIBUTE_UNSIGNED_INT.
        */
        static void setDefaultIndexFormat(const video::ERendererDataTypes Format);
        
        //! Returns the default index format.
        static video::ERendererDataTypes getDefaultIndexFormat();
        
        //! Allows or disallows mesh loaders to load textures.
        static void setTextureLoadingState(bool AllowTextureLoading);
        static bool getTextureLoadingState();
        
        //! Enables or disables reverse depth sorting used for early-z-culling.
        static void setReverseDepthSorting(bool Enable);
        static bool getReverseDepthSorting();
        
        /* Count lists */
        //! Returns count of vertices in the whole scene. Only Mesh and Terrain objects are included.
        virtual u32 getSceneVertexCount() const;
        
        //! Returns count of triangles in the whole scene. Only Mesh and Terrain objects are included.
        virtual u32 getSceneTriangleCount() const;
        
        //! Returns count of objects in the whole scene. Each kind of Node objects are included.
        virtual u32 getSceneObjectsCount() const;
        
    protected:
        
        friend class SceneNode;
        friend class RenderNode;
        friend class MaterialNode;
        friend class Mesh;
        
        /* === Functions === */
        
        SceneGraph(const ESceneGraphs Type);
        
        void sortRenderList(std::list<RenderNode*> &ObjectList, const dim::matrix4f &BaseMatrix);
        void sortLightList(std::list<Light*> &ObjectList);
        
        EMeshFileFormats getMeshFileFormat(const io::stringc &Filename, const EMeshFileFormats DefaultFormat) const;
        ESceneFileFormats getSceneFileFormat(const io::stringc &Filename, const ESceneFileFormats DefaultFormat) const;
        
        void finishRenderScene();
        
        /* === Inline functions === */
        
        inline Mesh* integrateNewMesh(Mesh* NewMesh)
        {
            if (NewMesh)
            {
                NewMesh->getMaterial()->setWireframe(WireframeFront_, WireframeBack_);
                addSceneNode(dynamic_cast<RenderNode*>(NewMesh));
            }
            return NewMesh;
        }
        
        /* === Templates === */
        
        template <class T> void clearRenderObjectList(const ENodeTypes Type, std::list<RenderNode*> &ObjectList)
        {
            for (std::list<RenderNode*>::iterator it = ObjectList.begin(); it != ObjectList.end();)
            {
                if ((*it)->getType() == Type)
                {
                    delete *it;
                    it = ObjectList.erase(it);
                }
                else
                    ++it;
            }
        }
        
        template <class T> bool removeObjectFromList(SceneNode* Object, std::list<T*> &SearchList, bool isDelete = false)
        {
            if (!Object)
                return false;
            
            for (typename std::list< T*, std::allocator<T*> >::iterator it = SearchList.begin(); it != SearchList.end(); ++it)
            {
                if (*it == Object)
                {
                    if (isDelete)
                        delete Object;
                    SearchList.erase(it);
                    return true;
                }
            }
            
            return false;
        }
        
        template <class T> void addChildToList(
            const Node* ParentNode, std::list<SceneNode*> &NodeList, const std::list<T*> &SearchList) const
        {
            for (typename std::list< T*, std::allocator<T*> >::const_iterator it = SearchList.begin(); it != SearchList.end(); ++it)
            {
                if ((*it)->getParent() == ParentNode)
                    NodeList.push_back(*it);
            }
        }
        
        template <class T> SceneNode* findChildInList(
            const SceneNode* ParentNode, const std::list<T*> &SearchList, const io::stringc &Name) const
        {
            for (typename std::list< T*, std::allocator<T*> >::const_iterator it = SearchList.begin(); it != SearchList.end(); ++it)
            {
                if ((*it)->getParent() == ParentNode && (*it)->getName() == Name)
                    return *it;
            }
            
            return 0;
        }
        
        template <class T, class L> void addNodeToList(
            const io::stringc &Name, std::list<SceneNode*> &NodeList, const std::list<L*> &SearchList) const
        {
            for (typename std::list< L*, std::allocator<L*> >::const_iterator it = SearchList.begin(); it != SearchList.end(); ++it)
            {
                if ((*it)->getName() == Name)
                    NodeList.push_back(*it);
            }
        }
        
        template <class T> SceneNode* findNodeInList(
            const io::stringc &Name, const std::list<T*> &SearchList) const
        {
            for (typename std::list< T*, std::allocator<T*> >::const_iterator it = SearchList.begin(); it != SearchList.end(); ++it)
            {
                if ((*it)->getName() == Name)
                    return *it;
            }
            
            return 0;
        }
        
        template <class T> std::list<T*> filterRenderNodeList(const ENodeTypes Type) const
        {
            std::list<T*> NodeList;
            
            for (std::list<RenderNode*>::const_iterator it = RenderList_.begin(); it != RenderList_.end(); ++it)
            {
                if ((*it)->getType() == Type)
                    NodeList.push_back(static_cast<T*>(*it));
            }
            
            return NodeList;
        }
        
        /* === Members === */
        
        ESceneGraphs GraphType_;
        bool hasChildTree_;
        
        /* All object lists (3d scene objects, lights etc.) */
        std::list<SceneNode*>   NodeList_;
        std::list<Camera*>      CameraList_;
        std::list<Light*>       LightList_;
        std::list<RenderNode*>  RenderList_;
        
        std::list<Animation*>   AnimationList_;
        
        std::map<std::string, Mesh*> MeshMap_;
        
        Camera* ActiveCamera_;
        Mesh* ActiveMesh_;
        
        /* RenderState members */
        bool isStencilEffects_;
        
        video::EWireframeTypes WireframeFront_, WireframeBack_;
        
        /* Statie members */
        static const video::VertexFormat* DefaultVertexFormat_;
        static video::ERendererDataTypes DefaultIndexFormat_;
        
        static bool TextureLoadingState_;
        static bool ReverseDepthSorting_;
        
};

extern StencilManager* __spStencilManager;


} // /namespace scene

} // /namespace sp


#endif



// ================================================================================
