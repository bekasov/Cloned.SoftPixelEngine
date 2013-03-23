/*
 * Exporter header
 * 
 * This file is part of the "SoftPixel Scene Importer/Exporter" (Copyright (c) 2012 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef __SPS_EXPORTER_H__
#define __SPS_EXPORTER_H__


#include "spsCore.h"
#include "spsFormatHandler.h"
#include "spsFileWriter.h"


namespace sps
{


class SPS_EXPORT SpSceneExporter : public SpSceneFormatHandler
{
    
    public:
        
        SpSceneExporter();
        virtual ~SpSceneExporter();
        
        bool ExportScene(const std::string &Filename, const EFormatVersions Version = FORMATVERSION_LATEST);
        
    protected:
        
        /* === Type definitions === */
        
        typedef bool (SpSceneExporter::*WriteObjectProc)(void);
        
        /* === Macros === */
        
        static const WriteObjectProc WriteObjectProcList[LUMP_COUNT];
        
        /* === User functions === */
        
        virtual uint32 GetLumpElementCount(const ELumpDirectories Lump) const = 0;
        
        virtual bool CommitSceneConfig  (SpSceneConfig      &Object);
        virtual bool CommitMesh         (SpMesh             &Object);
        virtual bool CommitCamera       (SpCamera           &Object);
        virtual bool CommitWayPoint     (SpWayPoint         &Object);
        virtual bool CommitLight        (SpLight            &Object);
        virtual bool CommitBoundVolume  (SpBoundVolume      &Object);
        virtual bool CommitSound        (SpSound            &Object);
        virtual bool CommitSprite       (SpSprite           &Object);
        virtual bool CommitAnimNode     (SpAnimNode         &Object);
        virtual bool CommitTexture      (SpTexture          &Object);
        virtual bool CommitTextureClass (SpTextureClass     &Object);
        virtual bool CommitLightmap     (SpLightmap         &Object);
        virtual bool CommitLightmapScene(SpLightmapScene    &Object);
        virtual bool CommitShaderClass  (SpShaderClass      &Object);
        
        /* === Writing functions === */
        
        bool ReturnWithError();
        bool ReturnWithError(const std::string &Message, const EErrorTypes Type = ERROR_DEFAULT);
        
        bool WriteHeader(int32 Version);
        bool WriteLump(const ELumpDirectories Type, const SpLump &Lump, const std::string &Name, WriteObjectProc Proc);
        
        bool WriteSceneConfig   ();
        bool WriteMesh          ();
        bool WriteCamera        ();
        bool WriteWayPoint      ();
        bool WriteLight         ();
        bool WriteBoundVolume   ();
        bool WriteSound         ();
        bool WriteSprite        ();
        bool WriteAnimNode      ();
        bool WriteTexture       ();
        bool WriteTextureClass  ();
        bool WriteLightmap      ();
        bool WriteLightmapScene ();
        bool WriteShaderClass   ();
        bool WriteStoryboardItem();
        
        bool WriteBaseObject                    (const SpBaseObject                     &Object                     );
        bool WriteMaterial                      (const SpMaterial                       &Material                   );
        bool WriteViewCulling                   (const SpViewCulling                    &ViewCulling                );
        bool WriteScriptTemplateData            (const SpScriptTemplateData             &ScriptTemplate             );
        bool WriteScriptData                    (const SpScriptData                     &ScriptData                 );
        bool WriteShaderRTObject                (const SpShaderRTObject                 &ShaderRTObject             );
        bool WriteAnimationKeyframe             (const SpAnimationKeyframe              &Keyframe                   );
        bool WriteAnimation                     (const SpAnimation                      &Animation                  );
        bool WriteAnimationObject               (const SpAnimationObject                &AnimObject                 );
        bool WriteSurfaceLayer                  (const SpSurfaceLayer                   &SurfaceLayer               );
        bool WriteSurface                       (const SpSurface                        &Surface                    );
        bool WriteCollision                     (const SpCollision                      &Collision                  );
        bool WriteMeshConstructionBasic         (const SpMeshConstructionBasic          &Construct                  );
        bool WriteMeshConstructionProcStaircase (const SpMeshConstructionProcStaircase  &Construct                  );
        bool WriteMeshConstructionProc          (const SpMeshConstructionProc           &Construct                  );
        bool WriteMeshConstructionResource      (const SpMeshConstructionResource       &Construct                  );
        bool WriteMeshConstruction              (const SpMeshConstruction               &Construct                  );
        bool WriteAnimScriptBaseObject          (const SpAnimScriptBaseObject           &Object                     );
        bool WriteTextureClassLayer             (const SpTextureClassLayer              &TexLayer                   );
        bool WriteLightmapSceneVertex           (const SpLightmapSceneVertex            &Vertex, uint8 LayerCount   );
        bool WriteLightmapSceneLayer            (const SpLightmapSceneLayer             &Layer                      );
        bool WriteLightmapSceneSurface          (const SpLightmapSceneSurface           &Surface                    );
        bool WriteShader                        (const SpShader                         &Shader                     );
        bool WriteShaderParameter               (const SpShaderParameter                &Param                      );
        
        /* === Members === */
        
        SpFileWriter File_;
        
};


} // /namespace sps


#endif



// ================================================================================
