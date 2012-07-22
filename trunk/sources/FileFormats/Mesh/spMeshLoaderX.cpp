/*
 * Mesh loader X file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "FileFormats/Mesh/spMeshLoaderX.hpp"

#ifdef SP_COMPILE_WITH_MESHLOADER_X


#include "Platform/spSoftPixelDeviceOS.hpp"


namespace sp
{

extern video::RenderSystem* __spVideoDriver;
extern scene::SceneGraph* __spSceneManager;

namespace scene
{


/*
 * Internal members
 */

const s32 X_MESH_MAGIC      = *((s32*)"xof ");
const s32 X_MESH_VERSION    = *((s32*)"0302");
const s32 X_MESH_BINARY     = *((s32*)"bin ");
const s32 X_MESH_TEXT       = *((s32*)"txt ");
const s32 X_MESH_FLOAT_32   = *((s32*)"0032");
const s32 X_MESH_FLOAT_64   = *((s32*)"0064");

const s32 X_MESH_BINARY_ZIP = *((s32*)"bzip");
const s32 X_MESH_TEXT_ZIP   = *((s32*)"tzip");


/*
 * MeshLoaderX class
 */

MeshLoaderX::MeshLoaderX()
    : MeshLoader(), CurTemplate_(0), BoneAnim_(0), CurAnimBone_(0)
{
    init();
}
MeshLoaderX::~MeshLoaderX()
{
    clear();
}

Mesh* MeshLoaderX::loadMesh(const io::stringc &Filename, const io::stringc &TexturePath)
{
    if (!openLoadFile(Filename, TexturePath))
        return Mesh_;
    
    CurLineNr_ = 0;
    FloatSize_ = 0;
    
    if (!readHeader() || !readMesh())
    {
        io::Log::error("Loading X mesh failed");
        FileSys_->closeFile(File_);
        return Mesh_;
    }
    
    FileSys_->closeFile(File_);
    
    buildMesh();
    
    return Mesh_;
}


/*
 * ======= Private: =======
 */

void MeshLoaderX::init()
{
    isBinary_       = false;
    LastArraySize_  = 0;
    CurLineNr_      = 0;
    
    registerDefaultTemplates();
}
void MeshLoaderX::clear()
{
    for (std::vector<SRegisteredTemplateX*>::iterator it = RegisteredTemplateList_.begin(); it != RegisteredTemplateList_.end(); ++it)
        MemoryManager::deleteMemory(*it);
}


bool MeshLoaderX::readLineTxt()
{
    s32 pos;
    
    do
    {
        // Read the next line in text file
        Line_ = File_->readString();
        ++CurLineNr_;
        
        // Remove comments
        if ( ( pos = Line_.find("#", 0) ) != -1 || ( pos = Line_.find("//", 0) ) != -1 )
            Line_ = Line_.left(pos);
        
        Trimed_ = Line_.trim();
    }
    while (!Trimed_.size() && !File_->isEOF());
    
    // Crop the space and tabulator characters at the front of the line
    if ( ( pos = Line_.findNotSpaces() ) != -1 )
        cropLineTxt(pos);
    
    return true;
}

bool MeshLoaderX::readLinePartTxt()
{
    return !Trimed_.size() ? readLineTxt() : true;
}

void MeshLoaderX::cropLineTxt(u32 CropEndPos)
{
    if (CropEndPos > 0)
    {
        Line_   = Line_.right(Line_.size() - CropEndPos);
        Trimed_ = Line_.trim();
    }
}

void MeshLoaderX::cropLineTxt(u32 CropBeginPos, u32 CropEndPos)
{
    if (CropBeginPos < CropEndPos && CropBeginPos > 0 && CropEndPos > 0)
    {
        Line_   = Line_.left(CropBeginPos) + Line_.right(Line_.size() - CropEndPos);
        Trimed_ = Line_.trim();
    }
}


bool MeshLoaderX::readHeader()
{
    // Check file size
    if (File_->getSize() < 12)
    {
        io::Log::error("X mesh file is too small");
        return false;
    }
    
    // Check magic number
    if (File_->readValue<s32>() != X_MESH_MAGIC)
    {
        io::Log::error("X mesh has invalid magic number");
        return false;
    }
    
    // Check version number
    if (File_->readValue<s32>() != X_MESH_VERSION)
    {
        io::Log::error("X mesh has invalid version (must be \"0302\")");
        return false;
    }
    
    // Check the format (binary or text)
    const s32 Format = File_->readValue<s32>();
    
    if (Format == X_MESH_BINARY)
        isBinary_ = true;
    else if (Format == X_MESH_TEXT)
        isBinary_ = false;
    else if (Format == X_MESH_BINARY_ZIP)
    {
        io::Log::error("X mesh binary compression is not supported");
        return false;
    }
    else if (Format == X_MESH_TEXT_ZIP)
    {
        io::Log::error("X mesh text compression is not supported");
        return false;
    }
    else
    {
        io::Log::error("X mesh has invalid format (must be \"bin \" or \"txt \")");
        return false;
    }
    
    const s32 FloatType = File_->readValue<s32>();
    
    if (FloatType == X_MESH_FLOAT_32)
    {
        isFloat64_ = false;
        FloatSize_ = 4;
    }
    else if (FloatType == X_MESH_FLOAT_64)
    {
        isFloat64_ = true;
        FloatSize_ = 8;
    }
    else
    {
        io::Log::error("X mesh has invalid float size (must be \"0032\" or \"0064\")");
        return false;
    }
    
    #if 1
    if (isBinary_)
    {
        io::Log::error("Binary X mesh files are not supported yet");
        return false;
    }
    #endif
    
    return true;
}

bool MeshLoaderX::readMesh()
{
    STemplateX* Template = 0;
    
    // Clear the line strings
    Line_   = "";
    Trimed_ = "";
    
    while (!File_->isEOF())
    {
        // Check if the next template is just a declaration
        checkForTemplateDeclarationTxt();
        
        // Read the next template
        Template = 0;
        
        if (!readNextTemplate(Template))
        {
            MemoryManager::deleteMemory(Template);
            return false;
        }
        
        // Examine the new template
        if (!examineTemplate(Template))
            return false;
        
        #if defined(_DEBX_) && 0
        _deb_PrintTemplate(Template);
        #endif
        
        MemoryManager::deleteMemory(Template);
        
        // Read the next line in file to find the end of file early enough
        readLinePartTxt();
    }
    
    return true;
}

bool MeshLoaderX::readNextTemplate(STemplateX* &Template)
{
    return isBinary_ ? readNextTemplateBin(Template) : readNextTemplateTxt(Template);
}


bool MeshLoaderX::readNextTemplateTxt(STemplateX* &Template)
{
    s32 pos1, pos2;
    io::stringc TemplateTypeName, TemplateName;
    
    bool haveTemplateName   = false;
    bool isTemplateBegin    = false;
    
    SRegisteredTemplateX* RegisteredTemplate = 0;
    
    // Read the file until the template has end
    while (!File_->isEOF())
    {
        // Read the next line in file
        readLinePartTxt();
        
        if (!haveTemplateName)
        {
            // Search the template's type name
            if ( ( pos1 = Line_.find(" ", 0) ) == -1 && ( pos1 = Line_.find("{", 0) ) == -1 )
                pos1 = Line_.size();
            
            if (!pos1)
            {
                printErrorLI("Template wanted but not found");
                return false;
            }
            
            TemplateTypeName    = Line_.left(pos1).trim();
            TemplateName        = "";
            
            // Search the template's name begin
            if ( ( pos1 = Line_.findNotSpaces(pos1 + 1) ) != -1 )
            {
                // Search the block begin
                if ( ( pos2 = Line_.find("{", pos1) ) != -1)
                    isTemplateBegin = true;
                else
                    pos2 = Line_.size() - 1;
                
                TemplateName = Line_.section(pos1, pos2).trim();
                
                if (isTemplateBegin)
                    cropLineTxt(pos2 + 1);
            }
            
            // Get the registered template
            RegisteredTemplate = RegisteredTemplateMap_[TemplateTypeName.lower().str()];
            
            if (!RegisteredTemplate)
            {
                printErrorLI("Unknown template type name: \"" + TemplateTypeName + "\"");
                return false;
            }
            
            #if defined(_DEBX_) && 0
            io::printMessage("TemplateTypeName: " + TemplateTypeName + (TemplateName != "" ? ", TemplateName: " + TemplateName : ""));
            #endif
            
            haveTemplateName = true;
            
            // Create a new template
            Template = new STemplateX();
            {
                Template->Type      = RegisteredTemplate->Type;
                Template->TypeName  = TemplateTypeName;
                Template->Name      = TemplateName;
            }
            CurTemplate_ = Template;
        }
        
        if (!isTemplateBegin)
        {
            // Search the block begin
            if ( ( pos1 = Line_.find("{", 0) ) != -1 )
            {
                isTemplateBegin = true;
                cropLineTxt(pos1 + 1);
            }
            else
            {
                printErrorLI("Template block begin wanted but not found");
                return false;
            }
        }
        else
        {
            // Search the template members
            if (!readTemplateMembersTxt(Template, RegisteredTemplate))
                return false;
            
            // Search for affluence member end characters
            pos1 = 0;
            while (Line_[pos1] == ';') ++pos1;
            
            if (pos1)
            {
                Line_ = Line_.right(Line_.size() - pos1);
                Trimed_ = Line_.trim();
            }
            
            // Search member templates
            if (RegisteredTemplate->acceptOtherMembers)
            {
                STemplateX* MemberTemplate;
                
                while (1)
                {
                    // Read the next line in file
                    readLinePartTxt();
                    
                    // Check if a template reference follows
                    if (checkLineForTemplateReferenceTxt())
                        continue;
                    
                    // Check if a new template follows
                    if (!checkLineForTemplateTxt())
                        break;
                    
                    MemberTemplate = 0;
                    
                    if (!readNextTemplateTxt(MemberTemplate))
                        return false;
                    
                    if (MemberTemplate)
                        Template->MemberList.push_back(STemplateMemberX(MemberTemplate));
                }
            }
            
            // Read the file until the block has end
            while (!File_->isEOF())
            {
                if ( ( pos1 = Line_.find("}", 0) ) != -1 )
                {
                    cropLineTxt(pos1 + 1);
                    return true;
                }
                
                readLineTxt();
            }
        }
    }
    
    return true;
}

bool MeshLoaderX::readTemplateMembersTxt(STemplateX* Template, SRegisteredTemplateX* RegisteredTemplate)
{
    if (!Template || !RegisteredTemplate)
    {
        printErrorLI("Invalid memory container while reading template");
        return false;
    }
    
    if (RegisteredTemplate->MemberList.empty())
        return true;
    
    #if defined(_DEBX_) && 0
    io::printMessage("Template (" + RegisteredTemplate->TypeName + ")");
    #endif
    
    STemplateMemberX* CurMember = 0;
    
    for (std::vector<SRegisteredTemplateMemberX*>::iterator it = RegisteredTemplate->MemberList.begin();
         it != RegisteredTemplate->MemberList.end(); ++it)
    {
        // Add a new member to the template member list
        Template->MemberList.resize(Template->MemberList.size() + 1);
        CurMember = &(Template->MemberList.back());
        
        // Read the next member
        if (!readNextMemberTxt(CurMember, *it, false))
            return false;
    }
    
    #if defined(_DEBX_) && 0
    io::printMessage("Template (/" + RegisteredTemplate->TypeName + ")");
    #endif
    
    return true;
}

bool MeshLoaderX::readNextMemberTxt(
    STemplateMemberX* CurMember, SRegisteredTemplateMemberX* RegisteredMember, bool isArrayElement)
{
    if (!CurMember || !RegisteredMember || !CurTemplate_)
    {
        printErrorLI("Invalid memory container while reading template member");
        return false;
    }
    
    s32 pos1, pos2;
    bool hasMemberValueFound = false;
    
    // Set the member's type
    CurMember->Type = RegisteredMember->Type;
    
    // Read the file until the end of the member has been found
    do
    {
        
        // Read the next line in file
        readLinePartTxt();
        
        pos1 = -1;
        
        // Determine which type the member has
        switch (CurMember->Type)
        {
            case MEMBER_TEMPLATE:
            {
                // Create a new template
                CurMember->Template = new STemplateX();
                CurMember->Template->Type = RegisteredMember->TemplateMember->Type;
                
                STemplateX* TmpTemplate = CurTemplate_;
                CurTemplate_ = CurMember->Template;
                
                // Read the member template
                if (!readTemplateMembersTxt(CurMember->Template, RegisteredMember->TemplateMember))
                    return false;
                
                CurTemplate_ = TmpTemplate;
                hasMemberValueFound = true;
            }
            break;
            
            case MEMBER_ARRAY:
            {
                s32 ArraySize = RegisteredMember->ArraySize;
                
                // Get the array size
                if (ArraySize < 0)
                    ArraySize = LastArraySize_;
                
                CurMember->ArrayList.resize(ArraySize);
                
                #if defined(_DEBX_) && 0
                io::printMessage("Array (Size = " + io::stringc(ArraySize) + ")");
                #endif
                
                // Read all array elements 
                for (std::vector<STemplateMemberX>::iterator itAry = CurMember->ArrayList.begin();
                     itAry != CurMember->ArrayList.end(); ++itAry)
                {
                    if (!readNextMemberTxt(&(*itAry), RegisteredMember->ArrayMember, true))
                        return false;
                }
                
                #if defined(_DEBX_) && 0
                io::printMessage("/Array");
                #endif
                
                hasMemberValueFound = true;
            }
            break;
            
            default:
            {
                // Search the member end character
                if ( ( pos1 = findBreakTokenTxt() ) == -1 )
                {
                    printErrorLI("Missing member end character: \";\" or \",\"");
                    return false;
                }
                
                hasMemberValueFound = true;
                
                io::stringc TmpStr = Line_.left(pos1);
                io::stringc TmpStrTimed = TmpStr.trim();
                
                switch (CurMember->Type)
                {
                    case MEMBER_DWORD:
                    case MEMBER_WORD:
                        CurMember->ValInteger = TmpStrTimed.val<s32>();
                        
                        if (RegisteredMember->isArraySizeQualifier)
                            LastArraySize_ = CurMember->ValInteger;
                        
                        #if defined(_DEBX_) && 0
                        io::printMessage(
                            "ValInteger: " + io::stringc(CurMember->ValInteger) +
                            (RegisteredMember->isArraySizeQualifier ? " (ArraySizeQualifier)" : "")
                        );
                        #endif
                        break;
                    
                    case MEMBER_FLOAT:
                    case MEMBER_DOUBLE:
                        CurMember->ValFloat = TmpStrTimed.val<f32>();
                        CurMember->ValDouble = TmpStrTimed.val<f64>();
                        #if defined(_DEBX_) && 0
                        io::printMessage("ValFloat: " + io::stringc(CurMember->ValFloat));
                        #endif
                        break;
                    
                    case MEMBER_STRING:
                    {
                        if ( ( pos1 = Line_.find("\"", 0) ) == -1 || ( pos2 = Line_.find("\"", pos1 + 1) ) == -1 )
                        {
                            printErrorLI("String wanted but not found");
                            return false;
                        }
                        
                        CurMember->ValString = Line_.section(pos1 + 1, pos2);
                    }
                    break;
                    
                    default:
                        hasMemberValueFound = false; break;
                } // /switch
            }
            break;
        } // /switch
        
    }
    while (!hasMemberValueFound);
    
    // Crop the line until the member end character
    if ( ( pos1 = findBreakTokenTxt() ) == -1)
    {
        printErrorLI("Missing member end character: \";\" or \",\"");
        return false;
    }
    
    // Check if the member end character needs also to be croped
    if (isArrayElement)
        cropLineTxt(Line_[pos1] == ';' ? pos1 : pos1 + 1);
    else
        cropLineTxt(pos1 + 1);
    
    return true;
}

s32 MeshLoaderX::findBreakTokenTxt() const
{
    s32 pos1 = Line_.find(";", 0);
    s32 pos2 = Line_.find(",", 0);
    
    if ( pos1 == -1 || ( pos2 != -1 && pos2 < pos1 ) )
        pos1 = pos2;
    
    return pos1;
}

bool MeshLoaderX::checkLineForTemplateReferenceTxt()
{
    s32 pos1, pos2;
    
    // Check if the line specifies template reference
    if ( ( pos1 = Line_.find("{", 0) ) != -1 && ( pos2 = Line_.find("}", pos1 + 1) ) != -1 && !Line_.left(pos1).trim().size() )
    {
        // Get the reference name
        const io::stringc ReferenceName = Line_.section(pos1 + 1, pos2).trim();
        
        #if defined(_DEBX_) && 0
        io::getMessage(ReferenceName);
        #endif
        
        // Add the reference name to the current template
        CurTemplate_->ReferenceList.push_back(ReferenceName);
        
        cropLineTxt(pos2 + 1);
        
        return true;
    }
    
    return false;
}

bool MeshLoaderX::checkLineForTemplateTxt()
{
    s32 pos;
    
    // Search the template's type name
    if ( ( pos = Line_.find(" ", 0) ) == -1 && ( pos = Line_.find("{", 0) ) == -1 )
        pos = Line_.size();
    
    const io::stringc TemplateTypeName = Line_.left(pos).trim();
    
    if (!TemplateTypeName.size())
        return false;
    
    // Return true if the template type name is valid
    return RegisteredTemplateMap_[TemplateTypeName.lower().str()] != 0;
}

void MeshLoaderX::checkForTemplateDeclarationTxt()
{
    s32 pos;
    
    // Read the file until each template declarations has been ignored
    while (!File_->isEOF())
    {
        // Determine if a template declaration has been detected
        if (Line_.left(8).trim() == "template" && Line_.find("{") != -1)
        {
            // Read the file until the template declaration has been end
            while (!File_->isEOF())
            {
                if ( ( pos = Line_.find("}") ) != -1 )
                {
                    Line_ = Trimed_ = "";
                    break;
                }
                
                // Read the next line
                readLineTxt();
            }
        }
        else
            break;
        
        // Read the next line
        readLineTxt();
    }
}


bool MeshLoaderX::readNextTemplateBin(STemplateX* Template)
{
    // !TODO!
    return true;
}

io::stringc MeshLoaderX::readNextTokenBin() const
{
    io::stringc Str;
    
    const s16 Token = File_->readValue<s16>();
    u32 Len;
    
    switch (Token)
    {
        case 0x01: // Name token
            return File_->readStringData();
        case 0x02: // String token
            Str = File_->readStringData();
            File_->readValue<s16>();
            return Str;
        case 0x03: // Integer token
            File_->setSeek(4, io::FILEPOS_CURRENT);
            return "<int>";
        case 0x05: // GUID token
            File_->setSeek(16, io::FILEPOS_CURRENT);
            return "<guid>";
        case 0x06: // Integer list token
            Len = File_->readValue<u32>();
            File_->setSeek(Len*4, io::FILEPOS_CURRENT);
            return "<int_list>";
        case 0x07: // Float list token
            Len = File_->readValue<u32>();
            File_->setSeek(Len*FloatSize_, io::FILEPOS_CURRENT);
            return "<flt_list>";
        case 0x0A:
            return "{";
        case 0x0B:
            return "}";
        case 0x0C:
            return "(";
        case 0x0D:
            return ")";
        case 0x0E:
            return "[";
        case 0x0F:
            return "]";
        case 0x10:
            return "<";
        case 0x11:
            return ">";
        case 0x12:
            return ".";
        case 0x13:
            return ",";
        case 0x14:
            return ";";
        case 0x1F:
            return "template";
        case 0x28:
            return "workd";
        case 0x29:
            return "dword";
        case 0x2A:
            return "float";
        case 0x2B:
            return "double";
        case 0x2C:
            return "char";
        case 0x2D:
            return "uchar";
        case 0x2E:
            return "sword";
        case 0x2F:
            return "sdword";
        case 0x30:
            return "void";
        case 0x31:
            return "string";
        case 0x32:
            return "unicode";
        case 0x33:
            return "cstring";
        case 0x34:
            return "array";
    }
    
    return "";
}


bool MeshLoaderX::examineTemplateMember(STemplateMemberX* Member)
{
    if (!Member)
    {
        io::Log::error("Invalid template member memory address");
        return false;
    }
    
    switch (Member->Type)
    {
        case MEMBER_ARRAY:
            for (std::vector<STemplateMemberX>::iterator it = Member->ArrayList.begin(); it != Member->ArrayList.end(); ++it)
            {
                if (!examineTemplateMember(&(*it)))
                    return false;
            }
            break;
            
        case MEMBER_TEMPLATE:
            if (!examineTemplate(Member->Template))
                return false;
            break;
            
        default:
            break;
    }
    
    return true;
}

bool MeshLoaderX::examineTemplate(STemplateX* Template)
{
    if (!Template)
    {
        io::Log::error("Invalid template memory address");
        return false;
    }
    
    switch (Template->Type)
    {
        case TEMPLATE_FRAME:
        {
            if (Template->Name == "")
            {
                io::Log::error("Missing bone name in \"Frame\" template");
                return false;
            }
            
            scene::AnimationBone* LastAnimBone = CurAnimBone_;
            
            // Create the animation if not still exist
            if (!Anim_)
                Anim_ = BoneAnim_ = (scene::AnimationSkeletal*)Mesh_->addAnimation(scene::ANIMATION_SKELETAL);
            
            // Add a new bone to the skeletal animation
            const io::stringc BoneName = Template->Name;
            
            CurAnimBone_ = BoneAnim_->addBone(LastAnimBone, 0.0f, dim::quaternion(), 1.0f, BoneName);
            
            AnimBoneMap_[BoneName.str()] = CurAnimBone_;
            
            // Additional templates
            for (std::vector<STemplateMemberX>::iterator it = Template->MemberList.begin(); it != Template->MemberList.end(); ++it)
            {
                if (!examineTemplateMember(&(*it)))
                    return false;
            }
            
            CurAnimBone_ = LastAnimBone;
        }
        break;
        
        case TEMPLATE_ANIMATION:
        {
            if (Template->ReferenceList.empty())
            {
                io::Log::error("Missing bone name in \"Animation\" template");
                return false;
            }
            
            // Get the bone
            const io::stringc BoneName = Template->ReferenceList[0];
            
            CurAnimBone_ = AnimBoneMap_[BoneName.str()];
            
            if (!CurAnimBone_)
            {
                io::Log::error("Bone \"" + BoneName + "\" does not exist");
                return false;
            }
            
            // Additional templates
            for (std::vector<STemplateMemberX>::iterator it = Template->MemberList.begin(); it != Template->MemberList.end(); ++it)
            {
                if (!examineTemplateMember(&(*it)))
                    return false;
            }
        }
        break;
        
        case TEMPLATE_ANIMATIONKEY:
        {
            if (!CurAnimBone_)
            {
                io::Log::error("No current bone set for animation key");
                return false;
            }
            
            if (Template->MemberList.size() != 3)
            {
                io::Log::error("Invalid count of elements in member list of \"AnimationKey\" template");
                return false;
            }
            
            const s32 KeyType   = Template->MemberList[0].ValInteger;
            const u32 KeyCount  = Template->MemberList[2].ArrayList.size();
            
            s32 Time;
            f32 Vec[4];
            u32 i = 0;
            
            if (CurAnimBone_->getKeyframeList().size() < KeyCount)
                CurAnimBone_->getKeyframeList().resize(KeyCount);
            
            // Add each animation key
            for (std::vector<STemplateMemberX>::iterator it = Template->MemberList[2].ArrayList.begin();
                 it != Template->MemberList[2].ArrayList.end(); ++it, ++i)
            {
                if (KeyType == ANIMKEY_POSITION)
                {
                    if (getTemplateMemberAnimationKey(*it, 3, Vec, Time))
                        CurAnimBone_->getKeyframeList()[i].Translation = dim::vector3df(Vec[0], Vec[1], Vec[2]);
                }
                else if (KeyType == ANIMKEY_ROTATION)
                {
                    if (getTemplateMemberAnimationKey(*it, 4, Vec, Time))
                        CurAnimBone_->getKeyframeList()[i].Rotation = dim::quaternion(-Vec[1], -Vec[2], -Vec[3], -Vec[0]);
                }
                else if (KeyType == ANIMKEY_SCALE)
                {
                    if (getTemplateMemberAnimationKey(*it, 3, Vec, Time))
                        CurAnimBone_->getKeyframeList()[i].Scale = dim::vector3df(Vec[0], Vec[1], Vec[2]);
                }
            }
            
            if (!CurAnimBone_->getKeyframeList().empty())
            {
                if (KeyType == ANIMKEY_POSITION)
                    CurAnimBone_->setTranslation(CurAnimBone_->getKeyframeList()[0].Translation);
                else if (KeyType == ANIMKEY_ROTATION)
                    CurAnimBone_->setRotation(CurAnimBone_->getKeyframeList()[0].Rotation);
                else if (KeyType == ANIMKEY_SCALE)
                    CurAnimBone_->setScale(CurAnimBone_->getKeyframeList()[0].Scale);
            }
        }
        break;
        
        case TEMPLATE_MESH:
        {
            if (Template->MemberList.size() < 4)
            {
                io::Log::error("Too few elements in member list of \"Mesh\" template");
                return false;
            }
            
            dim::vector3df Vec;
            u32 IndicesCount;
            u32 FaceIndices[4];
            
            // Create a new surface
            Surface_ = Mesh_->createMeshBuffer(SceneGraph::getDefaultVertexFormat(), SceneGraph::getDefaultIndexFormat());
            
            // Add all vertices
            for (std::vector<STemplateMemberX>::iterator it = Template->MemberList[1].ArrayList.begin();
                 it != Template->MemberList[1].ArrayList.end(); ++it)
            {
                if (!it->Template || it->Template->Type != TEMPLATE_VECTOR || it->Template->MemberList.size() != 3)
                {
                    io::Log::error("\"Vector\" template wanted but not available in \"Mesh\" template");
                    return false;
                }
                
                Vec.X = it->Template->MemberList[0].ValFloat;
                Vec.Y = it->Template->MemberList[1].ValFloat;
                Vec.Z = it->Template->MemberList[2].ValFloat;
                
                Surface_->addVertex(CurTransformation_ * Vec);
            }
            
            // Add all triangles
            for (std::vector<STemplateMemberX>::iterator it = Template->MemberList[3].ArrayList.begin();
                 it != Template->MemberList[3].ArrayList.end(); ++it)
            {
                if (!it->Template || it->Template->Type != TEMPLATE_MESHFACE || it->Template->MemberList.size() != 2)
                {
                    io::Log::error("\"MeshFace\" template wanted but not available in \"Mesh\" template");
                    return false;
                }
                
                IndicesCount = it->Template->MemberList[1].ArrayList.size();
                
                if (IndicesCount == 3)
                {
                    FaceIndices[0] = it->Template->MemberList[1].ArrayList[0].ValInteger;
                    FaceIndices[1] = it->Template->MemberList[1].ArrayList[1].ValInteger;
                    FaceIndices[2] = it->Template->MemberList[1].ArrayList[2].ValInteger;
                    
                    Surface_->addTriangle(FaceIndices[0], FaceIndices[1], FaceIndices[2]);
                }
                else if (IndicesCount == 4)
                {
                    FaceIndices[0] = it->Template->MemberList[1].ArrayList[0].ValInteger;
                    FaceIndices[1] = it->Template->MemberList[1].ArrayList[1].ValInteger;
                    FaceIndices[2] = it->Template->MemberList[1].ArrayList[2].ValInteger;
                    FaceIndices[3] = it->Template->MemberList[1].ArrayList[3].ValInteger;
                    
                    Surface_->addTriangle(FaceIndices[0], FaceIndices[1], FaceIndices[2]);
                    Surface_->addTriangle(FaceIndices[0], FaceIndices[2], FaceIndices[3]);
                }
                else
                {
                    io::Log::error(
                        "\"MeshFace\" template needs to have 3 or 4 indices but has " + io::stringc(IndicesCount)
                    );
                    return false;
                }
            }
            
            /*
            // Add vertex weights
            if (CurAnimBone_)
            {
                const u32 VertexCount = Mesh_->getVertexCount(Surface);
                
                std::vector<SVertexWeight> VertexWeights;
                VertexWeights.resize(VertexCount);
                
                for (u32 i = 0; i < VertexCount; ++i)
                {
                    VertexWeights[i].Surface    = Surface;
                    VertexWeights[i].Index      = i;
                    VertexWeights[i].Position   = Mesh_->getVertexCoord(i);
                    VertexWeights[i].Normal     = Mesh_->getVertexNormal(i);
                }
                
                CurAnimBone_->setVertexWeights(VertexWeights);
            }
            */
            
            // Additional templates
            for (std::vector<STemplateMemberX>::iterator it = Template->MemberList.begin() + 4; it != Template->MemberList.end(); ++it)
            {
                if (!examineTemplateMember(&(*it)))
                    return false;
            }
            
            // Update the normals if the mesh normals are not defined
            Surface_->updateNormals(Mesh_->getMaterial()->getShading());
        }
        break;
        
        case TEMPLATE_MESHTEXTURECOORDS:
        {
            if (Template->MemberList.size() < 2)
            {
                io::Log::error("Too few elements in member list of \"MeshTextureCoords\" template");
                return false;
            }
            
            u32 i = 0;
            dim::point2df Vec;
            
            // Set all vertex texture coordinates
            for (std::vector<STemplateMemberX>::iterator it = Template->MemberList[1].ArrayList.begin();
                 it != Template->MemberList[1].ArrayList.end(); ++it, ++i)
            {
                if (!it->Template || it->Template->Type != TEMPLATE_COORDS2D || it->Template->MemberList.size() != 2)
                {
                    io::Log::error("\"Coords2D\" template wanted but not available in \"MeshTextureCoords\" template");
                    return false;
                }
                
                Vec.X = -it->Template->MemberList[0].ValFloat;
                Vec.Y = it->Template->MemberList[1].ValFloat;
                
                Surface_->setVertexTexCoord(i, Vec);
            }
        }
        break;
        
        case TEMPLATE_MATERIAL:
        {
            if (Template->Name != "")
                MaterialMap_[Template->Name.str()] = getTemplateMemberMaterial(Template);
        }
        break;
        
        case TEMPLATE_MESHMATERIALLIST:
        {
            SMaterialX Material;
            
            // Get the material data
            if (!Template->ReferenceList.empty())
                Material = MaterialMap_[Template->ReferenceList[0].str()];
            else if (Template->MemberList.size() > 3 && Template->MemberList[3].Template)
                Material = getTemplateMemberMaterial(Template->MemberList[3].Template);
            
            // Apply the material to the mesh
            Surface_->paint(Material.Diffuse, true);
            
            for (std::list<video::Texture*>::iterator it = Material.TextureList.begin(); it != Material.TextureList.end(); ++it)
                Surface_->addTexture(*it);
        }
        break;
        
        case TEMPLATE_FRAMETRANSFORMMATRIX:
        {
            if (Template->MemberList.size() != 1 || !Template->MemberList[0].Template ||
                Template->MemberList[0].Template->Type != TEMPLATE_MATRIX4X4 ||
                Template->MemberList[0].Template->MemberList.size() != 1 ||
                Template->MemberList[0].Template->MemberList[0].ArrayList.size() != 16)
            {
                io::Log::error("\"FrameTransformMatrix\" template is corrupt");
                return false;
            }
            
            for (s32 i = 0; i < 16; ++i)
                CurTransformation_[i] = Template->MemberList[0].Template->MemberList[0].ArrayList[i].ValFloat;
        }
        break;
        
        default:
        {
            for (std::vector<STemplateMemberX>::iterator it = Template->MemberList.begin(); it != Template->MemberList.end(); ++it)
            {
                if (!examineTemplateMember(&(*it)))
                    return false;
            }
        }
        break;
    }
    
    return true;
}


void MeshLoaderX::buildMesh()
{
    Mesh_->updateMeshBuffer();
    
    if (BoneAnim_)
        BoneAnim_->updateSkeleton();
}


io::stringc MeshLoaderX::getLinesIndicate() const
{
    return " (Line #" + io::stringc(CurLineNr_) + ")";
}

void MeshLoaderX::printErrorLI(const io::stringc &Message) const
{
    if (isBinary_)
        io::Log::error(Message);
    else
        io::Log::error(Message + getLinesIndicate());
}

void MeshLoaderX::printWarningLI(const io::stringc &Message) const
{
    if (isBinary_)
        io::Log::warning(Message);
    else
        io::Log::warning(Message + getLinesIndicate());
}


MeshLoaderX::SRegisteredTemplateX* MeshLoaderX::registerNewTemplate(const ETemplateTypes Type, const io::stringc &Name)
{
    const io::stringc LCaseName = Name.lower();
    
    SRegisteredTemplateX* Template = new SRegisteredTemplateX();
    {
        Template->Type      = Type;
        Template->TypeName  = LCaseName;
    }
    RegisteredTemplateList_.push_back(Template);
    
    RegisteredTemplateMap_[LCaseName.str()] = Template;
    
    return Template;
}

void MeshLoaderX::registerDefaultTemplates()
{
    /* === Temporary template pointers === */
    
    SRegisteredTemplateX* Template                  = 0;
    SRegisteredTemplateX* TemplateVector            = 0;
    SRegisteredTemplateX* TemplateColorRGBA         = 0;
    SRegisteredTemplateX* TemplateColorRGB          = 0;
    SRegisteredTemplateX* TemplateIndexedColor      = 0;
    SRegisteredTemplateX* TemplateBoolean           = 0;
    SRegisteredTemplateX* TemplateBoolean2d         = 0;
    SRegisteredTemplateX* TemplateCoords2d          = 0;
    SRegisteredTemplateX* TemplateMatrix4x4         = 0;
    SRegisteredTemplateX* TemplateMeshFace          = 0;
    SRegisteredTemplateX* TemplateMeshFaceWraps     = 0;
    SRegisteredTemplateX* TemplateMeshTextureCoords = 0;
    SRegisteredTemplateX* TemplateMeshNormals       = 0;
    SRegisteredTemplateX* TemplateMeshVertexColors  = 0;
    SRegisteredTemplateX* TemplateMeshMaterialList  = 0;
    SRegisteredTemplateX* TemplateFloatKeys         = 0;
    SRegisteredTemplateX* TemplateTimedFloatKeys    = 0;
    
    /* === Register each default template === */
    
    Template = registerNewTemplate(TEMPLATE_HEADER, "Header");
    Template->addMember(MEMBER_WORD);   // major
    Template->addMember(MEMBER_WORD);   // minor
    Template->addMember(MEMBER_DWORD);  // flags
    
    Template = registerNewTemplate(TEMPLATE_VECTOR, "Vector");
    Template->addMember(MEMBER_FLOAT);  // x
    Template->addMember(MEMBER_FLOAT);  // y
    Template->addMember(MEMBER_FLOAT);  // z
    TemplateVector = Template;
    
    Template = registerNewTemplate(TEMPLATE_COORDS2D, "Coords2d");
    Template->addMember(MEMBER_FLOAT);  // u
    Template->addMember(MEMBER_FLOAT);  // v
    TemplateCoords2d = Template;
    
    Template = registerNewTemplate(TEMPLATE_QUATERNION, "Quaternion");
    Template->addMember(MEMBER_FLOAT);      // s
    Template->addMember(TemplateVector);    // v
    
    Template = registerNewTemplate(TEMPLATE_MATRIX4X4, "Matrix4x4");
    Template->addMember(MEMBER_FLOAT, 16);  // matrix
    TemplateMatrix4x4 = Template;
    
    Template = registerNewTemplate(TEMPLATE_COLORRGBA, "ColorRGBA");
    Template->addMember(MEMBER_FLOAT);  // red
    Template->addMember(MEMBER_FLOAT);  // green
    Template->addMember(MEMBER_FLOAT);  // blue
    Template->addMember(MEMBER_FLOAT);  // alpha
    TemplateColorRGBA = Template;
    
    Template = registerNewTemplate(TEMPLATE_COLORRGB, "ColorRGB");
    Template->addMember(MEMBER_FLOAT);  // red
    Template->addMember(MEMBER_FLOAT);  // green
    Template->addMember(MEMBER_FLOAT);  // blue
    TemplateColorRGB = Template;
    
    Template = registerNewTemplate(TEMPLATE_INDEXEDCOLOR, "Indexed Color");
    Template->addMember(MEMBER_DWORD);      // index
    Template->addMember(TemplateColorRGBA); // indexColor
    TemplateIndexedColor = Template;
    
    Template = registerNewTemplate(TEMPLATE_BOOLEAN, "Boolean");
    Template->addMember(MEMBER_DWORD);  // truefalse
    TemplateBoolean = Template;
    
    Template = registerNewTemplate(TEMPLATE_BOOLEAN2D, "Boolean2d");
    Template->addMember(TemplateBoolean);   // u
    Template->addMember(TemplateBoolean);   // v
    TemplateBoolean2d = Template;
    
    Template = registerNewTemplate(TEMPLATE_MATERIAL, "Material");
    Template->addMember(TemplateColorRGBA); // faceColor
    Template->addMember(MEMBER_FLOAT);      // power
    Template->addMember(TemplateColorRGB);  // specularColor
    Template->addMember(TemplateColorRGB);  // emissiveColor
    Template->acceptOtherMembers = true;
    
    Template = registerNewTemplate(TEMPLATE_TEXTUREFILENAME, "TextureFilename");
    Template->addMember(MEMBER_STRING); // faceColor
    
    Template = registerNewTemplate(TEMPLATE_MESHFACE, "MeshFace");
    Template->addArraySizeQualifier(MEMBER_DWORD);  // nFaceVertexIndices
    Template->addMember(MEMBER_DWORD, -1);          // faceVertexIndices
    TemplateMeshFace = Template;
    
    Template = registerNewTemplate(TEMPLATE_MESHFACE, "MeshFaceWraps");
    Template->addArraySizeQualifier(MEMBER_DWORD);  // nFaceWrapValues
    Template->addMember(TemplateBoolean2d, -1);     // faceWrapValues
    TemplateMeshFaceWraps = Template;
    
    Template = registerNewTemplate(TEMPLATE_MESHTEXTURECOORDS, "MeshTextureCoords");
    Template->addArraySizeQualifier(MEMBER_DWORD);  // nTextureCoords
    Template->addMember(TemplateCoords2d, -1);      // textureCoords
    TemplateMeshTextureCoords = Template;
    
    Template = registerNewTemplate(TEMPLATE_MESHNORMALS, "MeshNormals");
    Template->addArraySizeQualifier(MEMBER_DWORD);  // nNormals
    Template->addMember(TemplateVector, -1);        // normal
    Template->addArraySizeQualifier(MEMBER_DWORD);  // nFaceNormals
    Template->addMember(TemplateMeshFace, -1);      // faceNormals
    TemplateMeshNormals = Template;
    
    Template = registerNewTemplate(TEMPLATE_MESHVERTEXCOLORS, "MeshVertexColors");
    Template->addArraySizeQualifier(MEMBER_DWORD);  // nVertexColors
    Template->addMember(TemplateIndexedColor, -1);  // vertexColors
    TemplateMeshVertexColors = Template;
    
    Template = registerNewTemplate(TEMPLATE_MESHMATERIALLIST, "MeshMaterialList");
    Template->addArraySizeQualifier(MEMBER_DWORD);  // nMaterials
    Template->addArraySizeQualifier(MEMBER_DWORD);  // nFaceIndices
    Template->addMember(MEMBER_DWORD, -1);          // FaceIndices
    Template->acceptOtherMembers = true;
    TemplateMeshMaterialList = Template;
    
    Template = registerNewTemplate(TEMPLATE_MESH, "Mesh");
    Template->addArraySizeQualifier(MEMBER_DWORD);  // nVertices
    Template->addMember(TemplateVector, -1);        // vertices
    Template->addArraySizeQualifier(MEMBER_DWORD);  // nFaces
    Template->addMember(TemplateMeshFace, -1);      // faces
    Template->acceptOtherMembers = true;
    
    Template = registerNewTemplate(TEMPLATE_FRAMETRANSFORMMATRIX, "FrameTransformMatrix");
    Template->addMember(TemplateMatrix4x4); // frameMatrix
    
    Template = registerNewTemplate(TEMPLATE_FRAME, "Frame");
    Template->acceptOtherMembers = true;
    
    Template = registerNewTemplate(TEMPLATE_FLOATKEYS, "FloatKeys");
    Template->addArraySizeQualifier(MEMBER_DWORD);  // nValues
    Template->addMember(MEMBER_FLOAT, -1);          // values
    TemplateFloatKeys = Template;
    
    Template = registerNewTemplate(TEMPLATE_TIMEDFLOATKEYS, "TimedFloatKeys");
    Template->addMember(MEMBER_DWORD);      // time
    Template->addMember(TemplateFloatKeys); // tfkeys
    TemplateTimedFloatKeys = Template;
    
    Template = registerNewTemplate(TEMPLATE_ANIMATIONKEY, "AnimationKey");
    Template->addMember(MEMBER_DWORD);                  // keyType
    Template->addArraySizeQualifier(MEMBER_DWORD);      // nKeys
    Template->addMember(TemplateTimedFloatKeys, -1);    // keys
    
    Template = registerNewTemplate(TEMPLATE_ANIMATIONOPTIONS, "AnimationOptions");
    Template->addMember(MEMBER_DWORD);  // openclosed
    Template->addMember(MEMBER_DWORD);  // position quality
    
    Template = registerNewTemplate(TEMPLATE_ANIMATION, "Animation");
    Template->acceptOtherMembers = true;
    
    Template = registerNewTemplate(TEMPLATE_ANIMATIONSET, "AnimationSet");
    Template->acceptOtherMembers = true;
}


video::color MeshLoaderX::getTemplateMemberColor(const STemplateMemberX &Member) const
{
    video::color Clr;
    
    if ( Member.Template && ( Member.Template->Type == TEMPLATE_COLORRGB || Member.Template->Type == TEMPLATE_COLORRGBA ) )
    {
        if (Member.Template->MemberList.size() >= 3)
        {
            Clr.Red     = (u8)(Member.Template->MemberList[0].ValFloat * 255);
            Clr.Green   = (u8)(Member.Template->MemberList[1].ValFloat * 255);
            Clr.Blue    = (u8)(Member.Template->MemberList[2].ValFloat * 255);
        }
        if (Member.Template->MemberList.size() == 4)
            Clr.Alpha   = (u8)(Member.Template->MemberList[3].ValFloat * 255);
    }
    
    return Clr;
}

MeshLoaderX::SMaterialX MeshLoaderX::getTemplateMemberMaterial(const STemplateX* Template) const
{
    SMaterialX Material;
    
    if (Template->Type == TEMPLATE_MATERIAL && Template->MemberList.size() >= 4)
    {
        // Get the basic members
        Material.Diffuse    = getTemplateMemberColor(Template->MemberList[0]);
        Material.Specular   = getTemplateMemberColor(Template->MemberList[1]);
        Material.Emission   = getTemplateMemberColor(Template->MemberList[2]);
        Material.Shininess  = Template->MemberList[3].ValFloat;
        
        // Get additional members
        if (SceneGraph::getTextureLoadingState())
        {
            for (std::vector<STemplateMemberX>::const_iterator it = Template->MemberList.begin() + 4; it != Template->MemberList.end(); ++it)
            {
                if (it->Template && it->Template->Type == TEMPLATE_TEXTUREFILENAME && it->Template->MemberList.size() >= 1)
                {
                    Material.TextureList.push_back(
                        __spVideoDriver->loadTexture(TexturePath_ + it->Template->MemberList[0].ValString)
                    );
                }
            }
        }
    }
    
    return Material;
}

bool MeshLoaderX::getTemplateMemberAnimationKey(const STemplateMemberX &Member, u32 VecSize, f32* Vec, s32 &Time) const
{
    if (Member.Template && Member.Template->Type == TEMPLATE_TIMEDFLOATKEYS && Member.Template->MemberList.size() == 2)
    {
        Time = Member.Template->MemberList[0].ValInteger;
        
        STemplateMemberX MemFloatKeys = Member.Template->MemberList[1];
        
        if (MemFloatKeys.Template && MemFloatKeys.Template->Type == TEMPLATE_FLOATKEYS && MemFloatKeys.Template->MemberList.size() == 2)
        {
            const u32 ValCount = MemFloatKeys.Template->MemberList[1].ArrayList.size();
            
            if (ValCount == VecSize)
            {
                for (u32 i = 0; i < VecSize; ++i)
                    Vec[i] = MemFloatKeys.Template->MemberList[1].ArrayList[i].ValFloat;
                return true;
            }
        }
    }
    
    return false;
}


#if defined(_DEBX_) && 1

void MeshLoaderX::_deb_PrintTemplateMember(STemplateMemberX &Member)
{
    switch (Member.Type)
    {
        case MEMBER_UNKNOWN:
            io::printMessage("Unknown"); break;
        
        case MEMBER_ARRAY:
        {
            io::printMessage("Array {");
            
            io::upperTab();
            for (std::vector<STemplateMemberX>::iterator it = Member.ArrayList.begin(); it != Member.ArrayList.end(); ++it)
                _deb_PrintTemplateMember(*it);
            io::lowerTab();
            
            io::printMessage("} // /Array");
        }
        break;
        
        case MEMBER_TEMPLATE:
            _deb_PrintTemplate(Member.Template); break;
        
        case MEMBER_UCHAR:
        case MEMBER_WORD:
        case MEMBER_DWORD:
        case MEMBER_SWORD:
        case MEMBER_SDWORD:
        case MEMBER_ULONGLONG:
            io::printMessage("Int: " + io::stringc(Member.ValInteger)); break;
        
        case MEMBER_FLOAT:
            io::printMessage("Flt: " + io::stringc(Member.ValFloat)); break;
        
        case MEMBER_DOUBLE:
            io::printMessage("Dbl: " + io::stringc(Member.ValDouble)); break;
        
        case MEMBER_STRING:
            io::printMessage("Str: " + Member.ValString); break;
    }
}

void MeshLoaderX::_deb_PrintTemplate(STemplateX* Template)
{
    if (!Template)
        return;
    
    io::printMessage("TEMPLATE \"" + Template->TypeName + "\": \"" + Template->Name + "\" {");
    
    io::upperTab();
    for (std::vector<STemplateMemberX>::iterator it = Template->MemberList.begin(); it != Template->MemberList.end(); ++it)
        _deb_PrintTemplateMember(*it);
    io::lowerTab();
    
    io::printMessage("} // /TEMPLATE");
}

#endif


/*
 * STemplateMemberX structure
 */

MeshLoaderX::STemplateMemberX::STemplateMemberX(const EMemberTypes InitType)
    : Type(InitType), Template(0)
{
    ValInteger  = 0;
    ValFloat    = 0;
    ValDouble   = 0;
}
MeshLoaderX::STemplateMemberX::STemplateMemberX(STemplateX* InitTemplate)
    : Type(MEMBER_TEMPLATE), Template(InitTemplate)
{
    ValInteger  = 0;
    ValFloat    = 0;
    ValDouble   = 0;
}
MeshLoaderX::STemplateMemberX::~STemplateMemberX()
{
}


/*
 * STemplateX structure
 */

MeshLoaderX::STemplateX::STemplateX() : Type(TEMPLATE_UNKNOWN)
{
}
MeshLoaderX::STemplateX::~STemplateX()
{
    for (std::vector<STemplateMemberX>::iterator it = MemberList.begin(); it != MemberList.end(); ++it)
        MemoryManager::deleteMemory(it->Template);
}


/*
 * SRegisteredTemplateMemberX structure
 */

MeshLoaderX::SRegisteredTemplateMemberX::SRegisteredTemplateMemberX(
    const EMemberTypes InitType)
    : Type(InitType), ArrayMember(0), TemplateMember(0),
    isArraySizeQualifier(false), ArraySize(0)
{
}
MeshLoaderX::SRegisteredTemplateMemberX::SRegisteredTemplateMemberX(
    const EMemberTypes InitType, s32 InitArraySize)
    : Type(MEMBER_ARRAY), ArrayMember(new SRegisteredTemplateMemberX(InitType)),
    TemplateMember(0), isArraySizeQualifier(false), ArraySize(InitArraySize)
{
}
MeshLoaderX::SRegisteredTemplateMemberX::SRegisteredTemplateMemberX(
    SRegisteredTemplateX* InitTemplateMember)
    : Type(MEMBER_TEMPLATE), ArrayMember(0), TemplateMember(InitTemplateMember),
    isArraySizeQualifier(false), ArraySize(0)
{
}
MeshLoaderX::SRegisteredTemplateMemberX::SRegisteredTemplateMemberX(
    SRegisteredTemplateX* InitTemplateMember, s32 InitArraySize)
    : Type(MEMBER_ARRAY), ArrayMember(new SRegisteredTemplateMemberX(InitTemplateMember)),
    TemplateMember(0), isArraySizeQualifier(false), ArraySize(InitArraySize)
{
}
MeshLoaderX::SRegisteredTemplateMemberX::~SRegisteredTemplateMemberX()
{
    MemoryManager::deleteMemory(ArrayMember);
}


/*
 * SRegisteredTemplateX structure
 */

MeshLoaderX::SRegisteredTemplateX::SRegisteredTemplateX()
    : Type(TEMPLATE_UNKNOWN), acceptOtherMembers(false)
{
}
MeshLoaderX::SRegisteredTemplateX::~SRegisteredTemplateX()
{
    for (std::vector<SRegisteredTemplateMemberX*>::iterator it = MemberList.begin(); it != MemberList.end(); ++it)
        MemoryManager::deleteMemory(*it);
}

void MeshLoaderX::SRegisteredTemplateX::addMember(const EMemberTypes Type)
{
    MemberList.push_back(new SRegisteredTemplateMemberX(Type));
}
void MeshLoaderX::SRegisteredTemplateX::addMember(const EMemberTypes Type, s32 ArraySize)
{
    MemberList.push_back(new SRegisteredTemplateMemberX(Type, ArraySize));
}
void MeshLoaderX::SRegisteredTemplateX::addMember(SRegisteredTemplateX* TemplateMember)
{
    MemberList.push_back(new SRegisteredTemplateMemberX(TemplateMember));
}
void MeshLoaderX::SRegisteredTemplateX::addMember(SRegisteredTemplateX* TemplateMember, s32 ArraySize)
{
    MemberList.push_back(new SRegisteredTemplateMemberX(TemplateMember, ArraySize));
}
void MeshLoaderX::SRegisteredTemplateX::addArraySizeQualifier(const EMemberTypes Type)
{
    SRegisteredTemplateMemberX* NewMember = new SRegisteredTemplateMemberX(Type);
    NewMember->isArraySizeQualifier = true;
    MemberList.push_back(NewMember);
}


} // /namespace scene

} // /namespace sp


#endif



// ================================================================================
