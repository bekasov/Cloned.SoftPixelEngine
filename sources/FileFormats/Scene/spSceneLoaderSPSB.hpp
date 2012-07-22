/*
 * SoftPixel Sandbox Scene loader header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_SCENELOADER_SPSB_H__
#define __SP_SCENELOADER_SPSB_H__


#include "Base/spStandard.hpp"

#ifdef SP_COMPILE_WITH_SCENELOADER_SPSB


#include "Base/spInputOutputLog.hpp"
#include "Base/spInputOutputFileSystem.hpp"
#include "Base/spDimension.hpp"
#include "Base/spBasicMeshGenerator.hpp"
#include "RenderSystem/spTextureBase.hpp"
#include "FileFormats/Mesh/spMeshLoader.hpp"
#include "SoundSystem/spSoundDevice.hpp"

#include <list>


namespace sp
{
namespace scene
{


class Mesh;
class Light;
class Camera;
class Billboard;


enum ESceneLoaderFlags
{
    SCENEFLAG_ABSOLUTEPATH      = 0x00000001, //!< Load all resources with the absolute path. By default relative paths are used.
    
    SCENEFLAG_CONFIG            = 0x00000002, //!< Load scene configuration. By default used.
    
    SCENEFLAG_MESHES            = 0x00000004, //!< Load meshes. By default used.
    SCENEFLAG_LIGHTS            = 0x00000008, //!< Load light sources. By default used.
    SCENEFLAG_CAMERAS           = 0x00000010, //!< Load cameras. By default used.
    SCENEFLAG_WAYPOINTS         = 0x00000020, //!< Load way points. By default used.
    SCENEFLAG_BOUNDVOLUMES      = 0x00000040, //!< Load bounding volumes. By default used.
    SCENEFLAG_SOUNDS            = 0x00000080, //!< Load sounds. By default used.
    SCENEFLAG_SPRITES           = 0x00000100, //!< Load sprites. By default used.
    SCENEFLAG_ANIMNODES         = 0x00000200, //!< Load animation nodes. By default used.
    
    SCENEFLAG_TEXTURES          = 0x00010000, //!< Load textures and texture classes. By default used.
    SCENEFLAG_LIGHTMAPS         = 0x00020000, //!< Load lightmaps and lightmap scene. By default used.
    SCENEFLAG_SHADERS           = 0x00040000, //!< Load shaders and shader classes. By default used.
    
    //! All objects: meshes, sprites, cameras etc.
    SCENEFLAG_OBJECTS       =
        SCENEFLAG_MESHES |
        SCENEFLAG_LIGHTS |
        SCENEFLAG_CAMERAS |
        SCENEFLAG_WAYPOINTS |
        SCENEFLAG_BOUNDVOLUMES |
        SCENEFLAG_SOUNDS |
        SCENEFLAG_SPRITES |
        SCENEFLAG_ANIMNODES,
    
    //! All options are used.
    SCENEFLAG_ALL           = ~0,
};


/**
"SoftPixel Sandbox Scene" loader class. This class is used to load 3D scenes created by the "SoftPixel Sandbox" (Official world editor of the "SoftPixel Engine").
You can simply load the basic construction of a scene by calling: spScene->loadScene("YourSceneFile.spsb").
When you want to use this scene format in your own games you have to write a child class which inherits from this one.
Then you only have to overwrite the "apply..." and/or "observe..." functions to extend your own features.
In this way you can e.g. use bump-mapping or other shader effects by manipulating the texture mapping.
Just overwrite the "applyTextureMapping" function. For more information see the documentation of the individual virtual functions.
*/
class SP_EXPORT SceneLoaderSPSB : public SceneLoader
{
    
    public:
        
        SceneLoaderSPSB();
        virtual ~SceneLoaderSPSB();
        
        /* === Functions === */
        
        virtual Mesh* loadScene(const io::stringc &Filename, const io::stringc &TexturePath, const s32 Flags);
        
    protected:
        
        /* === Macros === */
        
        static const s32 MAGIC_NUMBER;
        static const s32 VERSION_MIN_SUPPORT;
        static const s32 VERSION_MAX_SUPPORT;
        
        /* === Enumerations & structures === */
        
        #include "spSceneLoaderSPSBStructures.hpp"
        
        /* === Functions === */
        
        bool readHeader();
        bool seekLump(const SLump &Lump);
        
        virtual void readSceneConfig();
        
        virtual void readMeshes();
        virtual void readLights();
        virtual void readCameras();
        virtual void readWayPoints();
        virtual void readBoundVolumes();
        virtual void readSounds();
        virtual void readSprites();
        virtual void readAnimNodes();
        
        virtual void readTextures();
        virtual void readTextureClasses();
        virtual void readLightmaps();
        virtual void readLightmapScene();
        virtual void readShaders();
        
        virtual void readBaseObject(SBaseObject &Object);
        virtual void readViewCulling(SViewCulling &ViewCulling);
        virtual void readScriptTemplates(std::vector<SScriptData> &ScriptDataList);
        virtual void readScriptData(SScriptData &ScriptData);
        virtual void readShaderRTObject(SShaderRTObject &Object);
        virtual void readAnimationObject(scene::SceneNode* Node);
        
        //! Applies all queues: parent node setup, bot way point links etc.
        virtual void applyQueues();
        //! Applies material states.
        virtual void applyMaterial(const SMaterial &Material, video::MaterialStates* MatStates);
        //! Applies vertex- and index format of the specified hardware mesh buffer.
        virtual void applyMeshBufferFormat(
            video::MeshBuffer* Surface, video::VertexFormat* VertexFormat, const video::ERendererDataTypes IndexFormat
        );
        //! Applies texture mapping for the specified surface.
        virtual void applyTextureMapping(
            video::MeshBuffer* Surface, video::Texture* Tex, u32 TexId, u8 Layer
        );
        //! Applies texture setup configuration.
        virtual void applyTexture(video::Texture* Tex, const STextureConfig &TexConfig, u32 TexId);
        //! Applies base object information: transformation, name and visibility.
        virtual void applyBaseObject(scene::SceneNode* Node, const SBaseObject &Object);
        //! Applies collision- and picking model. This is just an interface function.
        virtual void applyCollision(scene::Mesh* Object, const ECollisionModels CollModel, const scene::EPickingTypes PickModel);
        //! Applies the script templates. Will be called as the last operation when an object is created.
        virtual void applyScriptTemplates(const SBaseObject &Object);
        
        //! Applies a basic mesh construction using the scene::BasicMeshGenerator class.
        virtual scene::Mesh* applyBasicMesh(const scene::EBasicMeshes Type, const scene::SMeshConstruct &Construct);
        //! Applies a resource mesh loading using the scene::SceneGraph class. This just calls "spScene->loadMesh(AbsolutePath)".
        virtual scene::Mesh* applyResourceMesh(const io::stringc &AbsolutePath);
        //! Applies a bounding volume.
        virtual void applyBoundingVolume(const SBaseObject &BaseObject);
        //! Applies a sound object.
        virtual audio::Sound* applySound(const SSound &Sound);
        //! Applies a sprite object.
        virtual scene::Billboard* applySprite(const SSprite &Sprite);
        //! Applies an animation-node object.
        virtual void applyAnimNode(const SBaseObject &BaseObject);
        
        //! Observes the camera render target texture. This is just an interface function.
        virtual void observeCameraRenderTarget(scene::Camera* Cam, video::Texture* Tex);
        //! Observes the lightmap usage information for the specified light source. This is just an interface function.
        virtual void observeLight(scene::Light* Object, const SBaseObject &BaseObject, bool UseForLightmaps);
        
        //! Completes the mesh construction. This will be called after a mesh has been created and set up.
        virtual void completeMeshConstruction(scene::Mesh* Object, const SBaseObject &BaseObject);
        
        virtual io::stringc getAbsolutePath(const io::stringc &Path) const;
        
        /* === Inline functions === */
        
        inline io::stringc readString() const
        {
            return File_->readStringData();
        }
        inline void readBuffer(void* Buffer, u32 Size, u32 Count = 1) const
        {
            File_->readBuffer(Buffer, Size, Count);
        }
        inline dim::matrix4f readMatrix() const
        {
            return File_->readMatrix<f32>();
        }
        
        /* === Templates === */
        
        template <typename T> inline T readValue() const
        {
            return File_->readValue<T>();
        }
        
        /* === Members === */
        
        s32 FormatVersion_;
        SHeader Header_;
        
        std::map<u32, video::Texture*> Textures_;
        std::map<u32, STextureClass> TextureClasses_;
        std::map<u32, scene::SceneNode*> ObjectIdMap_;
        std::map<u32, dim::matrix4f> AnimNodeMatrixMap_;
        
        std::vector<video::Texture*> LightmapTextures_;
        
        std::list<SWayPoint> WayPoints_;
        std::list<SParentQueue> QueueParents_;
        
        io::stringc ResourcePath_;
        io::stringc ScriptTemplateFilename_;
        scene::Mesh* SkyBox_;
        
};


} // /namespace scene

} // /namespace sp


#endif

#endif



// ================================================================================
