/*
 * Scene manager header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_SCENE_MANAGER_H__
#define __SP_SCENE_MANAGER_H__


#include "Base/spStandard.hpp"
#include "Base/spBasicMeshGenerator.hpp"
#include "Base/spGeometryStructures.hpp"
#include "SceneGraph/spSceneLight.hpp"

#include <list>


namespace sp
{
namespace scene
{


class Mesh;
class Billboard;
class Terrain;
class Camera;


/**
The scene manager is the object that manages all scene objects: meshes, lights, cameras etc.
Whereas a scene graph can references to these objects which are to be rendered. e.g. meshes which are used
to just describe a collision model should not be inserted into a scene graph.
\since Version 3.2
\see SceneGraph
*/
class SP_EXPORT SceneManager
{
    
    public:
        
        SceneManager();
        ~SceneManager();
        
        /* === Functions === */
        
        /**
        Creates an empty Mesh object.
        With this you can build your own 3d models at the beginning.
        Look at the "Mesh" class reference to learn more about building models.
        \return Pointer to an Mesh object which represents the 3D models.
        */
        Mesh* createMesh();
        
        /**
        Creates a standard primitive.
        \param Type: Type of the standard primitve (e.g. Cube, Sphere etc.).
        \param BuildContruct: Structure which holds the model contrstruction data (e.g. Segments, Radius etc.).
        */
        Mesh* createMesh(const EBasicMeshes Model, const SMeshConstruct &BuildConstruct = SMeshConstruct());
        
        /**
        Creates a "SuperShape". This is a 'sphere-like' model which can look very strange with the right parameter values ;-).
        \param ValueList: List of the 12 values which describe the construction of the super-shape.
        Play with it to learn how the values affect the final model.
        \param Detail: Segments which defines the detail of the model.
        */
        Mesh* createSuperShape(const f32 (&ValueList)[12], s32 Detail = DEF_MESH_SEGMENTS);
        
        /**
        Creates a skybox with 6 surfaces. For more information take a look at the "MeshGenerator" class.
        \see MeshGenerator
        \see MeshGenerator::createSkyBox
        */
        Mesh* createSkyBox(video::Texture* (&TextureList)[6], f32 Radius = 50.0f);
        
        /**
        Creates a height field. It is an alternate to a dynamic terrain.
        \param HeightMapTexture: Texture which holds the height-map data
        \param Detail: Segments for the height-field to be created
        */
        Mesh* createHeightField(const video::Texture* TexHeightMap, const s32 Segments = DEF_MESH_SEGMENTS);
        
        //! Creates a mesh out of the specified meshes.
        Mesh* createMeshList(std::list<Mesh*> MergeList, bool isOldDelete);
        
        //! Creates a mesh out of the specified mesh's surface.
        Mesh* createMeshSurface(Mesh* Model, u32 Surface);
        
        /**
        Loads a 3D model from file. The supported file formats are listed in the "EModelTypes" enumeration
        and at the "SoftPixel Engine"'s homepage under "features".
        \param[in] Filename Specifies the model filename which is to be loaded.
        \param[in] TexturePath Specifies the directory path where the model's textures are stored.
        When a texture filename is saved in the model file (e.g. "Texture1.bmp") the "TexturePath" is pushed to the front
        and loaded. e.g. [TexturePath + "Texture1.bmp"]. By default the path where the model is saved is used.
        \param[in] Format Specifies which file format the model has. By default the format will be determined automatically.
        \param[in] Flags Specifies the loading flags. This can be a combination of the EMeshLoaderFlags enumeration values.
        \see EMeshLoaderFlags
        */
        Mesh* loadMesh(
            io::stringc Filename, io::stringc TexturePath = video::TEXPATH_IGNORE,
            const EMeshFileFormats Format = MESHFORMAT_UNKNOWN, const s32 Flags = 0
        );
        
        /**
        Loads a model or instanciates it when it has already been loaded.
        \see SceneGraph::loadMesh
        \note When the root mesh will be deleted you also need to delete all derived meshes, too!
        */
        Mesh* getMesh(
            const io::stringc &Filename, io::stringc TexturePath = video::TEXPATH_IGNORE, const EMeshFileFormats Format = MESHFORMAT_UNKNOWN
        );
        
        /**
        Saves a model to the disk.
        \param Model: 3D model which is to be saved.
        \param Filename: Filename/ path where the file shall be created.
        \param FileType: Specifies which file format the file shall get.
        By default the engine can decide it by the filename's extension
        */
        bool saveMesh(Mesh* Model, io::stringc Filename, const EMeshFileFormats Format = MESHFORMAT_UNKNOWN);
        
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
        Mesh* loadScene(
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
        \todo Move this to "MeshGenerator" namespace
        */
        void createFurMesh(
            Mesh* Model, video::Texture* FurTexture,
            s32 LayerCount = 25, f32 HairLength = 0.2f, s32 HairCloseness = 2
        );
        
        //! Creates a simple scene node and returns the pointer to the new object.
        SceneNode* createNode();
        
        //! Creates a standard camera. Without cameras you cannot see the scene which shall be rendered.
        Camera* createCamera();
        
        /**
        Creates a dynamic light source.
        \param Type: Light's type. Supported are three types: Directional (by default), Point, Spot.
        Point and Spot lights can be volumetric using "Light::setVolumetric".
        \return Pointer to a Light object.
        */
        Light* createLight(const ELightModels Type = LIGHT_DIRECTIONAL);
        
        /**
        Creates a Billboard. Billboards (or Sprites) are mostly used for effects like Fire, Sparks, Lense flares etc.
        \param hTexture: Billboard's base texture. 0 is also allowed but then you only have a white quad.
        \return Pointer to a Billboard object.
        */
        Billboard* createBillboard(video::Texture* BaseTexture = 0);
        
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
        Terrain* createTerrain(
            const video::SHeightMapTexture &TextureHeightMap, const dim::size2di &Resolution, s32 GeoMIPLevels = DEF_GEOMIP_LEVELS
        );
        
        /**
        Copies the specified scene node and returns a pointer to the new
        object or a null pointer if the template object was specified as a null pointer.
        */
        SceneNode*  copyNode(const SceneNode*   TemplateObject);
        Mesh*       copyNode(const Mesh*        TemplateObject);
        Light*      copyNode(const Light*       TemplateObject);
        Billboard*  copyNode(const Billboard*   TemplateObject);
        Camera*     copyNode(const Camera*      TemplateObject);
        Terrain*    copyNode(const Terrain*     TemplateObject);
        
        /**
        Deletes the specified object. All renderer resources are also released.
        \return True if the object could be deleted.
        Otherwise the object does not exist in the SceneManager's list or an other error has been occured.
        */
        bool deleteNode(SceneNode* Object);
        
        /**
        Tries to find each node with the specified name.
        \param Name: Name of the wanted nodes.
        \return List with each Node object which could found.
        */
        std::list<SceneNode*> findNodes(const io::stringc &Name) const;
        
        /**
        Tries to find the specified node.
        \param Name: Name of the wanted node.
        \return Pointer to the Node object which could found. If no object could found the return value is 0.
        */
        SceneNode* findNode(const io::stringc &Name) const;
        
        /**
        Tries to find each child of the specified parent.
        \param ParentNode: Pointer to the Node object which is the parent of the wanted children.
        \return List with each Node object which could found.
        */
        std::list<SceneNode*> findChildren(const SceneNode* Parent) const;
        
        /**
        Tries to find the specified child of the specified parent.
        \param ParentNode: Pointer to the Node object which is the parent of the wanted child.
        \param Name: Name of the wanted child.
        \return Pointer to the child Node object if the search was successful. Otherwise 0.
        */
        SceneNode* findChild(const SceneNode* Parent, const io::stringc &Name) const;
        
        //! Deletes the specified animation.
        void deleteAnimation(Animation* Anim);
        
        //! Deletes all animations.
        void clearAnimations();
        
        //! Updates all animations.
        void updateAnimations();
        
        //! Clears the whole scene from the specified objects.
        void clearScene(
            bool isDeleteNodes = true, bool isDeleteMeshes = true,
            bool isDeleteCameras = true, bool isDeleteLights = true,
            bool isDeleteBillboards = true, bool isDeleteTerrains = true
        );
        
        /**
        Removes the specified texture from each "Mesh" and "Terrain" object which use this texture.
        \param hTexture: Texture which is to be removed.
        */
        void removeTexture(const video::Texture* Tex);
        
        //! Returns count of mesh buffers in the whole scene. Only Mesh objects are included.
        u32 getSceneMeshBufferCount() const;
        //! Returns count of vertices in the whole scene. Only Mesh objects are included.
        u32 getSceneVertexCount() const;
        //! Returns count of triangles in the whole scene. Only Mesh objects are included.
        u32 getSceneTriangleCount() const;
        //! Returns count of objects in the whole scene. Each kind of Node objects are included.
        u32 getSceneObjectsCount() const;
        
        /* === Templates === */
        
        /**
        Creates a Camera. Without cameras you cannot see the scene which shall be rendered.
        \return Pointer to the specified Camera or Camera derived class object.
        */
        template <class T> T* createCamera()
        {
            T* NewCamera = MemoryManager::createMemory<T>("scene::Camera");
            CameraList_.push_back(NewCamera);
            return NewCamera;
        }
        
        //! Creates a new animation. Use the NodeAnimation, MorphTargetAnimation and SkeletalAnimation classes.
        template <class T> T* createAnimation(const io::stringc &Name = "")
        {
            T* NewAnim = MemoryManager::createMemory<T>(Name.size() ? Name : "Animation");
            NewAnim->setName(Name);
            AnimationList_.push_back(NewAnim);
            return NewAnim;
        }
        
        /* === Inline functions === */
        
        inline const std::list<Mesh*>& getMeshList() const
        {
            return MeshList_;
        }
        inline const std::list<Billboard*>& getBillboardList() const
        {
            return BillboardList_;
        }
        inline const std::list<Terrain*>& getTerrainList() const
        {
            return TerrainList_;
        }
        inline const std::list<Light*>& getLightList() const
        {
            return LightList_;
        }
        inline const std::list<Camera*>& getCameraList() const
        {
            return CameraList_;
        }
        inline const std::list<SceneNode*>& getNodeList() const
        {
            return NodeList_;
        }
        inline const std::list<Animation*>& getAnimationList() const
        {
            return AnimationList_;
        }
        
        /* === Static functions === */
        
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
        
        static EMeshFileFormats getMeshFileFormat(const io::stringc &Filename, const EMeshFileFormats DefaultFormat);
        static ESceneFileFormats getSceneFileFormat(const io::stringc &Filename, const ESceneFileFormats DefaultFormat);
        
    private:
        
        /* === Templates === */
        
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
        
        template <class T> void filterNodeByName(
            const io::stringc &Name, std::list<SceneNode*> &NodeList, const std::list<T*> &SearchList) const
        {
            for (typename std::list< T*, std::allocator<T*> >::const_iterator it = SearchList.begin(); it != SearchList.end(); ++it)
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
        
        template <class T> T* copySceneNode(std::list<T*> &NodeList, const T* TemplateObject)
        {
            if (TemplateObject)
            {
                T* NewObject = TemplateObject->copy();
                NodeList.push_back(NewObject);
                return NewObject;
            }
            return 0;
        }
        
        template <class T> void deleteSceneNode(std::list<T*> &NodeList, T* Object)
        {
            MemoryManager::removeElement(NodeList, Object, true);
        }
        
        /* === Members === */
        
        std::list<SceneNode*>   NodeList_;
        
        std::list<Mesh*>        MeshList_;
        std::list<Billboard*>   BillboardList_;
        std::list<Terrain*>     TerrainList_;
        std::list<Camera*>      CameraList_;
        std::list<Light*>       LightList_;
        
        std::list<Animation*>   AnimationList_;
        
        std::map<std::string, Mesh*> MeshMap_;
        
        static const video::VertexFormat* DefaultVertexFormat_;
        static video::ERendererDataTypes DefaultIndexFormat_;
        
        static bool TextureLoadingState_;
        
};


} // /namespace scene

} // /namespace sp


#endif



// ================================================================================
