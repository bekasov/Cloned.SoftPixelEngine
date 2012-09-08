/*
 * Script loader file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "Framework/Tools/spToolScriptLoader.hpp"

#ifdef SP_COMPILE_WITH_SCRIPTLOADER


#include "Platform/spSoftPixelDeviceOS.hpp"

#include <boost/foreach.hpp>


namespace sp
{

extern SoftPixelDevice* __spDevice;
extern video::RenderSystem* __spVideoDriver;
extern scene::SceneGraph* __spSceneManager;

namespace tool
{


#define __SCRIPT_DEBUG__

ScriptLoader::ScriptLoader(
    scene::SceneGraph* ActiveSceneGraph, audio::SoundDevice* ActiveSoundDevice) :
    File_               (0                                                      ),
    ActiveSceneGraph_   (ActiveSceneGraph ? ActiveSceneGraph : __spSceneManager ),
    ActiveSoundDevice_  (ActiveSoundDevice                                      ),
    CurLineNr_          (0                                                      ),
    CurSurface_         (0                                                      )
{
}
ScriptLoader::~ScriptLoader()
{
    MemoryManager::deleteList(CurSurfaceList_);
}

io::stringc ScriptLoader::getVersion() const
{
    return "ConstructiveScriptLoader - v.3.0 beta";
}


bool ScriptLoader::loadScriptFile(const io::stringc &Filename)
{
    WorkDir_    = Filename.getPathPart();
    CurLineNr_  = 0;
    
    if ( !( File_ = FileSys_.openFile(Filename, io::FILE_READ) ) )
        return false;
    
    io::Log::message("Load script file: \"" + Filename + "\"");
    io::Log::upperTab();
    
    bool Result = readScript();
    
    if (!Result)
        io::Log::error("Loading script failed");
    
    FileSys_.closeFile(File_);
    io::Log::lowerTab();
    
    return Result;
}

void ScriptLoader::setConstant(const io::stringc &Name, const io::stringc &Value)
{
    UniformMap_[Name.str()] = Value;
}

void ScriptLoader::clearLists()
{
    /* Delete all textures */
    for (std::vector<video::Texture*>::iterator it = Textures_.List.begin(); it != Textures_.List.end(); ++it)
    {
        if (*it && __spVideoDriver->isTexture(*it))
            __spVideoDriver->deleteTexture(*it);
    }
    Textures_.List.clear();
    
    /* Delete all sounds */
    if (ActiveSoundDevice_)
    {
        foreach (audio::Sound* Obj, Sounds_.List)
            ActiveSoundDevice_->deleteSound(Obj);
    }
    Sounds_.List.clear();
    
    /* Delete all meshes */
    foreach (scene::Mesh* Obj, Meshes_.List)
        ActiveSceneGraph_->deleteNode(Obj);
    Meshes_.List.clear();
    
    /* Delete all cameras */
    foreach (scene::Camera* Obj, Cameras_.List)
        ActiveSceneGraph_->deleteNode(Obj);
    Cameras_.List.clear();
    
    /* Delete all lights */
    foreach (scene::Light* Obj, Lights_.List)
        ActiveSceneGraph_->deleteNode(Obj);
    Lights_.List.clear();
    
    /* Delete all billboards */
    foreach (scene::Billboard* Obj, Billboards_.List)
        ActiveSceneGraph_->deleteNode(Obj);
    Billboards_.List.clear();
    
    /* Delete each surface */
    MemoryManager::deleteList(CurSurfaceList_);
}


/*
 * ========== Protected: ==========
 */

bool ScriptLoader::readScript()
{
    // Read the file until the end
    while (!File_->isEOF())
    {
        // Read the next line context
        if (!readLine())
            return false;
        
        // Read the next command and determine which type it is
        CurCommand_ = readNextCommand();
        
        switch (CurCommand_.Type)
        {
            case CMD_DIRECT:
                if (!readCommandDirect())
                    return false;
                break;
            case CMD_MACRO:
                if (!readCommandMacro())
                    return false;
                break;
            case CMD_FORLOOP:
                if (!readCommandForLoop())
                    return false;
                break;
            case CMD_BLOCK:
                if (!readCommandBlock())
                    return false;
                break;
            case CMD_ACCESS:
                if (!readCommandAccess())
                    return false;
                break;
            default:
                break;
        }
    }
    
    return true;
}


io::stringc ScriptLoader::readNextLine()
{
    ++CurLineNr_;
    return File_->readString();
}

bool ScriptLoader::checkEOF() const
{
    if (File_->isEOF())
    {
        printErrorLI("Unexpected end of file in expression");
        return true;
    }
    return false;
}

bool ScriptLoader::readLine()
{
    io::stringc CurLine;
    s32 pos1, pos2, tmp = 0;
    s32 QuotesCount = 0;
    Line_ = "";
    
    while (1)
    {
        // Read the next line
        CurLine = readNextLine();
        
        // Search multi-line comment
        if ( ( pos1 = CurLine.find("/*", 0) ) != -1 )
        {
            tmp = pos1 + 2;
            
            // Pass the loop until the end of the multi-line comment has found
            do
            {
                // Search the end of this multi-line comment
                pos2 = CurLine.find("*/", tmp);
                
                if (pos2 != -1)
                {
                    // Remove the comment out of the line context
                    CurLine = CurLine.left(pos1) + CurLine.right(CurLine.size() - pos2 - 2);
                }
                else
                {
                    // Check if the file ends unexpected
                    if (checkEOF())
                        return false;
                    
                    tmp = CurLine.size();
                    CurLine += readNextLine();
                }
            }
            while (pos2 == -1);
        }
        
        // Search single-line comment
        if ( ( pos1 = CurLine.find("//", 0) ) != -1 )
            CurLine = CurLine.left(pos1);
        
        // Add the read line to the line context
        Line_ += CurLine;
        
        // Check if the current line is not empty
        if (CurLine.size())
        {
            // Search multi-line strings
            pos1 = 0;
            
            while ( ( pos1 = CurLine.find("\"", pos1) ) != -1 )
            {
                ++pos1;
                ++QuotesCount;
            }
            
            // If the count of quotes is even stop reading lines
            if (QuotesCount % 2)
                Line_ += "\n";
            else
                break;
            
            // Check if the file ends unexpected
            if (checkEOF())
                return false;
        }
        else
        {
            // If the count of quotes is even and the file has end stop reading lines
            if (QuotesCount % 2 && checkEOF())
                return false;
            else if (File_->isEOF())
                break;
        }
    }
    
    // Store the trimed version of the line context
    Trimed_ = Line_.trim();
    
    return true;
}

void ScriptLoader::cropLineInside(s32 BeginPos, s32 EndPos)
{
    Line_ = Line_.left(BeginPos) + Line_.right(Line_.size() - EndPos);
}
void ScriptLoader::cropLineOutside(s32 BeginPos, s32 EndPos)
{
    Line_ = Line_.section(BeginPos, EndPos);
}

s32 ScriptLoader::readString(io::stringc &Str, s32 BeginPos) const
{
    s32 pos1, pos2;
    
    if ( ( pos1 = Line_.find("\"", BeginPos) ) != -1 && ( pos2 = Line_.find("\"", pos1 + 1) ) != -1 )
    {
        // Add the string to the given variable
        Str += Line_.section(pos1 + 1, pos2);
        
        // Replace extended characters
        Str = Str.replace("\\n", "\n");
        Str = Str.replace("\\t", "\t");
        
        // Return the end of the string in the line context
        return pos2 + 1;
    }
    
    return -1;
}

ScriptLoader::SCommand ScriptLoader::readNextCommand(s32 AllowedCommands) const
{
    SCommand Cmd;
    
    Cmd.Type = CMD_NONE;
    
    io::stringc CurName;
    io::stringc TmpTrimed = Trimed_.lower();
    s32 pos1, pos2;
    
    // Determine which kind of command it is
    if ( ( AllowedCommands & CMD_DIRECT ) &&
         ( ( CurName = TmpTrimed.left(6) ) == "import"  ||
           ( CurName = TmpTrimed.left(7) ) == "message" ||
           ( CurName = TmpTrimed.left(5) ) == "error"   ||
           ( CurName = TmpTrimed.left(7) ) == "warning" ||
           ( CurName = TmpTrimed.left(7) ) == "workdir" ) )
    {
        Cmd.Name = CurName;
        Cmd.Type = CMD_DIRECT;
    }
    else if ( ( AllowedCommands & CMD_MACRO ) &&
              ( ( CurName = TmpTrimed.left(7) ) == "uniform" ||
                ( CurName = TmpTrimed.left(3) ) == "var" ) )
    {
        Cmd.Name = CurName;
        Cmd.Type = CMD_MACRO;
    }
    else if ( ( AllowedCommands & CMD_FORLOOP ) &&
              ( Trimed_.left(3) == "for" || Trimed_.left(4) == "next" ) )
    {
        if (Trimed_.left(3) == "for")
            Cmd.Name = "for";
        else
            Cmd.Name = "next";
        
        Cmd.Type = CMD_FORLOOP;
    }
    else if ( ( AllowedCommands & CMD_BLOCK ) &&
              ( ( CurName = TmpTrimed.left( 5) ) == "node:"         ||
                ( CurName = TmpTrimed.left( 7) ) == "camera:"       ||
                ( CurName = TmpTrimed.left( 6) ) == "light:"        ||
                ( CurName = TmpTrimed.left( 5) ) == "mesh:"         ||
                ( CurName = TmpTrimed.left(10) ) == "billboard:"    ||
                ( CurName = TmpTrimed.left( 8) ) == "terrain:"      ||
                ( CurName = TmpTrimed.left( 8) ) == "texture:"      ||
                ( CurName = TmpTrimed.left( 6) ) == "sound:"        ||
                ( CurName = TmpTrimed.left( 7) ) == "shader:"       ||
                ( CurName = TmpTrimed.left( 8) ) == "surface:"      ||
                ( CurName = TmpTrimed.left( 7) ) == "vertex:"       ||
                ( CurName = TmpTrimed.left( 9) ) == "triangle:"     ||
                ( CurName = TmpTrimed.left( 5) ) == "anim:"         ||
                ( CurName = TmpTrimed.left( 6) ) == "frame:" ) )
    {
        Cmd.Name = CurName;
        Cmd.Type = CMD_BLOCK;
    }
    else if ( ( AllowedCommands & CMD_ACCESS ) &&
              Trimed_.left(1) == "[" &&
              ( pos1 = Line_.find("[", 0) ) != -1 &&
              ( pos2 = Line_.find("]", 0) ) != -1 )
    {
        Cmd.Name = Line_.section(pos1 + 1, pos2);
        Cmd.Type = CMD_ACCESS;
    }
    
    return Cmd;
}

bool ScriptLoader::readNextParameter(SParameter &Param, s32 BeginPos)
{
    s32 pos1 = -1, pos2, tmp = BeginPos;
    
    // Reset the parameter information
    Param.Name          = "";
    Param.StrValue      = "";
    Param.IntValue      = 0;
    Param.FltValue      = 0.0f;
    Param.VecValue.vec2 = 0.0f;
    Param.VecValue.vec3 = 0.0f;
    Param.VecValue.vec4 = dim::vector4df(0, 0, 0, 0);
    Param.VecValue.clr  = video::color(0, 0, 0, 0);
    
    // Search the end of the parameter (terminated by semicolon ";")
    if ( ( pos2 = Line_.find(";", BeginPos) ) == -1 )
        return false;
    
    // Search the begin of the parameter
    while (static_cast<u32>(tmp) < Line_.size() && tmp < pos2)
    {
        // Search for a valid parameter begin (upper and lower case letters and the underscore "_")
        if ( ( Line_[tmp] >= 'A' && Line_[tmp] <= 'Z' ) ||
             ( Line_[tmp] >= 'a' && Line_[tmp] <= 'z' ) ||
             Line_[tmp] == '_' )
        {
            pos1 = tmp;
            break;
        }
        
        ++tmp;
    }
    
    if (pos1 == -1)
        pos1 = 0;
    
    // Store the parameter part and crop the line context
    io::stringc CurLine = Line_.section(pos1, pos2);
    cropLineInside(pos1, pos2 + 1);
    
    // Search the parameter name
    if ( ( pos1 = CurLine.find("=", 0) ) == -1 )
    {
        if ( ( pos1 = CurLine.find("\t", 0) ) == -1 )
        {
            if ( ( pos1 = CurLine.find(" ", 0) ) == -1 )
                pos1 = CurLine.size();
        }
    }
    
    // Get the parameter name
    Param.Name = CurLine.left(pos1).trim().lower();
    
    // Search parameter values
    if ( ( pos1 = CurLine.find("=", 0) ) == -1 )
        return true;
    
    // Crop the current line at the front until the "=" character (we only want the parameter values now)
    CurLine = CurLine.right(CurLine.size() - pos1 - 1);
    
    // Search each parameter value
    do
    {
        // Search the next parameter value
        if (!readNextParameterValue(Param, CurLine))
            return false;
    }
    while (CurLine.trim().size());
    
    return true;
}

bool ScriptLoader::readNextParameterValue(SParameter &Param, io::stringc &CurLineContext)
{
    s32 pos1, pos2, tmp;
    
    // Search the "+" character if several parameter values are used
    io::stringc CurLine;
    
    if ( ( tmp = CurLineContext.find("+", 0) ) != -1 )
    {
        CurLine = CurLineContext.left(tmp);
        CurLineContext = CurLineContext.right(CurLineContext.size() - tmp - 1);
    }
    else
    {
        CurLine = CurLineContext;
        CurLineContext = "";
    }
    
    // Search string value
    if ( ( pos1 = CurLine.find("\"", 0) ) != -1 && ( pos2 = CurLine.find("\"", pos1 + 1) ) != -1 )
        Param.StrValue += CurLine.section(pos1 + 1, pos2);
    else
    {
        // Serach vector value
        if ( ( pos1 = CurLine.find("vec(", 0) ) != -1 && ( pos2 = CurLine.find(")", 0) ) != -1 )
        {
            // Get the vector string context
            const io::stringc VecStr = CurLine.section(pos1 + 4, pos2).trim();
            
            // Search each vector component
            pos1 = 0;
            std::vector<f32> VecList;
            
            do
            {
                if ( ( pos2 = VecStr.find(",", pos1) ) == -1 )
                    pos2 = VecStr.size();
                
                VecList.push_back(VecStr.section(pos1, pos2).val<f32>());
                
                pos1 = pos2 + 1;
            }
            while (static_cast<u32>(pos1) < VecStr.size());
            
            // Insert the vector components into the vector containers
            switch (VecList.size())
            {
                case 1:
                    Param.VecValue.vec2 += dim::point2df(VecList[0]);
                    Param.VecValue.vec3 += dim::vector3df(VecList[0]);
                    Param.VecValue.vec4 += dim::vector3df(VecList[0]);
                    Param.VecValue.clr  += video::color((u8)(VecList[0] * 255));
                    Param.Components    = 1;
                    break;
                    
                case 2:
                    Param.VecValue.vec2 += dim::point2df(VecList[0], VecList[1]);
                    Param.VecValue.vec3 += dim::vector3df(VecList[0], VecList[1], 0.0f);
                    Param.VecValue.vec4 += dim::vector3df(VecList[0], VecList[1], 0.0f);
                    Param.VecValue.clr  += video::color((u8)(VecList[0] * 255), (u8)(VecList[1] * 255), 0);
                    Param.Components    = 2;
                    break;
                    
                case 3:
                    Param.VecValue.vec2 += dim::point2df(VecList[0], VecList[1]);
                    Param.VecValue.vec3 += dim::vector3df(VecList[0], VecList[1], VecList[2]);
                    Param.VecValue.vec4 += dim::vector3df(VecList[0], VecList[1], VecList[2]);
                    Param.VecValue.clr  += video::color((u8)(VecList[0] * 255), (u8)(VecList[1] * 255), (u8)(VecList[2] * 255));
                    Param.Components    = 3;
                    break;
                    
                case 4:
                    Param.VecValue.vec2 += dim::point2df(VecList[0], VecList[1]);
                    Param.VecValue.vec3 += dim::vector3df(VecList[0], VecList[1], VecList[2]);
                    
                    Param.VecValue.vec4.X += VecList[0];
                    Param.VecValue.vec4.Y += VecList[1];
                    Param.VecValue.vec4.Z += VecList[2];
                    Param.VecValue.vec4.W += VecList[3];
                    
                    Param.VecValue.clr  += video::color((u8)(VecList[0] * 255), (u8)(VecList[1] * 255), (u8)(VecList[2] * 255), (u8)(VecList[3] * 255));
                    Param.Components    = 4;
                    break;
                    
                default:
                    printErrorLI("Invalid count of components in vector. Must be 1, 2, 3 or 4");
                    return false;
            }
        }
        else if ( ( pos1 = CurLine.find("[", 0) ) != -1 && ( pos2 = CurLine.find("]", 0) ) != -1 ) // Search object access
        {
            // Get access object name
            io::stringc ObjName = CurLine.section(pos1 + 1, pos2).trim();
            
            // Search object member begin
            pos1 = CurLine.find("->", 0);
            
            // Get member name
            io::stringc MemName;
            
            if (pos1 != -1)
                MemName = CurLine.right(CurLine.size() - pos1 - 2).trim();
            
            // Try access to object
            scene::SceneNode* Obj = NodeMap_[ObjName.str()];
            
            if (Obj)
            {
                dim::vector4df Vec;
                
                // Get member value
                if (MemName == "pos")
                    Vec = Obj->getPosition();
                else if (MemName == "rot")
                    Vec = Obj->getRotation();
                else if (MemName == "scl")
                    Vec = Obj->getScale();
                else
                    return true;
                
                // Apply vector values
                Param.VecValue.vec2 += dim::point2df(Vec.X, Vec.Y);
                Param.VecValue.vec3 += dim::vector3df(Vec.X, Vec.Y, Vec.Z);
                
                Param.VecValue.vec4.X += Vec.X;
                Param.VecValue.vec4.Y += Vec.Y;
                Param.VecValue.vec4.Z += Vec.Z;
                Param.VecValue.vec4.W += Vec.W;
                
                Param.VecValue.clr  += video::color((u8)(Vec.X * 255), (u8)(Vec.Y * 255), (u8)(Vec.Z * 255), (u8)(Vec.W * 255));
                Param.Components    = 4;
                
                return true;
            }
            
            // Try access to variable
            SParameter Variable = VariableMap_[ObjName.str()];
            
            if (MemName == "x" || MemName == "r")
            {
                Param.VecValue.vec2.X       += Variable.VecValue.vec4.X;
                Param.VecValue.vec3.X       += Variable.VecValue.vec4.X;
                Param.VecValue.vec4.X       += Variable.VecValue.vec4.X;
                Param.VecValue.clr.Red      += Variable.VecValue.clr.Red;
                Param.FltValue              += Variable.VecValue.vec4.X;
                Param.IntValue              += (s32)Variable.VecValue.vec4.X;
                Param.StrValue              += io::stringc(Variable.VecValue.vec4.X);
                Param.Components            = 1;
            }
            else if (MemName == "y" || MemName == "g")
            {
                Param.VecValue.vec2.Y       += Variable.VecValue.vec4.Y;
                Param.VecValue.vec3.Y       += Variable.VecValue.vec4.Y;
                Param.VecValue.vec4.Y       += Variable.VecValue.vec4.Y;
                Param.VecValue.clr.Green    += Variable.VecValue.clr.Green;
                Param.FltValue              += Variable.VecValue.vec4.Y;
                Param.IntValue              += (s32)Variable.VecValue.vec4.Y;
                Param.StrValue              += io::stringc(Variable.VecValue.vec4.Y);
                Param.Components            = 1;
            }
            else if (MemName == "z" || MemName == "b")
            {
                Param.VecValue.vec3.Z       += Variable.VecValue.vec4.Z;
                Param.VecValue.vec4.Z       += Variable.VecValue.vec4.Z;
                Param.VecValue.clr.Blue     += Variable.VecValue.clr.Blue;
                Param.FltValue              += Variable.VecValue.vec4.Z;
                Param.IntValue              += (s32)Variable.VecValue.vec4.Z;
                Param.StrValue              += io::stringc(Variable.VecValue.vec4.Z);
                Param.Components            = 1;
            }
            else if (MemName == "w" || MemName == "a")
            {
                Param.VecValue.vec4.W       += Variable.VecValue.vec4.W;
                Param.VecValue.clr.Alpha    += Variable.VecValue.clr.Alpha;
                Param.FltValue              += Variable.VecValue.vec4.W;
                Param.IntValue              += (s32)Variable.VecValue.vec4.W;
                Param.StrValue              += io::stringc(Variable.VecValue.vec4.W);
                Param.Components            = 1;
            }
            else if (MemName == "vec")
            {
                Param.VecValue.vec2     += Variable.VecValue.vec2;
                Param.VecValue.vec3     += Variable.VecValue.vec3;
                
                Param.VecValue.vec4.X   += Variable.VecValue.vec4.X;
                Param.VecValue.vec4.Y   += Variable.VecValue.vec4.Y;
                Param.VecValue.vec4.Z   += Variable.VecValue.vec4.Z;
                Param.VecValue.vec4.W   += Variable.VecValue.vec4.W;
                
                Param.VecValue.clr      += Variable.VecValue.clr;
                
                Param.Components        = 4;
            }
            else
            {
                Param.StrValue      += Variable.StrValue;
                Param.FltValue      += Variable.FltValue;
                Param.IntValue      += Variable.IntValue;
                Param.Components    = 1;
            }
        }
        else if (CurLine.find("true", 0) != -1) // Search boolean value (true)
        {
            Param.BitValue      = true;
            Param.Components    = 1;
        }
        else if (CurLine.find("false", 0) != -1) // Search boolean value (false)
        {
            Param.BitValue      = false;
            Param.Components    = 1;
        }
        else
        {
            // Search integer and float value
            const io::stringc ValStr = CurLine.trim();
            
            Param.StrValue += io::stringc(ValStr.val<f32>());
            Param.IntValue += ValStr.val<s32>();
            Param.FltValue += ValStr.val<f32>();
            
            Param.Components = 1;
        }
    }
    
    return true;
}

io::stringc ScriptLoader::getLinesIndicate() const
{
    return " (Line #" + io::stringc(CurLineNr_) + ")";
}

void ScriptLoader::printErrorLI(const io::stringc &Message) const
{
    io::Log::error(Message + getLinesIndicate());
}
void ScriptLoader::printWarningLI(const io::stringc &Message) const
{
    io::Log::warning(Message + getLinesIndicate());
}


bool ScriptLoader::readCommandDirect()
{
    // Read the content string
    io::stringc Str, Param;
    
    s32 pos1 = readString(Str);
    readString(Param, pos1);
    
    Param = Param.lower();
    
    // Determine which type of direct command it is
    if (CurCommand_.Name == "import")
    {
        // Store basic information temporaryly for recursive function call
        io::File* TmpFile       = File_;
        u32 TmpLineNr           = CurLineNr_;
        io::stringc TmpLine     = Line_;
        io::stringc TmpTrimed   = Trimed_;
        
        // Import the other specified script file
        loadScriptFile(WorkDir_ + Str);
        
        // Reset the basic information
        File_       = TmpFile;
        CurLineNr_  = TmpLineNr;
        Line_       = TmpLine;
        Trimed_     = TmpTrimed;
    }
    else if (CurCommand_.Name == "message")
    {
        if (Param == "msgbox")  io::Log::message(Str, io::LOG_MSGBOX);
        else                    io::Log::message(Str);
    }
    else if (CurCommand_.Name == "error")
    {
        if (Param == "msgbox")  io::Log::error(Str, io::LOG_MSGBOX);
        else                    io::Log::error(Str);
    }
    else if (CurCommand_.Name == "warning")
    {
        if (Param == "msgbox")  io::Log::warning(Str, io::LOG_MSGBOX);
        else                    io::Log::warning(Str);
    }
    else if (CurCommand_.Name == "workdir")
        WorkDir_ = Str;
    
    return true;
}

bool ScriptLoader::readCommandMacro()
{
    SParameter Param;
    
    if (readNextParameter(Param, CurCommand_.Name.size()))
    {
        if (CurCommand_.Name == "uniform")
        {
            // Register the specified uniform as a valid one
            InternalUniformMap_[Param.Name.str()] = true;
        }
        else if (CurCommand_.Name == "var")
        {
            // Add the new variable
            VariableMap_[Param.Name.str()] = Param;
            
            #ifdef __SCRIPT_DEBUG__
            //io::getMessage(Param.StrValue);
            #endif
        }
    }
    else
    {
        printErrorLI("Incomplete macro declaration");
        return false;
    }
    
    return true;
}

bool ScriptLoader::readCommandForLoop()
{
    if (CurCommand_.Name == "for")
    {
        s32 pos1, pos2, pos3;
        
        // Search the begin index
        if ( ( pos1 = Line_.find("=", 0) ) == -1 )
        {
            printErrorLI("Missing \"=\" directive in for loop");
            return false;
        }
        
        // Search the end index
        if ( ( pos2 = Line_.find("to", 0) ) == -1 )
        {
            printErrorLI("Missing \"to\" directive in for loop");
            return false;
        }
        
        // Search the index varaible begin
        if ( ( pos3 = Line_.find("for", 0) ) == -1 )
        {
            printErrorLI("Corrupt for loop directive at the begin");
            return false;
        }
        
        // Get the index variable name
        const io::stringc IndexName = Line_.section(pos3 + 3, pos1).trim();
        
        // Store the for loop range
        SLoopRange Loop;
        {
            Loop.FilePos    = File_->getSeek();
            Loop.FileLine   = CurLineNr_;
            Loop.Index      = Line_.section(pos1 + 1, pos2).trim().val<s32>();
            Loop.MaxIndex   = Line_.right(Line_.size() - pos2 - 2).trim().val<s32>();
        }
        LoopMap_[IndexName.str()] = Loop;
    }
    else if (CurCommand_.Name == "next")
    {
        // Find the index variable name
        s32 pos = Line_.find("next", 0);
        
        if (pos == -1)
        {
            printErrorLI("Corrupt for loop directive at the end");
            return false;
        }
        
        // Get the index variable name
        const io::stringc IndexName = Line_.right(Line_.size() - pos - 4).trim();
        
        // Get the loop range
        SLoopRange Loop = LoopMap_[IndexName.str()];
        
        // Boost the loop index
        if (Loop.Index < Loop.MaxIndex)
        {
            // Go to the begin of the for loop
            File_->setSeek(Loop.FilePos);
            CurLineNr_ = Loop.FileLine;
            
            ++LoopMap_[IndexName.str()].Index;
        }
    }
    
    return true;
}

bool ScriptLoader::readCommandBlock()
{
    ParamMap_.clear();
    
    bool isBlockBegin = false;
    s32 ParamBeginPos = 0;
    SCommand TmpCommand, LastCommand;
    
    // Read the file until the block has end
    do
    {
        // Check if the block has begun
        if ( !isBlockBegin && ( ParamBeginPos = Line_.find("{", 0) ) != -1 )
        {
            isBlockBegin = true;
            ++ParamBeginPos;
        }
        
        if (isBlockBegin)
        {
            // Crop the line at the front
            if (ParamBeginPos > 0)
            {
                Line_   = Line_.right(Line_.size() - ParamBeginPos);
                Trimed_ = Line_.trim();
            }
            
            // Read the next command and determine which type it is
            TmpCommand = readNextCommand(CMD_BLOCK);
            
            if (TmpCommand.Type == CMD_BLOCK)
            {
                // Store the current information for recursive function call
                std::map<std::string, SParameter> TmpParamMap = ParamMap_;
                LastCommand = CurCommand_;
                CurCommand_ = TmpCommand;
                
                // Read the next internal block
                if (!readCommandBlock())
                    return false;
                
                // Reset the information
                ParamMap_   = TmpParamMap;
                CurCommand_ = LastCommand;
            }
            else
            {
                // Read all parameters in the line context
                SParameter Param;
                
                while (readNextParameter(Param))
                {
                    Param.Used = true;
                    ParamMap_[Param.Name.str()] = Param;
                }
                
                // Check if the block has end
                if (Line_.find("}", 0) != -1)
                    break;
            }
        }
        
        // Read the next line context
        if (!readLine())
            return false;
        
        ParamBeginPos = 0;
    }
    while (!File_->isEOF());
    
    // Analyse the parameters for the specified block type
    if (CurCommand_.Name == "node:")
        return examineBlockNode();
    else if (CurCommand_.Name == "mesh:")
        return examineBlockMesh();
    else if (CurCommand_.Name == "camera:")
        return examineBlockCamera();
    else if (CurCommand_.Name == "light:")
        return examineBlockLight();
    else if (CurCommand_.Name == "billboard:")
        return examineBlockBillboard();
    else if (CurCommand_.Name == "terrain:")
        return examineBlockTerrain();
    else if (CurCommand_.Name == "texture:")
        return examineBlockTexture();
    else if (CurCommand_.Name == "sound:")
        return examineBlockSound();
    else if (CurCommand_.Name == "shader:")
        return examineBlockShader();
    else if (CurCommand_.Name == "anim:")
        return examineBlockAnim();
    else if (CurCommand_.Name == "frame:")
        return examineBlockFrame();
    else if (CurCommand_.Name == "surface:")
        return examineBlockSurface();
    else if (CurCommand_.Name == "vertex:")
        return examineBlockVertex();
    else if (CurCommand_.Name == "triangle:")
        return examineBlockTriangle();
    
    printErrorLI("Unknown block type");
    
    return false;
}

bool ScriptLoader::readCommandAccess()
{
    SParameter Param;
    
    // Get the parameter
    s32 pos1 = Line_.find("->", 0);
    
    if (pos1 == -1)
    {
        if ( ( pos1 = Line_.find("]", 0) ) == -1)
        {
            printErrorLI("Corrput directive after object access");
            return false;
        }
        else
            ++pos1;
    }
    else
        pos1 += 2;
    
    if (!readNextParameter(Param, pos1))
        return false;
    
    // Try access to object
    scene::SceneNode* Obj = NodeMap_[CurCommand_.Name.str()];
    
    if (Obj)
    {
        // Analyse the parameter
        if (Param.Name == "pos")            Obj->setPosition(Param.VecValue.vec3);
        else if (Param.Name == "scl")       Obj->setScale(Param.VecValue.vec3);
        else if (Param.Name == "name")      printWarningLI("Object names may not be changed in script file");
        else if (Param.Name == "rot")
        {
            if (Param.Components == 4)
                Obj->setRotationMatrix(dim::quaternion(Param.VecValue.vec4).getMatrix());
            else
                Obj->setRotation(Param.VecValue.vec3);
        }
        else if (Param.Name == "visible")   Obj->setVisible(Param.BitValue);
        else if (Param.Name == "parent")
        {
            scene::SceneNode* ParentObj = NodeMap_[CurParam_.StrValue.str()];
            if (ParentObj) Obj->setParent(ParentObj);
        }
        
        // Analyse the parameter for meshes
        if (Obj->getType() == scene::NODE_MESH)
        {
            scene::Mesh* MeshObj = (scene::Mesh*)Obj;
            
            if (Param.Name == "color")  MeshObj->paint(Param.VecValue.clr);
        }
        
        return true;
    }
    
    // Try access to variable
    SParameter Variable = VariableMap_[CurCommand_.Name.str()];
    
    if (Param.Name == "x" || Param.Name == "r")
    {
        Variable.VecValue.vec2.X    = Param.FltValue;
        Variable.VecValue.vec3.X    = Param.FltValue;
        Variable.VecValue.vec4.X    = Param.FltValue;
        Variable.VecValue.clr.Red   = Param.IntValue;
    }
    else if (Param.Name == "y" || Param.Name == "g")
    {
        Variable.VecValue.vec2.Y    = Param.FltValue;
        Variable.VecValue.vec3.Y    = Param.FltValue;
        Variable.VecValue.vec4.Y    = Param.FltValue;
        Variable.VecValue.clr.Green = Param.IntValue;
    }
    else if (Param.Name == "z" || Param.Name == "b")
    {
        Variable.VecValue.vec3.Z    = Param.FltValue;
        Variable.VecValue.vec4.Z    = Param.FltValue;
        Variable.VecValue.clr.Blue  = Param.IntValue;
    }
    else if (Param.Name == "w" || Param.Name == "a")
    {
        Variable.VecValue.vec4.W    = Param.FltValue;
        Variable.VecValue.clr.Alpha = Param.IntValue;
    }
    else
    {
        Variable.FltValue = Param.FltValue;
        Variable.IntValue = Param.IntValue;
        Variable.StrValue = Param.StrValue;
    }
    
    VariableMap_[CurCommand_.Name.str()] = Variable;
    
    return true;
}


bool ScriptLoader::isParam(const io::stringc &Name)
{
    CurParam_ = ParamMap_[Name.str()];
    return CurParam_.Used;
}

bool ScriptLoader::examineBlockNode()
{
    scene::SceneNode* Obj = ActiveSceneGraph_->createNode();
    
    examineBlockNode(Obj);
    
    return true;
}

bool ScriptLoader::examineBlockMesh()
{
    scene::Mesh* Obj = 0;
    scene::SMeshConstruct Construct;
    
    if (isParam("segs"))    Construct.SegmentsHorz = Construct.SegmentsVert = CurParam_.IntValue;
    if (isParam("vsegs"))   Construct.SegmentsVert = CurParam_.IntValue;
    if (isParam("hsegs"))   Construct.SegmentsHorz = CurParam_.IntValue;
    
    if (isParam("file"))
        Obj = ActiveSceneGraph_->loadMesh(WorkDir_ + CurParam_.StrValue);
    else if (isParam("prim"))
    {
        const io::stringc Prim = CurParam_.StrValue.lower();
        
        if (Prim == "cube")                 Obj = ActiveSceneGraph_->createMesh(scene::MESH_CUBE,           Construct);
        else if (Prim == "cone")            Obj = ActiveSceneGraph_->createMesh(scene::MESH_CONE,           Construct);
        else if (Prim == "cylinder")        Obj = ActiveSceneGraph_->createMesh(scene::MESH_CYLINDER,       Construct);
        else if (Prim == "sphere")          Obj = ActiveSceneGraph_->createMesh(scene::MESH_SPHERE,         Construct);
        else if (Prim == "icosphere")       Obj = ActiveSceneGraph_->createMesh(scene::MESH_ICOSPHERE,      Construct);
        else if (Prim == "torus")           Obj = ActiveSceneGraph_->createMesh(scene::MESH_TORUS,          Construct);
        else if (Prim == "torusknot")       Obj = ActiveSceneGraph_->createMesh(scene::MESH_TORUSKNOT,      Construct);
        else if (Prim == "spiral")          Obj = ActiveSceneGraph_->createMesh(scene::MESH_SPIRAL,         Construct);
        else if (Prim == "pipe")            Obj = ActiveSceneGraph_->createMesh(scene::MESH_PIPE,           Construct);
        else if (Prim == "disk")            Obj = ActiveSceneGraph_->createMesh(scene::MESH_DISK,           Construct);
        else if (Prim == "plane")           Obj = ActiveSceneGraph_->createMesh(scene::MESH_PIPE,           Construct);
        else if (Prim == "tetrahedron")     Obj = ActiveSceneGraph_->createMesh(scene::MESH_TETRAHEDRON,    Construct);
        else if (Prim == "cuboctahedron")   Obj = ActiveSceneGraph_->createMesh(scene::MESH_CUBOCTAHEDRON,  Construct);
        else if (Prim == "icosahedron")     Obj = ActiveSceneGraph_->createMesh(scene::MESH_ICOSAHEDRON,    Construct);
        else if (Prim == "octahedron")      Obj = ActiveSceneGraph_->createMesh(scene::MESH_OCTAHEDRON,     Construct);
        else if (Prim == "dodecahedron")    Obj = ActiveSceneGraph_->createMesh(scene::MESH_DISK,           Construct);
        else if (Prim == "teapot")          Obj = ActiveSceneGraph_->createMesh(scene::MESH_TEAPOT,         Construct);
        else
        {
            printErrorLI("Unknown basic primitive mesh: \"" + Prim + "\"");
            return false;
        }
    }
    else
        Obj = ActiveSceneGraph_->createMesh();
    
    if (!Obj)
    {
        printErrorLI("Could not create mesh");
        return false;
    }
    
    examineBlockMaterialNode(Obj);
    
    if (isParam("color"))
        Obj->paint(CurParam_.VecValue.clr);
    if (isParam("tex"))
    {
        video::Texture* Tex = TextureMap_[CurParam_.StrValue.str()];
        if (Tex)
            Obj->addTexture(Tex);
    }
    
    if (!CurSurfaceList_.empty())
        applySurfaces(Obj);
    
    Obj->updateNormals();
    
    return true;
}

bool ScriptLoader::examineBlockLight()
{
    scene::Light* Obj = 0;
    scene::ELightModels Type = scene::LIGHT_DIRECTIONAL;
    
    video::color Diffuse, Ambient, Specular;
    
    if (isParam("type"))
    {
        if (CurParam_.StrValue == "dir")        Type = scene::LIGHT_DIRECTIONAL;
        else if (CurParam_.StrValue == "point") Type = scene::LIGHT_POINT;
        else if (CurParam_.StrValue == "spot")  Type = scene::LIGHT_SPOT;
    }
    
    Obj = ActiveSceneGraph_->createLight(Type);
    Obj->getLightingColor(Diffuse, Ambient, Specular);
    
    examineBlockNode(Obj);
    
    if (isParam("diffuse"))     Diffuse = CurParam_.VecValue.clr;
    if (isParam("ambient"))     Ambient = CurParam_.VecValue.clr;
    if (isParam("specular"))    Specular = CurParam_.VecValue.clr;
    
    // If first light has been create activate lighting
    ActiveSceneGraph_->setLighting();
    
    Obj->setLightingColor(Diffuse, Ambient, Specular);
    
    return true;
}

bool ScriptLoader::examineBlockCamera()
{
    scene::Camera* Obj = ActiveSceneGraph_->createCamera();
    
    f32 Near = Obj->getRangeNear();
    f32 Far = Obj->getRangeFar();
    
    examineBlockNode(Obj);
    
    if (isParam("ortho"))   Obj->setOrtho(true);
    if (isParam("fov"))     Obj->setFOV(CurParam_.FltValue);
    if (isParam("near"))    Near = CurParam_.FltValue;
    if (isParam("far"))     Far = CurParam_.FltValue;
    if (isParam("viewport"))
    {
        Obj->setViewport(
            dim::rect2di(
                (s32)CurParam_.VecValue.vec4.X, (s32)CurParam_.VecValue.vec4.Y,
                (s32)CurParam_.VecValue.vec4.Z, (s32)CurParam_.VecValue.vec4.W
            )
        );
    }
    
    Obj->setRange(Near, Far);
    Obj->updatePerspective();
    
    return true;
}

bool ScriptLoader::examineBlockBillboard()
{
    
    return true;
}

bool ScriptLoader::examineBlockTerrain()
{
    
    return true;
}

bool ScriptLoader::examineBlockTexture()
{
    video::Texture* Obj = 0;
    
    if (isParam("file"))
    {
        Obj = __spVideoDriver->loadTexture(WorkDir_ + CurParam_.StrValue);
        
        if (isParam("size"))
        {
            Obj->setSize(
                dim::size2di((s32)CurParam_.VecValue.vec2.X, (s32)CurParam_.VecValue.vec2.Y)
            );
        }
    }
    else if (isParam("size"))
    {
        Obj = __spVideoDriver->createTexture(
            dim::size2di((s32)CurParam_.VecValue.vec2.X, (s32)CurParam_.VecValue.vec2.Y)
        );
    }
    else
        Obj = __spVideoDriver->createTexture(video::DEF_TEXTURE_SIZE);
    
    if (isParam("format"))
    {
        if (CurParam_.StrValue == "gray")           Obj->setFormat(video::PIXELFORMAT_GRAY);
        else if (CurParam_.StrValue == "grayalpha") Obj->setFormat(video::PIXELFORMAT_GRAYALPHA);
        else if (CurParam_.StrValue == "rgb")       Obj->setFormat(video::PIXELFORMAT_RGB);
        else if (CurParam_.StrValue == "rgba")      Obj->setFormat(video::PIXELFORMAT_RGBA);
        else if (CurParam_.StrValue == "bgr")       Obj->setFormat(video::PIXELFORMAT_BGR);
        else if (CurParam_.StrValue == "bgra")      Obj->setFormat(video::PIXELFORMAT_BGRA);
    }
    if (isParam("filter"))
    {
        if (CurParam_.StrValue == "linear")         Obj->setFilter(video::FILTER_LINEAR);
        else if (CurParam_.StrValue == "smooth")    Obj->setFilter(video::FILTER_SMOOTH);
    }
    if (isParam("anisotropy"))
        Obj->setAnisotropicSamples(CurParam_.IntValue);
    if (isParam("mipmap"))
    {
        if (CurParam_.StrValue == "bilinear")           Obj->setMipMapFilter(video::FILTER_BILINEAR);
        else if (CurParam_.StrValue == "trilinear")     Obj->setMipMapFilter(video::FILTER_TRILINEAR);
        else if (CurParam_.StrValue == "anisotropic")   Obj->setMipMapFilter(video::FILTER_ANISOTROPIC);
    }
    if (isParam("name"))
        TextureMap_[CurParam_.StrValue.str()] = Obj;
    
    return true;
}

bool ScriptLoader::examineBlockSound()
{
    
    return true;
}

bool ScriptLoader::examineBlockShader()
{
    #ifdef __SCRIPT_DEBUG__
    //if (isParam("vs"))  io::getMessage(CurParam_.StrValue);
    //if (isParam("ps"))  io::getMessage(CurParam_.StrValue);
    #endif
    
    return true;
}

bool ScriptLoader::examineBlockAnim()
{
    CurAnim_.Used               = true;
    CurAnim_.Splines            = false;
    CurAnim_.SplineExpansion    = 1.0f;
    CurAnim_.Speed              = 1.0f;
    
    if (isParam("play"))        CurAnim_.Play               = CurParam_.StrValue;
    if (isParam("speed"))       CurAnim_.Speed              = CurParam_.FltValue;
    if (isParam("splines"))     CurAnim_.Splines            = CurParam_.BitValue;
    if (isParam("expansion"))   CurAnim_.SplineExpansion    = CurParam_.FltValue;
    
    return true;
}

bool ScriptLoader::examineBlockFrame()
{
    SAnimationFrame Frame;
    Frame.Speed = 1.0f;
    Frame.Scl   = 1.0f;
    
    if (isParam("speed"))   Frame.Speed = CurParam_.FltValue;
    if (isParam("pos"))     Frame.Pos   = CurParam_.VecValue.vec3;
    if (isParam("scl"))     Frame.Scl   = CurParam_.VecValue.vec3;
    if (isParam("rot"))
    {
        if (CurParam_.Components == 4)
            Frame.Rot = dim::quaternion(CurParam_.VecValue.vec4);
        else
            Frame.Rot = dim::quaternion(CurParam_.VecValue.vec3);
    }
    
    CurAnim_.Frames.push_back(Frame);
    
    return true;
}

bool ScriptLoader::examineBlockSurface()
{
    if (isParam("name"))  CurSurface_->setName(CurParam_.StrValue);
    if (isParam("tex"))
    {
        video::Texture* Tex = TextureMap_[CurParam_.StrValue.str()];
        if (Tex)
            CurSurface_->addTexture(Tex);
    }
    
    CurSurfaceList_.push_back(CurSurface_);
    CurSurface_ = 0;
    
    return true;
}

bool ScriptLoader::examineBlockVertex()
{
    if (!CurSurface_)
        CurSurface_ = new video::MeshBuffer();
    
    scene::SMeshVertex3D Vertex;
    
    if (isParam("pos"))     Vertex.setPosition(CurParam_.VecValue.vec3);
    if (isParam("normal"))  Vertex.setNormal(CurParam_.VecValue.vec3);
    if (isParam("map"))     Vertex.setTexCoord(0, CurParam_.VecValue.vec3);
    if (isParam("color"))   Vertex.setColor(CurParam_.VecValue.clr);
    
    CurSurface_->addVertex(Vertex);
    
    return true;
}

bool ScriptLoader::examineBlockTriangle()
{
    if (!CurSurface_)
        CurSurface_ = new video::MeshBuffer();
    
    scene::SMeshTriangle3D Triangle;
    
    if (isParam("indices"))
    {
        Triangle.a = (u32)CurParam_.VecValue.vec3.X;
        Triangle.b = (u32)CurParam_.VecValue.vec3.Y;
        Triangle.c = (u32)CurParam_.VecValue.vec3.Z;
    }
    
    CurSurface_->addTriangle(Triangle.a, Triangle.b, Triangle.c);
    
    return true;
}


bool ScriptLoader::examineBlockNode(scene::SceneNode* Obj)
{
    if (CurAnim_.Used)
    {
        applyAnimation(Obj);
        CurAnim_.Used = false;
    }
    
    if (isParam("name"))
        Obj->setName(CurParam_.StrValue);
    
    if (Obj->getName() != "")
        NodeMap_[Obj->getName().str()] = Obj;
    
    if (isParam("pos"))     Obj->setPosition(CurParam_.VecValue.vec3);
    if (isParam("rot"))
    {
        if (CurParam_.Components == 4)
            Obj->setRotationMatrix(dim::quaternion(CurParam_.VecValue.vec4).getMatrix());
        else
            Obj->setRotation(CurParam_.VecValue.vec3);
    }
    if (isParam("scl"))     Obj->setScale(CurParam_.VecValue.vec3);
    if (isParam("point"))   Obj->lookAt(CurParam_.VecValue.vec3);
    if (isParam("hide"))    Obj->setVisible(false);
    if (isParam("visible")) Obj->setVisible(CurParam_.BitValue);
    if (isParam("parent"))
    {
        scene::SceneNode* ParentObj = NodeMap_[CurParam_.StrValue.str()];
        if (ParentObj) Obj->setParent(ParentObj);
    }
    
    return true;
}

bool ScriptLoader::examineBlockMaterialNode(scene::MaterialNode* Obj)
{
    examineBlockNode(Obj);
    
    video::MaterialStates* Material = Obj->getMaterial();
    
    if (isParam("diffuse"))     Material->setDiffuseColor(CurParam_.VecValue.clr);
    if (isParam("ambient"))     Material->setAmbientColor(CurParam_.VecValue.clr);
    if (isParam("specular"))    Material->setSpecularColor(CurParam_.VecValue.clr);
    if (isParam("emission"))    Material->setEmissionColor(CurParam_.VecValue.clr);
    if (isParam("wire"))
    {
        if (CurParam_.StrValue == "solid")          Material->setWireframe(video::WIREFRAME_SOLID);
        else if (CurParam_.StrValue == "lines")     Material->setWireframe(video::WIREFRAME_LINES);
        else if (CurParam_.StrValue == "points")    Material->setWireframe(video::WIREFRAME_POINTS);
    }
    if (isParam("lighting"))    Material->setLighting(CurParam_.BitValue);
    if (isParam("fog"))         Material->setFog(CurParam_.BitValue);
    if (isParam("shading"))
    {
        if (CurParam_.StrValue == "flat")           Material->setShading(video::SHADING_FLAT);
        else if (CurParam_.StrValue == "gouraud")   Material->setShading(video::SHADING_GOURAUD);
        else if (CurParam_.StrValue == "phong")     Material->setShading(video::SHADING_PHONG);
        else if (CurParam_.StrValue == "perpixel")  Material->setShading(video::SHADING_PERPIXEL);
        
        if (Obj->getType() == scene::NODE_MESH)
            ((scene::Mesh*)Obj)->updateNormals();
    }
    if (isParam("face"))
    {
        if (CurParam_.StrValue == "front")      Material->setRenderFace(video::FACE_FRONT);
        else if (CurParam_.StrValue == "back")  Material->setRenderFace(video::FACE_BACK);
        else if (CurParam_.StrValue == "both")  Material->setRenderFace(video::FACE_BOTH);
    }
    
    return true;
}


void ScriptLoader::applyAnimation(scene::SceneNode* Obj)
{
    scene::NodeAnimation* NodeAnim = ActiveSceneGraph_->createAnimation<scene::NodeAnimation>();
    Obj->addAnimation(NodeAnim);
    
    foreach (const SAnimationFrame &Frame, CurAnim_.Frames)
    {
        NodeAnim->addKeyframe(
            scene::Transformation(Frame.Pos, Frame.Rot, Frame.Scl),
            static_cast<u64>(Frame.Speed * 1000)
        );
    }
    
    NodeAnim->setSpeed(CurAnim_.Speed);
    
         if (CurAnim_.Play == "oneshot"     ) NodeAnim->play(scene::PLAYBACK_ONESHOT        );
    else if (CurAnim_.Play == "oneloop"     ) NodeAnim->play(scene::PLAYBACK_ONELOOP        );
    else if (CurAnim_.Play == "loop"        ) NodeAnim->play(scene::PLAYBACK_LOOP           );
    else if (CurAnim_.Play == "pingpong"    ) NodeAnim->play(scene::PLAYBACK_PINGPONG       );
    else if (CurAnim_.Play == "pingpongloop") NodeAnim->play(scene::PLAYBACK_PINGPONG_LOOP  );
    
    if (CurAnim_.Splines)
    {
        NodeAnim->setSplineTranslation(true);
        NodeAnim->setSplineExpansion(CurAnim_.SplineExpansion);
    }
    
    CurAnim_.Frames.clear();
}

void ScriptLoader::applySurfaces(scene::Mesh* Obj)
{
    video::MeshBuffer* Surface = 0;
    u32 Indices[3];
    
    // Apply each surface
    for (std::list<video::MeshBuffer*>::iterator itSurf = CurSurfaceList_.begin(); itSurf != CurSurfaceList_.end(); ++itSurf)
    {
        Surface = Obj->createMeshBuffer();
        
        // Apply each vertex
        for (u32 i = 0; i < (*itSurf)->getVertexCount(); ++i)
        {
            Surface->addVertex(
                (*itSurf)->getVertexCoord(i),
                (*itSurf)->getVertexNormal(i),
                (*itSurf)->getVertexTexCoord(i, 0),
                (*itSurf)->getVertexColor(i),
                (*itSurf)->getVertexFog(i)
            );
        }
        
        // Apply each triangle
        for (u32 i = 0; i < (*itSurf)->getTriangleCount(); ++i)
        {
            (*itSurf)->getTriangleIndices(i, Indices);
            Surface->addTriangle(Indices);
        }
        
        MemoryManager::deleteMemory(*itSurf);
    }
    
    CurSurfaceList_.clear();
    CurSurface_ = 0;
    
    Obj->updateMeshBuffer();
}


} // /namespace tool

} // /namespace sp


#endif



// ================================================================================
