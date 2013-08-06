/*
 * Mesh loader OBJ file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "FileFormats/Mesh/spMeshLoaderOBJ.hpp"

#ifdef SP_COMPILE_WITH_MESHLOADER_OBJ


#include "Platform/spSoftPixelDeviceOS.hpp"

#include <boost/foreach.hpp>


namespace sp
{

extern video::RenderSystem* GlbRenderSys;

namespace scene
{


MeshLoaderOBJ::MeshLoaderOBJ() :
    MeshLoader  (   ),
    CurLineNr_  (0  ),
    TokenFlags_ (0  ),
    CurGroup_   (0  ),
    CurMaterial_(0  )
{
}
MeshLoaderOBJ::~MeshLoaderOBJ()
{
}

Mesh* MeshLoaderOBJ::loadMesh(const io::stringc &Filename, const io::stringc &TexturePath)
{
    if (!openLoadFile(Filename, TexturePath))
        return Mesh_;
    
    if (!parseFile(File_))
        io::Log::error("Loading OBJ mesh failed");
    
    if (!buildModel())
        io::Log::error("Building OBJ mesh failed");
    
    return Mesh_;
}


/*
 * ======= Private: =======
 */

bool MeshLoaderOBJ::parseFile(io::File* CurFile)
{
    /* Loop while reading the file */
    while (!CurFile->isEOF())
    {
        /* Read and parse next line */
        Line_ = CurFile->readString();
        ++CurLineNr_;
        
        if (!parseCurrentLineInFile())
        {
            FileSys_.closeFile(CurFile);
            return false;
        }
    }
    
    FileSys_.closeFile(CurFile);
    
    return true;
}

void MeshLoaderOBJ::createNewGroup(const io::stringc &Name)
{
    GroupList_.resize(GroupList_.size() + 1);
    
    CurGroup_ = &(GroupList_.back());
    CurGroup_->Name = Name;
}

void MeshLoaderOBJ::createNewMaterial(const io::stringc &Name)
{
    CurMaterial_ = &(Materials_[Name.str()]);
    CurMaterial_->Name = Name;
}

void MeshLoaderOBJ::checkGroupExistence()
{
    if (!CurGroup_)
    {
        createNewGroup("");
        io::Log::warning("Group is missing and has been created automatically");
    }
}

bool MeshLoaderOBJ::buildModel()
{
    /* Get vertex element count for faster access */
    const u32 CoordCount    = VertexCoords_.size();
    const u32 TexCoordCount = VertexTexCoords_.size();
    const u32 NormalCount   = VertexNormals_.size();
    
    u32 IndexOffset, j;
    video::color DiffuseColor;
    
    foreach (const SGroupOBJ& Group, GroupList_)
    {
        /* Create new surface */
        video::MeshBuffer* Surface = Mesh_->createMeshBuffer();
        
        Surface->setName(Group.Name);
        
        IndexOffset     = 0;
        DiffuseColor    = video::color(255);
        
        /* Apply material */
        if (Group.Material)
        {
            /* Apply colors */
            DiffuseColor = Group.Material->Diffuse;
            
            /* Add texture */
            if (Group.Material->ColorMap)
                Surface->addTexture(Group.Material->ColorMap);
        }
        
        /* Create faces */
        foreach (const SFaceOBJ& Face, Group.Faces)
        {
            /* Create vertices */
            foreach (u32 i, Face.CoordIndices)
            {
                if (i >= CoordCount)
                    return exitWithError("Invalid index for vertex coordiante", false);
                Surface->addVertex(VertexCoords_[i], 0, DiffuseColor);
            }
            
            j = 0;
            foreach (u32 i, Face.TexCoordIndices)
            {
                if (i >= TexCoordCount)
                    return exitWithError("Invalid index for texture coordiante", false);
                Surface->setVertexTexCoord(IndexOffset + j++, VertexTexCoords_[i]);
            }
            
            j = 0;
            foreach (u32 i, Face.NormalIndices)
            {
                if (i >= NormalCount)
                    return exitWithError("Invalid index for vertex normal", false);
                Surface->setVertexNormal(IndexOffset + j++, VertexNormals_[i]);
            }
            
            /* Create triangles */
            for (u32 i = 0; i < Face.CoordIndices.size() - 2; ++i)
                Surface->addTriangle(IndexOffset, IndexOffset + i + 1, IndexOffset + i + 2);
            
            IndexOffset += Face.CoordIndices.size();
        }
    }
    
    Mesh_->updateMeshBuffer();
    
    return true;
}

bool MeshLoaderOBJ::getNextToken()
{
    if (!Line_.size())
        return false;
    
    TokenFlags_ = 0;
    
    s32 First = -1, Last = -1, i;
    
    /* Search the string for the next token */
    for (i = 0; i < static_cast<s32>(Line_.size()); ++i)
    {
        const c8 Chr = Line_[i];
        
        /* '/' is a standalone token */
        if (Chr == '/')
        {
            if (First == -1)
            {
                First = i;
                Last = i + 1;
                break;
            }
            else
            {
                Last = i;
                break;
            }
        }
        /* Search for string or number */
        else if (Chr != ' ' && Chr != '\t' && Chr != '\n')
        {
            if (First == -1)
                First = i;
        }
        else if (First != -1)
        {
            Last = i;
            TokenFlags_ |= TOKENFLAG_UNTIL_BLANK;
            break;
        }
    }
    
    if (First == -1)
        return false;
    if (Last == -1)
        Last = i;
    
    /* Get token string part and cut old string */
    Token_ = Line_.section(First, Last);
    
    Line_ = Line_.right(Line_.size() - Last);
    
    return true;
}

bool MeshLoaderOBJ::parseCurrentLineInFile()
{
    if (!getNextToken())
        return true;
    
    if (Token_ == "#")
        return true;
    else if (Token_ == "v")
        return parseVertexCoord();
    else if (Token_ == "vt")
        return parseVertexTexCoord();
    else if (Token_ == "vn")
        return parseVertexNormal();
    else if (Token_ == "f")
        return parseFace();
    else if (Token_ == "g")
        return parseGroup();
    else if (Token_ == "usemtl")
        return parseUseMaterial();
    else if (Token_ == "mtllib")
        return parseMaterialLibrary();
    else if (Token_ == "newmtl")
        return parseNewMaterial();
    else if (Token_ == "map_Kd")
        return parseTexture();
    else if (Token_ == "Kd")
        return parseColorDiffuse();
    else if (Token_ == "o")
        return parseObject();
    
    return true;
}

bool MeshLoaderOBJ::exitWithError(const io::stringc &Message, bool PrintLineNr)
{
    if (PrintLineNr)
        io::Log::error("[Line #" + io::stringc(CurLineNr_) + "]: " + Message);
    else
        io::Log::error(Message);
    return false;
}

bool MeshLoaderOBJ::parseVertexCoord()
{
    s32 Count = 0;
    dim::vector3df Coord;
    
    while (getNextToken())
    {
        if (Count >= 3)
            break;
        Coord[Count++] = Token_.val<f32>();
    }
    
    VertexCoords_.push_back(Coord);
    
    return true;
}

bool MeshLoaderOBJ::parseVertexTexCoord()
{
    s32 Count = 0;
    dim::point2df TexCoord;
    
    while (getNextToken())
    {
        if (Count >= 2)
            break;
        TexCoord[Count++] = Token_.val<f32>();
    }
    
    TexCoord.Y = -TexCoord.Y;
    VertexTexCoords_.push_back(TexCoord);
    
    return true;
}

bool MeshLoaderOBJ::parseVertexNormal()
{
    s32 Count = 0;
    dim::vector3df Normal;
    
    while (getNextToken())
    {
        if (Count >= 3)
            break;
        Normal[Count++] = Token_.val<f32>();
    }
    
    VertexNormals_.push_back(Normal);
    
    return true;
}

bool MeshLoaderOBJ::parseFace()
{
    checkGroupExistence();
    
    CurGroup_->Faces.resize(CurGroup_->Faces.size() + 1);
    SFaceOBJ* Face = &(CurGroup_->Faces.back());
    
    s32 Type = 0;
    
    while (getNextToken())
    {
        if (Token_ == "/")
        {
            ++Type;
            continue;
        }
        
        switch (Type)
        {
            case 0:
                Face->CoordIndices.push_back(Token_.val<u32>() - 1);
                break;
            case 1:
                Face->TexCoordIndices.push_back(Token_.val<u32>() - 1);
                break;
            case 2:
                Face->NormalIndices.push_back(Token_.val<u32>() - 1);
                break;
        }
        
        if (TokenFlags_ & TOKENFLAG_UNTIL_BLANK)
            Type = 0;
    }
    
    return true;
}

bool MeshLoaderOBJ::parseGroup()
{
    if (!getNextToken())
        return exitWithError("Group defined but name is missing");
    
    createNewGroup(Token_);
    
    return true;
}

bool MeshLoaderOBJ::parseUseMaterial()
{
    checkGroupExistence();
    
    if (!getNextToken())
        return exitWithError("Material used but name is missing");
    
    MaterialType::iterator it = Materials_.find(Token_.str());
    
    if (it != Materials_.end())
        CurGroup_->Material = &(it->second);
    else
        io::Log::warning("Could not find material \"" + Token_ + "\"");
    
    return true;
}

bool MeshLoaderOBJ::parseMaterialLibrary()
{
    if (!Line_.trim().size())
        return exitWithError("Material library defined but filename is missing");
    
    Token_ = Line_.ltrim();
    
    const io::stringc Filename(Filename_.getPathPart() + Token_);
    
    io::Log::message("Load material \"" + Filename + "\"");
    io::Log::upperTab();
    
    io::File* MatFile = FileSys_.openFile(Filename, io::FILE_READ);
    
    if (!MatFile)
    {
        io::Log::lowerTab();
        return true;
    }
    
    if (!parseFile(MatFile))
    {
        FileSys_.closeFile(MatFile);
        
        io::Log::error("Loading OBJ material failed");
        io::Log::lowerTab();
        
        return false;
    }
    
    FileSys_.closeFile(MatFile);
    
    io::Log::lowerTab();
    
    return true;
}

bool MeshLoaderOBJ::parseNewMaterial()
{
    if (!getNextToken())
        return exitWithError("Material created but name is missing");
    
    createNewMaterial(Token_);
    
    return true;
}

bool MeshLoaderOBJ::parseTexture()
{
    if (!CurMaterial_)
        return exitWithError("Texture defined without material");
    if (!getNextToken())
        return exitWithError("Texture defined but filename is missing");
    
    CurMaterial_->ColorMap = GlbRenderSys->loadTexture(TexturePath_ + Token_);
    
    return true;
}

bool MeshLoaderOBJ::parseColorDiffuse()
{
    if (!CurMaterial_)
        return exitWithError("Diffuse color defined without material");
    
    s32 Count = 0;
    dim::vector3df Color;
    
    while (getNextToken())
    {
        if (Count >= 3)
            break;
        Color[Count++] = Token_.val<f32>();
    }
    
    CurMaterial_->Diffuse = video::color(Color);
    
    return true;
}

bool MeshLoaderOBJ::parseObject()
{
    if (!getNextToken())
        return exitWithError("Object defined but name is missing");
    
    Mesh_->setName(Token_);
    
    return true;
}


} // /namespace scene

} // /namespace sp


#endif



// ================================================================================
