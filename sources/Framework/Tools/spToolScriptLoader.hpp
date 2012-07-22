/*
 * Script loader header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_TOOL_SCRIPTLOADER_H__
#define __SP_TOOL_SCRIPTLOADER_H__


#include "Base/spStandard.hpp"

#ifdef SP_COMPILE_WITH_SCRIPTLOADER


#include "Base/spInputOutputString.hpp"
#include "Base/spInputOutputFileSystem.hpp"
#include "RenderSystem/spTextureBase.hpp"
#include "SoundSystem/spSoundDevice.hpp"
#include "SceneGraph/spSceneGraph.hpp"

#include <list>
#include <vector>


namespace sp
{
namespace tool
{


/**
The ScriptLoader can be used to load several textures, sounds and geometry objects. The basic idea of the
"ConstructiveScriptLoader" is to create 3D scenes very fast and easy. Here is an example of a script file:
\code
// // Comments are equal to those in C/C++
// 
// // Changes the current working directory to "media/" (only internally; your program will not be affected)
// workdir "media/"
// 
// // Loads the "media/HLC.png" file, resizes the texture to 32x32 texels, sets the min & mag filter
// texture: { file = "HLC.png"; size = vec(32, 32); filter = "bilinear"; }
\endcode
*/
class SP_EXPORT ScriptLoader
{
    
    public:
        
        ScriptLoader();
        virtual ~ScriptLoader();
        
        /* Functions */
        
        //! \return ScriptLoader version (e.g. "ConstructiveScriptLoader - v.2.0").
        virtual io::stringc getVersion() const;
        
        /**
        Loads a "SoftPixel Engine" script file.
        \param Filename: Script filename which is to be loaded.
        \return True if the script file could be loaded successful. Otherwise false.
        */
        bool loadScriptFile(const io::stringc &Filename);
        
        void setConstant(const io::stringc &Name, const io::stringc &Value);
        
        /* Inline functions */
        
        inline std::vector<video::Texture*> getTextureList() const
        {
            return Textures_.List;
        }
        inline std::vector<audio::Sound*> getSoundList() const
        {
            return Sounds_.List;
        }
        inline std::vector<scene::SceneNode*> getNodeList() const
        {
            return Nodes_.List;
        }
        inline std::vector<scene::Mesh*> getMeshList() const
        {
            return Meshes_.List;
        }
        inline std::vector<scene::Camera*> getCameraList() const
        {
            return Cameras_.List;
        }
        inline std::vector<scene::Light*> getLightList() const
        {
            return Lights_.List;
        }
        inline std::vector<scene::Billboard*> getBillboardList() const
        {
            return Billboards_.List;
        }
        
        virtual void clearLists();
        
    protected:
        
        /* === Enumerations === */
        
        enum ECommandTypes
        {
            CMD_NONE    = 0x00, // no command
            CMD_DIRECT  = 0x01, // import "other_script_file.txt"
            CMD_MACRO   = 0x02, // uniform foobar;
            CMD_FORLOOP = 0x04, // for i = 1 to 10
            CMD_BLOCK   = 0x08, // mesh: { ...
            CMD_ACCESS  = 0x10, // mesh("obj")->pos = vec(0.0);
            CMD_ALL     = CMD_DIRECT | CMD_MACRO | CMD_FORLOOP | CMD_BLOCK | CMD_ACCESS
        };
        
        /* === Structures === */
        
        struct SCommand
        {
            SCommand() : Type(CMD_NONE)
            {
            }
            ~SCommand()
            {
            }
            
            ECommandTypes Type;
            io::stringc Name;
        };
        
        struct SVector
        {
            dim::point2df vec2;
            dim::vector3df vec3;
            dim::vector4df vec4;
            video::color clr;
        };
        
        struct SParameter
        {
            SParameter() : Used(false)
            {
            }
            ~SParameter()
            {
            }
            
            bool Used;
            s32 Components;
            io::stringc Name;
            io::stringc StrValue;
            SVector VecValue;
            s32 IntValue;
            f32 FltValue;
            bool BitValue;
        };
        
        struct SAnimationFrame
        {
            dim::vector3df Pos, Scl;
            dim::quaternion Rot;
            f32 Speed;
        };
        
        struct SAnimation
        {
            bool Used;
            bool Splines;
            io::stringc Play;
            f32 Speed;
            f32 SplineExpansion;
            std::list<SAnimationFrame> Frames;
        };
        
        struct SLoopRange
        {
            u32 FilePos, FileLine;
            u32 Index, MaxIndex;
        };
        
        template <class T> struct SContainer
        {
            s32 EnumIndex;
            std::vector<T*> List;
            std::map<io::stringc, T*> Map;
        };
        
        /* === Functions === */
        
        bool readScript();
        
        io::stringc readNextLine();
        bool checkEOF() const;
        bool readLine();
        
        void cropLineInside(s32 BeginPos, s32 EndPos);
        void cropLineOutside(s32 BeginPos, s32 EndPos);
        
        s32 readString(io::stringc &Str, s32 BeginPos = 0) const;
        
        SCommand readNextCommand(s32 AllowedCommands = CMD_ALL) const;
        bool readNextParameter(SParameter &Param, s32 BeginPos = 0);
        bool readNextParameterValue(SParameter &Param, io::stringc &CurLineContext);
        
        io::stringc getLinesIndicate() const;
        void printErrorLI(const io::stringc &Message) const;
        void printWarningLI(const io::stringc &Message) const;
        
        bool readCommandDirect();
        bool readCommandMacro();
        bool readCommandForLoop();
        bool readCommandBlock();
        bool readCommandAccess();
        
        bool isParam(const io::stringc &Name);
        
        virtual bool examineBlockNode();
        virtual bool examineBlockMesh();
        virtual bool examineBlockLight();
        virtual bool examineBlockCamera();
        virtual bool examineBlockBillboard();
        virtual bool examineBlockTerrain();
        virtual bool examineBlockTexture();
        virtual bool examineBlockSound();
        virtual bool examineBlockShader();
        virtual bool examineBlockAnim();
        virtual bool examineBlockFrame();
        virtual bool examineBlockSurface();
        virtual bool examineBlockVertex();
        virtual bool examineBlockTriangle();
        
        bool examineBlockNode(scene::SceneNode* Obj);
        bool examineBlockMaterialNode(scene::MaterialNode* Obj);
        
        void applyAnimation(scene::SceneNode* Obj);
        void applySurfaces(scene::Mesh* Obj);
        
        /* === Members === */
        
        io::FileSystem* FileSys_;
        io::File* File_;
        
        // Data container
        SContainer<video::Texture>      Textures_;
        SContainer<audio::Sound>        Sounds_;
        SContainer<scene::SceneNode>    Nodes_;
        SContainer<scene::Mesh>         Meshes_;
        SContainer<scene::Camera>       Cameras_;
        SContainer<scene::Light>        Lights_;
        SContainer<scene::Billboard>    Billboards_;
        
        std::map<std::string, io::stringc> UniformMap_;
        std::map<std::string, SParameter> VariableMap_;
        std::map<std::string, bool> InternalUniformMap_;
        
        // Current states
        io::stringc Line_, Trimed_;
        u32 CurLineNr_;
        
        std::map<std::string, SLoopRange> LoopMap_;
        
        // Parameter members
        SCommand CurCommand_;
        std::map<std::string, SParameter> ParamMap_;
        SParameter CurParam_;
        
        std::map<std::string, scene::SceneNode*> NodeMap_;
        std::map<std::string, video::Texture*> TextureMap_;
        
        // Global script members
        io::stringc WorkDir_;
        
        SAnimation CurAnim_;
        
        video::MeshBuffer* CurSurface_;
        std::list<video::MeshBuffer*> CurSurfaceList_;
        
};


} // /namespace tool

} // /namespace sp


#endif

#endif



// ================================================================================
