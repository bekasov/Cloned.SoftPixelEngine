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
#include "SceneGraph/Animation/spNodeAnimation.hpp"
#include "SceneGraph/Animation/spMorphTargetAnimation.hpp"
#include "SceneGraph/Animation/spSkeletalAnimation.hpp"
#include "FileFormats/Mesh/spMeshFileFormats.hpp"
#include "RenderSystem/spShaderProgram.hpp"

#include <list>
#include <vector>


namespace sp
{
namespace scene
{


//! Supported scene graphs.
enum ESceneGraphs
{
    SCENEGRAPH_CUSTOM,          //!< Custom scene graph (user defined).
    SCENEGRAPH_SIMPLE,          //!< Simple default scene graph.
    SCENEGRAPH_SIMPLE_STREAM,   //!< Simple scene graph with streaming (used for multi-threading).
    SCENEGRAPH_FAMILY_TREE,     //!< Scene graph with child tree hierarchy.
    SCENEGRAPH_PORTAL_BASED,    //!< Portal-based scene graph.
};

//! Sort methods for the render node list.
enum ERenderListSortMethods
{
    /**
    Sorting with dependency to the depth distance between the renderable node and the view camera.
    This will be used when depth-sorting is enabled for a scene graph.
    */
    RENDERLIST_SORT_DEPTHDISTANCE,
    /**
    Sorting with dependency to the mesh buffers. This should be used if
    depth-sorting is disabled for performance optimization.
    */
    RENDERLIST_SORT_MESHBUFFER,
};


/**
This is the basic scene manager with all the basic functions like loading meshes, creating cameras and other objects.
To render a scene you will need one of the abstract scene manager classes like "SimpleSceneManager" or "ExpansiveSceneManager".
To know which type is the best for you take a look at their documentation sites. You also can create your own scene manager
by abstracting it from this class. Then you can handle scene rendering by yourself.
\ingroup group_scenegraph
\since Version 3.0
*/
class SP_EXPORT SceneGraph : public RenderNode
{
    
    public:
        
        virtual ~SceneGraph();
        
        /* === Functions === */
        
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
        virtual Mesh* createSuperShape(const f32 (&ValueList)[12], s32 Detail = DEF_MESH_SEGMENTS);
        
        /**
        Creates a skybox with 6 surfaces. For more information take a look at the "MeshGenerator" class.
        \see MeshGenerator
        \see MeshGenerator::createSkyBox
        */
        virtual Mesh* createSkyBox(video::Texture* (&TextureList)[6], f32 Radius = 50.0f);
        
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
            const io::stringc &Filename, const io::stringc &TexturePath = video::TEXPATH_IGNORE,
            const EMeshFileFormats Format = MESHFORMAT_UNKNOWN
        );
        
        /**
        Loads a model or instanciates it when it has already been loaded.
        \see SceneGraph::loadMesh
        \note When the root mesh will be deleted you also need to delete all derived meshes, too!
        */
        virtual Mesh* getMesh(
            const io::stringc &Filename, const io::stringc &TexturePath = video::TEXPATH_IGNORE,
            const EMeshFileFormats Format = MESHFORMAT_UNKNOWN
        );
        
        /**
        Saves a model to the disk.
        \param Model: 3D model which is to be saved.
        \param Filename: Filename/ path where the file shall be created.
        \param FileType: Specifies which file format the file shall get.
        By default the engine can decide it by the filename's extension
        */
        virtual bool saveMesh(
            Mesh* Model, const io::stringc &Filename, const EMeshFileFormats Format = MESHFORMAT_UNKNOWN
        );
        
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
            const io::stringc &Filename, const io::stringc &TexturePath = video::TEXPATH_IGNORE,
            const ESceneFileFormats Format = SCENEFORMAT_UNKNOWN, const s32 Flags = DEF_SCENE_FLAGS
        );
        
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
        Copies the specified scene node and returns a pointer to the new
        object or a null pointer if the template object was specified as a null pointer.
        \see SceneManager::copyNode
        */
        virtual SceneNode*  copyNode(const SceneNode*   TemplateObject);
        virtual Mesh*       copyNode(const Mesh*        TemplateObject);
        virtual Light*      copyNode(const Light*       TemplateObject);
        virtual Billboard*  copyNode(const Billboard*   TemplateObject);
        virtual Camera*     copyNode(const Camera*      TemplateObject);
        virtual Terrain*    copyNode(const Terrain*     TemplateObject);
        
        //! Deletes the specified scene node.
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
        virtual std::vector<SceneNode*> findChildren(const SceneNode* Parent) const;
        
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
        
        //! Renders the whole scene plain. This is used to render shadow maps.
        virtual void renderScenePlain(Camera* ActiveCamera);
        
        /**
        Renders the scene as a stero image. Use 3D glasses (red and green) to see the effect correctly.
        \param hCamera: Pointer to the camera object which gets the views location, projection etc.
        \param CamDegree: Degree or angle between to two points of view. With this parameter
        you can set the angle to concentrate the two eyes on a special point.
        \param CamDist: Distance between the two points of view or the two eyes.
        */
        virtual void renderSceneStereoImage(Camera* ActiveCamera, f32 CamDegree, f32 CamDist = 0.25f);
        
        //! Clears the whole scene from the specified objects. To delete all these objects use the SceneManager class.
        virtual void clearScene(
            bool isRemoveNodes = true, bool isRemoveMeshes = true,
            bool isRemoveCameras = true, bool isRemoveLights = true,
            bool isRemoveBillboards = true, bool isRemoveTerrains = true
        );
        
        /**
        Sets the wireframe mode of each "Mesh" and "Terrain" object (Billboards does not have wireframe mode yet).
        \param Type: Wireframe mode/ type (supported are: Solid, Lines, Points).
        */
        virtual void setWireframe(const video::EWireframeTypes Type);
        virtual void setWireframe(const video::EWireframeTypes TypeFront, const video::EWireframeTypes TypeBack);
        virtual void setRenderFace(const video::EFaceTypes Face);
        
        /**
        Enables or disables the lighting system. By default lighting is disabled.
        If lighting is disbaled the whole scene is full bright and now shadows (or better no shading) is proceeded.
        \param isLighting: Specifies if lighting shall be enabled or disabled (true/false)
        */
        virtual void setLighting(bool isLighting = true);
        virtual bool getLighting() const;
        
        virtual std::list<Mesh*> getMeshList() const;
        virtual std::list<Billboard*> getBillboardList() const;
        virtual std::list<Terrain*> getTerrainList() const;
        
        //! Returns count of mesh buffers in the whole scene. Only Mesh objects are included.
        virtual u32 getSceneMeshBufferCount() const;
        //! Returns count of vertices in the whole scene. Only Mesh objects are included.
        virtual u32 getSceneVertexCount() const;
        //! Returns count of triangles in the whole scene. Only Mesh objects are included.
        virtual u32 getSceneTriangleCount() const;
        //! Returns count of objects in the whole scene. Each kind of Node objects are included.
        virtual u32 getSceneObjectsCount() const;
        
        /**
        Sorts the list of renderable scene nodes.
        \param[in] Method Specifies the sorting method. If 'depth-sorting' is enabled every time
        the scene graph is rendered this function will be called with the parameter RENDERLIST_SORT_DEPTHDISTANCE.
        \see ERenderListSortMethods
        */
        virtual void sortRenderList(const ERenderListSortMethods Method, std::vector<RenderNode*> &ObjectList);
        virtual void sortRenderList(const ERenderListSortMethods Method);
        
        /* === Templates === */
        
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
        
        /* === Inline functions === */
        
        //! Returns the type of the scene graph.
        inline ESceneGraphs getGraphType() const
        {
            return GraphType_;
        }
        inline bool hasChildTree() const
        {
            return hasChildTree_;
        }
        
        inline const std::vector<RenderNode*>& getRenderList() const
        {
            return RenderList_;
        }
        inline const std::vector<Light*>& getLightList() const
        {
            return LightList_;
        }
        inline const std::vector<Camera*>& getCameraList() const
        {
            return CameraList_;
        }
        inline const std::vector<SceneNode*>& getNodeList() const
        {
            return NodeList_;
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
        Enables or disables the sorting for renderable nodes in by their depth distance to the view camera.
        \param[in] Enable Specifies whether depth sorting is to be enabled or disabled. By default true.
        \note Depth sorting is important for scenes with lots of transparent objects.
        If you are using a deferred renderer and/or you don't have transparent objects,
        disable this feature to increase your performance while rendering the scene.
        */
        inline void setDepthSorting(bool Enable)
        {
            DepthSorting_ = Enable;
        }
        //! Returns true if depth sorting is enabled. By default enabled.
        inline bool getDepthSorting() const
        {
            return DepthSorting_;
        }
        
        /* === Static functions === */
        
        /**
        Enables or disables reverse depth sorting used for early-z-culling.
        \todo This should be redesigned that the client programmer has
        more options about sorting the scene nodes.
        */
        static void setReverseDepthSorting(bool Enable);
        static bool getReverseDepthSorting();
        
    protected:
        
        friend class SceneNode;
        friend class RenderNode;
        friend class MaterialNode;
        friend class Mesh;
        
        /* === Functions === */
        
        SceneGraph(const ESceneGraphs Type);
        
        Mesh* integrateNewMesh(Mesh* NewMesh);
        
        /**
        Arranges the list of all renderable scene nodes, i.e. the objects will be transformed with the
        active view matrix and the list will be sorted if depth-sorting is enabled.
        This should be called before all renderable scene nodes will be rendered.
        */
        void arrangeRenderList(std::vector<RenderNode*> &ObjectList, const dim::matrix4f &BaseMatrix);
        /**
        Arranges the list of all light sources, i.e. the list will be sorted so that the nearest
        lights to the view camera are visible and the farthest away are invisible.
        */
        void arrangeLightList(std::vector<Light*> &ObjectList);
        
        //! Renders all (but maximal 8) fixed-function light sources.
        void renderLightsDefault(const dim::matrix4f &BaseMatrix);
        
        static void finishRenderScene();
        
        /* === Templates === */
        
        template <class T> void clearRenderObjectList(const ENodeTypes Type, std::vector<RenderNode*> &ObjectList)
        {
            for (std::vector<RenderNode*>::iterator it = ObjectList.begin(); it != ObjectList.end();)
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
        
        template <class T> bool removeObjectFromList(SceneNode* Object, std::vector<T*> &SearchList)
        {
            if (!Object)
                return false;
            
            for (typename std::vector< T*, std::allocator<T*> >::iterator it = SearchList.begin(); it != SearchList.end(); ++it)
            {
                if (*it == Object)
                {
                    SearchList.erase(it);
                    return true;
                }
            }
            
            return false;
        }
        
        template <class T> void addChildToList(
            const Node* ParentNode, std::vector<SceneNode*> &NodeList, const std::vector<T*> &SearchList) const
        {
            for (typename std::vector< T*, std::allocator<T*> >::const_iterator it = SearchList.begin(); it != SearchList.end(); ++it)
            {
                if ((*it)->getParent() == ParentNode)
                    NodeList.push_back(*it);
            }
        }
        
        template <class T> SceneNode* findChildInList(
            const SceneNode* ParentNode, const std::vector<T*> &SearchList, const io::stringc &Name) const
        {
            for (typename std::vector< T*, std::allocator<T*> >::const_iterator it = SearchList.begin(); it != SearchList.end(); ++it)
            {
                if ((*it)->getParent() == ParentNode && (*it)->getName() == Name)
                    return *it;
            }
            
            return 0;
        }
        
        template <class T, class L> void addNodeToList(
            const io::stringc &Name, std::list<SceneNode*> &NodeList, const std::vector<L*> &SearchList) const
        {
            for (typename std::vector< L*, std::allocator<L*> >::const_iterator it = SearchList.begin(); it != SearchList.end(); ++it)
            {
                if ((*it)->getName() == Name)
                    NodeList.push_back(*it);
            }
        }
        
        template <class T> SceneNode* findNodeInList(
            const io::stringc &Name, const std::vector<T*> &SearchList) const
        {
            for (typename std::vector< T*, std::allocator<T*> >::const_iterator it = SearchList.begin(); it != SearchList.end(); ++it)
            {
                if ((*it)->getName() == Name)
                    return *it;
            }
            
            return 0;
        }
        
        template <class T> std::list<T*> filterRenderNodeList(const ENodeTypes Type) const
        {
            std::list<T*> NodeList;
            
            for (std::vector<RenderNode*>::const_iterator it = RenderList_.begin(); it != RenderList_.end(); ++it)
            {
                if ((*it)->getType() == Type)
                    NodeList.push_back(static_cast<T*>(*it));
            }
            
            return NodeList;
        }
        
        /* === Members === */
        
        ESceneGraphs GraphType_;
        bool hasChildTree_;
        
        std::vector<SceneNode*>     NodeList_;      //!< \todo Rename this to "SceneNodes_".
        std::vector<Camera*>        CameraList_;    //!< \todo Rename this to "Cameras_".
        std::vector<Light*>         LightList_;     //!< \todo Rename this to "LightSources_".
        std::vector<RenderNode*>    RenderList_;    //!< \todo Rename this to "RenderNodes_".
        
        Camera* ActiveCamera_;
        Mesh* ActiveMesh_;
        
        video::EWireframeTypes WireframeFront_, WireframeBack_;
        
        bool DepthSorting_;
        
        static bool ReverseDepthSorting_;
        
};


} // /namespace scene

} // /namespace sp


#endif



// ================================================================================
