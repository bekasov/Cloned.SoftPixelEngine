/*
 * Importer header
 * 
 * This file is part of the "SoftPixel Scene Importer/Exporter" (Copyright (c) 2012 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef __SPS_IMPORTER_H__
#define __SPS_IMPORTER_H__


#include "spsCore.h"
#include "spsFormatHandler.h"
#include "spsFileReader.h"


namespace sps
{


class SPS_EXPORT SpSceneImporter : public SpSceneFormatHandler
{
    
    public:
        
        SpSceneImporter();
        virtual ~SpSceneImporter();
        
        bool ImportScene(const std::string &Filename);
        
    protected:
        
        /* === Type definitions === */
        
        typedef bool (SpSceneImporter::*ReadObjectProc)(void);
        
        /* === Macros === */
        
        static const ReadObjectProc ReadObjectProcList[LUMP_COUNT];
        
        /* === User functions === */
        
        virtual bool NotificationLump(const std::string &Name, int32 Progress, int32 MaxProgress);
        
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
        
        /* === Reading functions === */
        
        bool ReturnWithError();
        bool ReturnWithError(const std::string &Message, const EErrorTypes Type = ERROR_DEFAULT);
        
        void ComputeProgressMaximum();
        
        bool ReadHeader();
        bool ReadLump(const SpLump &Lump, const std::string &Name, ReadObjectProc Proc);
        
        bool ReadSceneConfig    ();
        bool ReadMesh           ();
        bool ReadCamera         ();
        bool ReadWayPoint       ();
        bool ReadLight          ();
        bool ReadBoundVolume    ();
        bool ReadSound          ();
        bool ReadSprite         ();
        bool ReadAnimNode       ();
        bool ReadTexture        ();
        bool ReadTextureClass   ();
        bool ReadLightmap       ();
        bool ReadLightmapScene  ();
        bool ReadShaderClass    ();
        
        bool ReadBaseObject                     (SpBaseObject                       &Object                     );
        bool ReadMaterial                       (SpMaterial                         &Material                   );
        bool ReadViewCulling                    (SpViewCulling                      &ViewCulling                );
        bool ReadScriptTemplateData             (SpScriptTemplateData               &ScriptTemplate             );
        bool ReadScriptData                     (SpScriptData                       &ScriptData                 );
        bool ReadShaderRTObject                 (SpShaderRTObject                   &ShaderRTObject             );
        bool ReadAnimationKeyframe              (SpAnimationKeyframe                &Keyframe                   );
        bool ReadAnimation                      (SpAnimation                        &Animation                  );
        bool ReadAnimationObject                (SpAnimationObject                  &AnimObject                 );
        bool ReadSurfaceLayer                   (SpSurfaceLayer                     &SurfaceLayer               );
        bool ReadSurface                        (SpSurface                          &Surface                    );
        bool ReadCollision                      (SpCollision                        &Collision                  );
        bool ReadMeshConstructionBasic          (SpMeshConstructionBasic            &Construct                  );
        bool ReadMeshConstructionProcStaircase  (SpMeshConstructionProcStaircase    &Construct                  );
        bool ReadMeshConstructionProc           (SpMeshConstructionProc             &Construct                  );
        bool ReadMeshConstructionResource       (SpMeshConstructionResource         &Construct                  );
        bool ReadMeshConstruction               (SpMeshConstruction                 &Construct                  );
        bool ReadAnimScriptBaseObject           (SpAnimScriptBaseObject             &Object                     );
        bool ReadTextureClassLayer              (SpTextureClassLayer                &TexLayer                   );
        bool ReadLightmapSceneVertex            (SpLightmapSceneVertex              &Vertex, uint8 LayerCount   );
        bool ReadLightmapSceneLayer             (SpLightmapSceneLayer               &Layer                      );
        bool ReadLightmapSceneSurface           (SpLightmapSceneSurface             &Surface                    );
        bool ReadShader                         (SpShader                           &Shader                     );
        bool ReadShaderParameter                (SpShaderParameter                  &Param                      );
        
        /* === Members === */
        
        SpFileReader File_;
        
        int32 Progress_, ProgressMax_;
        
};


} // /namespace sps


#endif



// ================================================================================
