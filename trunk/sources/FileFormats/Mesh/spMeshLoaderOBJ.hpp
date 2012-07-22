/*
 * Mesh loader OBJ header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_MESHLOADER_OBJ_H__
#define __SP_MESHLOADER_OBJ_H__


#include "Base/spStandard.hpp"

#ifdef SP_COMPILE_WITH_MESHLOADER_OBJ


#include "Base/spInputOutputLog.hpp"
#include "Base/spInputOutputFileSystem.hpp"
#include "Base/spDimension.hpp"
#include "FileFormats/Mesh/spMeshLoader.hpp"

#include <vector>


namespace sp
{
namespace scene
{


class SP_EXPORT MeshLoaderOBJ : public MeshLoader
{
    
    public:
        
        MeshLoaderOBJ();
        ~MeshLoaderOBJ();
        
        Mesh* loadMesh(const io::stringc &Filename, const io::stringc &TexturePath);
        
    private:
        
        /* === Enumerations === */
        
        enum ETokenFlags
        {
            TOKENFLAG_UNTIL_BLANK = 0x01,
        };
        
        /* === Structures === */
        
        struct SFaceOBJ
        {
            std::vector<u32> CoordIndices;
            std::vector<u32> TexCoordIndices;
            std::vector<u32> NormalIndices;
        };
        
        struct SMaterialOBJ
        {
            SMaterialOBJ() : ColorMap(0)
            {
            }
            ~SMaterialOBJ()
            {
            }
            
            /* Members */
            io::stringc Name;
            video::color Diffuse;
            video::color Ambient;
            video::color Specular;
            video::color Emission;
            video::color VertexColor;
            video::Texture* ColorMap;
        };
        
        struct SGroupOBJ
        {
            SGroupOBJ() : Material(0)
            {
            }
            ~SGroupOBJ()
            {
            }
            
            /* Members */
            io::stringc Name;
            SMaterialOBJ* Material;
            std::list<SFaceOBJ> Faces;
        };
        
        /* === Type definitions === */
        
        typedef std::map<std::string, SMaterialOBJ> MaterialType;
        
        /* === Functions === */
        
        bool parseFile(io::File* CurFile);
        bool buildModel();
        
        void createNewGroup(const io::stringc &Name);
        void createNewMaterial(const io::stringc &Name);
        void checkGroupExistence();
        
        bool exitWithError(const io::stringc &Message, bool PrintLineNr = true);
        
        bool getNextToken();
        bool parseCurrentLineInFile();
        
        bool parseVertexCoord();
        bool parseVertexTexCoord();
        bool parseVertexNormal();
        bool parseFace();
        bool parseGroup();
        bool parseUseMaterial();
        bool parseMaterialLibrary();
        bool parseNewMaterial();
        bool parseTexture();
        bool parseColorDiffuse();
        bool parseObject();
        
        /* === Members === */
        
        io::stringc Line_, Token_;
        u32 CurLineNr_;
        
        s32 TokenFlags_;
        
        std::vector<dim::vector3df> VertexCoords_;
        std::vector<dim::point2df> VertexTexCoords_;
        std::vector<dim::vector3df> VertexNormals_;
        
        SGroupOBJ* CurGroup_;
        std::list<SGroupOBJ> GroupList_;
        
        SMaterialOBJ* CurMaterial_;
        MaterialType Materials_;
        
};


} // /namespace scene

} // /namespace sp


#endif

#endif



// ================================================================================
