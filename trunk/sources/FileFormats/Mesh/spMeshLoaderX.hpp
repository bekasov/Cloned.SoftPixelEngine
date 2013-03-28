/*
 * Mesh loader X header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_MESHLOADER_X_H__
#define __SP_MESHLOADER_X_H__


#include "Base/spStandard.hpp"

#ifdef SP_COMPILE_WITH_MESHLOADER_X


#include "Base/spInputOutputLog.hpp"
#include "Base/spInputOutputFileSystem.hpp"
#include "Base/spDimension.hpp"
#include "RenderSystem/spTextureBase.hpp"
#include "FileFormats/Mesh/spMeshLoader.hpp"

#include <vector>
#include <string>


namespace sp
{
namespace scene
{


//#define _DEBX_

//! \todo This class is out of date and should be rewritten completely.
class SP_EXPORT MeshLoaderX : public MeshLoader
{
    
    public:
        
        MeshLoaderX();
        ~MeshLoaderX();
        
        Mesh* loadMesh(const io::stringc &Filename, const io::stringc &TexturePath);
        
    private:
        
        /* === Enumerations === */
        
        enum ETemplateTypes
        {
            TEMPLATE_UNKNOWN,
            
            TEMPLATE_ANIMATION,
            TEMPLATE_ANIMATIONKEY,
            TEMPLATE_ANIMATIONOPTIONS,
            TEMPLATE_ANIMATIONSET,
            TEMPLATE_BOOLEAN,
            TEMPLATE_BOOLEAN2D,
            TEMPLATE_COLORRGB,
            TEMPLATE_COLORRGBA,
            TEMPLATE_COORDS2D,
            TEMPLATE_FLOATKEYS,
            TEMPLATE_FRAME,
            TEMPLATE_FRAMETRANSFORMMATRIX,
            TEMPLATE_HEADER,
            TEMPLATE_INDEXEDCOLOR,
            TEMPLATE_MATERIAL,
            TEMPLATE_MATRIX4X4,
            TEMPLATE_MESH,
            TEMPLATE_MESHFACE,
            TEMPLATE_MESHFACEWRAPS,
            TEMPLATE_MESHMATERIALLIST,
            TEMPLATE_MESHNORMALS,
            TEMPLATE_MESHTEXTURECOORDS,
            TEMPLATE_MESHVERTEXCOLORS,
            TEMPLATE_QUATERNION,
            TEMPLATE_TEXTUREFILENAME,
            TEMPLATE_TIMEDFLOATKEYS,
            TEMPLATE_VECTOR,
        };
        
        enum EMemberTypes
        {
            MEMBER_UNKNOWN,
            
            MEMBER_ARRAY,
            MEMBER_BINARY,
            MEMBER_BINARY_RESOURCE,
            MEMBER_CHAR,
            MEMBER_CSTRING,
            MEMBER_DOUBLE,
            MEMBER_DWORD,
            MEMBER_FLOAT,
            MEMBER_SDWORD,
            MEMBER_STRING,
            MEMBER_SWORD,
            MEMBER_TEMPLATE,
            MEMBER_UCHAR,
            MEMBER_ULONGLONG,
            MEMBER_UNICODE,
            MEMBER_WORD,
        };
        
        enum EAnimationKeyTypes
        {
            ANIMKEY_POSITION    = 2,
            ANIMKEY_ROTATION    = 0,
            ANIMKEY_SCALE       = 1,
        };
        
        /* === Structures === */
        
        struct STemplateX;
        struct SRegisteredTemplateX;
        
        struct STemplateMemberX
        {
            STemplateMemberX(const EMemberTypes InitType = MEMBER_UNKNOWN);
            STemplateMemberX(STemplateX* InitTemplate);
            ~STemplateMemberX();
            
            /* Members */
            EMemberTypes Type;
            
            STemplateX* Template;
            std::vector<STemplateMemberX> ArrayList;
            
            s32 ValInteger;
            f32 ValFloat;
            f64 ValDouble;
            io::stringc ValString;
        };
        
        struct STemplateX
        {
            STemplateX();
            ~STemplateX();
            
            /* Members */
            ETemplateTypes Type;
            io::stringc TypeName;
            io::stringc Name;
            
            std::vector<STemplateMemberX> MemberList;
            std::vector<io::stringc> ReferenceList;
        };
        
        struct SRegisteredTemplateMemberX
        {
            SRegisteredTemplateMemberX(const EMemberTypes InitType = MEMBER_UNKNOWN);
            SRegisteredTemplateMemberX(const EMemberTypes InitType, s32 InitArraySize);
            SRegisteredTemplateMemberX(SRegisteredTemplateX* InitTemplateMember);
            SRegisteredTemplateMemberX(SRegisteredTemplateX* InitTemplateMember, s32 InitArraySize);
            ~SRegisteredTemplateMemberX();
            
            /* Members */
            EMemberTypes Type;
            SRegisteredTemplateMemberX* ArrayMember;
            SRegisteredTemplateX* TemplateMember;
            bool isArraySizeQualifier;
            
            // if > 0 specifies the count of elements in array
            // if < 0 specifies the element index (index = -1 - ArraySize) which specifies the real count of elements in array
            s32 ArraySize;
        };
        
        struct SRegisteredTemplateX
        {
            SRegisteredTemplateX();
            ~SRegisteredTemplateX();
            
            /* Functions */
            void addMember(const EMemberTypes Type);
            void addMember(const EMemberTypes Type, s32 ArraySize);
            void addMember(SRegisteredTemplateX* TemplateMember);
            void addMember(SRegisteredTemplateX* TemplateMember, s32 ArraySize);
            void addArraySizeQualifier(const EMemberTypes Type);
            
            /* Members */
            ETemplateTypes Type;
            io::stringc TypeName;
            bool acceptOtherMembers;
            
            std::vector<SRegisteredTemplateMemberX*> MemberList;
        };
        
        struct SMaterialX
        {
            video::color Diffuse, Specular, Emission;
            f32 Shininess;
            std::list<video::Texture*> TextureList;
        };
        
        /* === Functions === */
        
        void clear();
        
        bool readLineTxt();
        bool readLinePartTxt();
        void cropLineTxt(u32 CropEndPos);
        void cropLineTxt(u32 CropBeginPos, u32 CropEndPos);
        s32 findBreakTokenTxt() const;
        
        bool readHeader();
        bool readMesh();
        
        bool readNextTemplate(STemplateX* &Template);
        
        bool readNextTemplateTxt(STemplateX* &Template);
        bool readTemplateMembersTxt(STemplateX* Template, SRegisteredTemplateX* RegisteredTemplate);
        bool readNextMemberTxt(STemplateMemberX* CurMember, SRegisteredTemplateMemberX* RegisteredMember, bool isArrayElement);
        bool checkLineForTemplateReferenceTxt();
        bool checkLineForTemplateTxt();
        void checkForTemplateDeclarationTxt();
        
        bool readNextTemplateBin(STemplateX* Template);
        io::stringc readNextTokenBin() const;
        
        bool examineTemplateMember(STemplateMemberX* Member);
        bool examineTemplate(STemplateX* Template);
        
        void buildMesh();
        
        io::stringc getLinesIndicate() const;
        void printErrorLI(const io::stringc &Message) const;
        void printWarningLI(const io::stringc &Message) const;
        
        SRegisteredTemplateX* registerNewTemplate(const ETemplateTypes Type, const io::stringc &Name);
        void registerDefaultTemplates();
        
        video::color getTemplateMemberColor(const STemplateMemberX &Member) const;
        SMaterialX getTemplateMemberMaterial(const STemplateX* Template) const;
        bool getTemplateMemberAnimationKey(const STemplateMemberX &Member, u32 VecSize, f32* Vec, s32 &Time) const;
        
        #ifdef _DEBX_
        void _deb_PrintTemplateMember(STemplateMemberX &Member);
        void _deb_PrintTemplate(STemplateX* Template);
        #endif
        
        /* === Members === */
        
        io::stringc Line_, Trimed_;
        
        u32 CurLineNr_;
        
        bool isBinary_;
        bool isFloat64_;
        u32 FloatSize_;
        
        STemplateX* CurTemplate_;
        u32 LastArraySize_;
        
        // Container members
        std::vector<SRegisteredTemplateX*> RegisteredTemplateList_;
        std::map<std::string, SRegisteredTemplateX*> RegisteredTemplateMap_;
        
        std::map<std::string, SMaterialX> MaterialMap_;
        
        // Current creation members
        dim::matrix4f CurTransformation_;
        
        scene::SkeletalAnimation* BoneAnim_;
        scene::AnimationJoint* CurAnimBone_;
        
        std::map<std::string, scene::AnimationJoint*> AnimBoneMap_;
        
};


} // /namespace scene

} // /namespace sp


#endif

#endif



// ================================================================================
