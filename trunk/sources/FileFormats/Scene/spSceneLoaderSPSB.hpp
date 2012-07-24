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
#include "SceneGraph/Collision/spCollisionConfigTypes.hpp"

#include "Plugins/SpSbImportExport/spsImporter.h"

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
"SoftPixel Sandbox Scene" loader class. This class is used to load 3D scenes created by the
"SoftPixel Sandbox" (Official world editor of the "SoftPixel Engine").
You can simply load the basic construction of a scene by calling: spScene->loadScene("YourSceneFile.spsb").
When you want to use this scene format in your own games you have to write a sub-class which inherits from this one.
*/
class SP_EXPORT SceneLoaderSPSB : public SceneLoader, public sps::SpSceneImporter
{
    
    public:
        
        SceneLoaderSPSB();
        virtual ~SceneLoaderSPSB();
        
        /* === Functions === */
        
        virtual Mesh* loadScene(const io::stringc &Filename, const io::stringc &TexturePath, const s32 Flags);
        
    protected:
        
        /* === Enumerations === */
        
        enum ETextureClassLayerTypes
        {
            TEXCLASSLAYER_CUSTOM = 0,
            TEXCLASSLAYER_LIGHTMAP,
            TEXCLASSLAYER_SHADER,
        };
        
        /* === Structures === */
        
        struct SParentQueue
        {
            SParentQueue() :
                Object  (0),
                ParentId(0)
            {
            }
            ~SParentQueue()
            {
            }
            
            /* Members */
            SceneNode* Object;
            u32 ParentId;
        };
        
        /* === Functions === */
        
        virtual void Error(const std::string &Message, const EErrorTypes Type = ERROR_DEFAULT);
        virtual void Warning(const std::string &Message, const EWarningType Type = WARNING_DEFAULT);
        
        virtual bool CatchHeader        (const SpHeader         &Header);
        
        virtual bool CatchSceneConfig   (const SpSceneConfig    &Object);
        virtual bool CatchMesh          (const SpMesh           &Object);
        virtual bool CatchCamera        (const SpCamera         &Object);
        virtual bool CatchWayPoint      (const SpWayPoint       &Object);
        virtual bool CatchLight         (const SpLight          &Object);
        virtual bool CatchBoundVolume   (const SpBoundVolume    &Object);
        virtual bool CatchSound         (const SpSound          &Object);
        virtual bool CatchSprite        (const SpSprite         &Object);
        virtual bool CatchAnimNode      (const SpAnimNode       &Object);
        virtual bool CatchTexture       (const SpTexture        &Object);
        virtual bool CatchTextureClass  (const SpTextureClass   &Object);
        virtual bool CatchLightmap      (const SpLightmap       &Object);
        virtual bool CatchLightmapScene (const SpLightmapScene  &Object);
        virtual bool CatchShaderClass   (const SpShaderClass    &Object);
        
        //! Returns the final file path.
        virtual io::stringc getFinalPath(const io::stringc &Path) const;
        //! Applies all queues: parent node setup, bot way point links etc.
        virtual void applyQueues();
        virtual void addObjectToParentQueue(SceneNode* Node, u32 ParentId);
        
        virtual SpTextureClass* findTextureClass(u32 Id);
        virtual video::Texture* findTexture(u32 Id);
        virtual video::ShaderTable* findShaderClass(u32 Id);
        virtual KeyframeTransformation findAnimNodeTransformation(u32 Id);
        
        //! Applies base object information: transformation, name and visibility.
        virtual bool setupBaseObject            (SceneNode* Node, const SpBaseObject &Object);
        virtual bool setupViewCulling           (SceneNode* Node, const SpViewCulling &Object);
        virtual bool setupAnimation             (SceneNode* Node, const SpAnimationObject &Object);
        virtual bool setupMaterial              (video::MaterialStates* Material, const SpMaterial &Object);
        virtual bool setupSurface               (Mesh* MeshObj, video::MeshBuffer* Surface, const SpSurface &Object, u32 Index);
        virtual bool setupSurfaceTexture        (video::MeshBuffer* Surface, video::Texture* Tex, u32 TexId, u8 Layer);
        virtual bool setupSurfaceTextureClass   (video::MeshBuffer* Surface, const SpTextureClassLayer &TexClassLayer, bool NeedDefaultTex, u8 Layer);
        virtual bool setupMeshBufferFormat      (video::MeshBuffer* Surface, video::VertexFormat* VxFormat, const video::ERendererDataTypes IxFormat);
        virtual bool setupMeshCollision         (Mesh* MeshObj, const ECollisionModels CollModel, const EPickingTypes PickModel);
        virtual bool setupMeshShader            (Mesh* MeshObj, u32 ShaderClassId);
        virtual bool setupScriptTemplates       (SceneNode* Node, const SpBaseObject &Object, const SpScriptData &Script);
        virtual bool setupTexture               (video::Texture* Tex, const SpTexture &Object);
        virtual bool setupLightmapSceneSurface  (video::MeshBuffer* Surface, const SpLightmapSceneSurface &Object);
        
        virtual bool completeMeshConstruct  (Mesh*      MeshObj,    const SpMesh    &Object);
        virtual bool completeCameraConstruct(Camera*    CameraObj,  const SpCamera  &Object);
        virtual bool completeLightConstruct (Light*     LightObj,   const SpLight   &Object);
        virtual bool completeSpriteConstruct(Billboard* SpriteObj,  const SpSprite  &Object);
        
        virtual Mesh*           createSkyBox        (const std::string (&SkyBoxTexFilenames)[6]);
        virtual Mesh*           createMeshBasic     (const SpMeshConstructionBasic &Construct);
        virtual Mesh*           createMeshResource  (const SpMeshConstructionResource &Construct);
        virtual video::Shader*  createShader        (const SpShader &Object, video::ShaderTable* ShaderClassObj, const video::EShaderTypes Type);
        
        virtual video::VertexFormat* getVertexFormat(s8 VertexFormat);
        
        /* === Templates === */
        
        template <typename T> T* findObjectById(u32 Id, std::map<u32, T*> &Map, const io::stringc &Name)
        {
            if (Id > 0)
            {
                typename std::map<u32, T*>::iterator it = Map.find(Id);
                if (it != Map.end())
                    return it->second;
                else
                    Error(("Wrong ID number for " + Name + " (" + io::stringc(Id) + ")").str());
            }
            return 0;
        }
        
        /* === Inline functions === */
        
        inline dim::point2df convert(const SpVector2 &Vec) const
        {
            return *(const dim::point2df*)(&Vec.x);
        }
        inline dim::vector3df convert(const SpVector3 &Vec) const
        {
            return *(const dim::vector3df*)(&Vec.x);
        }
        inline dim::matrix4f convert(const SpMatrix4 &Mat) const
        {
            return *(const dim::matrix4f*)(&Mat.m[0]);
        }
        inline video::color convert(const SpColor &Color) const
        {
            return *(const video::color*)(&Color.r);
        }
        
    private:
        
        /* === Members === */
        
        std::map<u32, SceneNode*> ObjectIdMap_;
        std::map<u32, video::Texture*> Textures_;
        std::map<u32, video::ShaderTable*> ShaderClasses_;
        std::map<u32, SpTextureClass> TextureClasses_;
        std::map<u32, KeyframeTransformation> AnimNodeTransMap_;
        
        std::vector<video::Texture*> LightmapTextures_;
        
        //std::list<SWayPoint> WayPoints_;
        std::list<SParentQueue> QueueParents_;
        
        io::stringc ResourcePath_;
        io::stringc ScriptTemplateFilename_;
        
        bool HasLightmaps_;
        
};


} // /namespace scene

} // /namespace sp


#endif

#endif



// ================================================================================
