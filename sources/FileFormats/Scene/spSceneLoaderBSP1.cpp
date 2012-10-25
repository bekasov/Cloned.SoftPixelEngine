/*
 * Quake 1 BSP loader file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "FileFormats/Scene/spSceneLoaderBSP1.hpp"

#ifdef SP_COMPILE_WITH_SCENELOADER_BSP1


#include "FileFormats/Scene/spSceneLoaderBSP3.hpp"
#include "Platform/spSoftPixelDeviceOS.hpp"


namespace sp
{

extern video::RenderSystem* __spVideoDriver;
extern scene::SceneGraph* __spSceneManager;

namespace scene
{


SceneLoaderBSP1::SceneLoaderBSP1() :
    SceneLoader (   ),
    EntDesc_    (0  ),
    LightMap_   (0  )
{
}
SceneLoaderBSP1::~SceneLoaderBSP1()
{
    MemoryManager::deleteBuffer(LightMap_);
}

Mesh* SceneLoaderBSP1::loadScene(const io::stringc &Filename, const io::stringc &TexturePath, const s32 Flags)
{
    /* First settings */
    Filename_       = Filename;
    TexturePath_    = TexturePath;
    
    /* Create an empty mesh */
    MeshBase_       = new Mesh();
    
    /* Create an empty mesh for transparent textures */
    MeshTrans_      = __spSceneManager->createMesh();
    MeshTrans_->getMaterial()->setBlending(false);
    MeshTrans_->getMaterial()->setAlphaMethod(video::CMPSIZE_GREATER, 0.5);
    //MeshTrans_->setRenderFace(video::RENDER_BOTH);
    MeshTrans_->setParent(MeshBase_);
    
    /* Check if the file has been opened successful */
    if (!openForWriting())
        return MeshBase_;
    
    /* Read the header */
    readHeader();
    
    /* Read the lumps */
    readLumps();
    
    /* Close the file */
    FileSys_.closeFile(File_);
    
    /* Build the model */
    buildModel();
    
    /* Return the mesh */
    return MeshBase_;
    
}

bool SceneLoaderBSP1::readHeader()
{
    
    /* Read the header */
    File_->readBuffer(&Header_, sizeof(Header_));
    
    /* Check the version (0x17: Quake1/ 0x1E: Half-Life 1) */
    if (Header_.Version != 0x17 && Header_.Version != 0x1E)
    {
        /* Print an error message */
        io::Log::error("BSP (Quake/ Half-Life) file has incorrect version number");
        
        /* Exit the function and return false for a failure */
        return false;
    }
    
    #ifdef DEBUG_BSP1
    debHeader();
    #endif
    
    /* Exit the function and return true for success */
    return true;
    
}

void SceneLoaderBSP1::readLumps()
{
    
    /* Read all lumps */
    readLumpEntities();
    readLumpPlanes();
    readLumpWallTextures();
    readLumpVertices();
    readLumpVisData();
    readLumpNodes();
    readLumpTexInfo();
    readLumpLightMaps();
    readLumpClipNodes();
    readLumpLeafs();
    readLumpFacesList();
    readLumpEdges();
    readLumpEdgesList();
    readLumpFaces();
    readLumpModels();
    
    #ifdef DEBUG_BSP1
    debProtocol();
    #endif
    
}

void SceneLoaderBSP1::readLumpEntities()
{
    
    /* Temporary vairables */
    std::vector<io::stringc> ScriptData;
    
    /* Set the offset */
    File_->setSeek(Header_.DirEntries[BSP_LUMP_ENTITIES].Offset);
    
    /* Get the size of the string */
    s32 size = Header_.DirEntries[BSP_LUMP_ENTITIES].Length;
    
    /* Allocate new memory for the string */
    EntDesc_ = new c8[size];
    
    /* Read the string data */
    File_->readBuffer(EntDesc_, size);
    
    /* Create the script */
    BSPLoaderExtensions::createScript(ScriptData, EntDesc_);
    
    /* Analyse the script */
    examineScript(ScriptData);
    
    #if defined(DEBUG_BSP1) && 1
    std::ofstream deb2("DEBUG_BSP1_SCRIPT.log");
    deb2 << EntDesc_;
    deb2.close();
    #endif
    
    /* Delete the script RAW data */
    MemoryManager::deleteBuffer(EntDesc_);
    
}

void SceneLoaderBSP1::readLumpPlanes()
{
    
    /* Temporary variables */
    const s32 ChunkSize     = sizeof(SPlaneBSP);
    const s32 ChunkCount    = Header_.DirEntries[BSP_LUMP_PLANES].Length / ChunkSize;
    
    SPlaneBSP ChunkData;
    
    /* Set the offset */
    File_->setSeek(Header_.DirEntries[BSP_LUMP_PLANES].Offset);
    
    /* Loop for all chunks */
    for (s32 i = 0; i < ChunkCount; ++i)
    {
        
        /* Read the vertex data */
        File_->readBuffer(&ChunkData, sizeof(ChunkData));
        
        /* Swap the normal */
        swapVector(ChunkData.Normal);
        
        /* Add the plane to the list */
        PlaneList_.push_back(ChunkData);
        
    } // next chunk
    
}

void SceneLoaderBSP1::readLumpWallTextures()
{
    
    /* Temporary variables */
    std::vector<s32> OffsetList;
    SWallTextureBSP ChunkData;
    
    video::Texture* texDefault = 0;
    
    std::vector<io::stringc> TextureNameList;
    
    bool hasFound = false;
    
    /* Set the offset */
    File_->setSeek(Header_.DirEntries[BSP_LUMP_WALLTEXTURES].Offset);
    
    /* Read the count of textures */
    s32 CountTextures = File_->readValue<s32>();
    
    /* Read all offsets */
    for (s32 i = 0; i < CountTextures; ++i)
        OffsetList.push_back(File_->readValue<s32>());
    
    /* Loop for all chunks */
    for (s32 i = 0; i < CountTextures; ++i)
    {
        
        /* Check if the texture is valid */
        if (OffsetList[i] == -1)
            continue;
        
        /* Set the offset */
        File_->setSeek(Header_.DirEntries[BSP_LUMP_WALLTEXTURES].Offset + OffsetList[i]);
        
        /* Read the texture data */
        File_->readBuffer(&ChunkData, sizeof(ChunkData));
        
        /* Add the texture to the list */
        WallTextureList_.push_back(ChunkData);
        
        /* Get the name */
        c8 Name[17] = { 0 };
        memcpy(Name, ChunkData.Name, 16);
        
        /* Add the texture name to the list */
        TextureNameList.push_back( io::stringc(Name) );
        
        #ifdef COLOR_NO_TEXTURE
        q1ColorList.push_back(video::color(math::Rnd(255), math::Rnd(255), math::Rnd(255)));
        #endif
        #ifdef DEBUG_BSP1
        debWallTexture(ChunkData);
        #endif
        
    } // next chunk
    
    /* Read the RAW texture data (WAD files) */
    readTexturesRAW();
    
    /* Search the texture by the name */
    for (std::vector<io::stringc>::iterator it = TextureNameList.begin(); it != TextureNameList.end(); ++it)
    {
        /* Reset the 'found' flag */
        hasFound = false;
        
        /* Loop for all loaded RAW textures */
        for (std::list<video::Texture*>::iterator itTex = TextureListRAW_.begin(); itTex != TextureListRAW_.end(); ++itTex)
        {
            if ((*itTex)->getFilename() == *it)
            {
                /* Add the texture to the final list an exit the loop */
                TextureList_.push_back(*itTex);
                hasFound = true;
                break;
            }
        } // next RAW texture
        
        /* Check if a texture has been found */
        if (!hasFound)
        {
            /* Check if the RAW texture list is not empty */
            if (TextureListRAW_.size())
            {
                /* Add the first texture to the list */
                TextureList_.push_back(TextureListRAW_.front());
            }
            else
            {
                /* Create one times the default texture */
                if (!texDefault)
                    texDefault = __spVideoDriver->createTexture(dim::size2di(8));
                
                /* Add the default texture to the list */
                TextureList_.push_back(texDefault);
            }
        }
    } // next texture name
    
}

void SceneLoaderBSP1::readLumpVertices()
{
    
    /* Temporary variables */
    const s32 ChunkSize     = sizeof(dim::vector3df);
    const s32 ChunkCount    = Header_.DirEntries[BSP_LUMP_VERTICES].Length / ChunkSize;
    
    dim::vector3df Vertex;
    
    /* Set the offset */
    File_->setSeek(Header_.DirEntries[BSP_LUMP_VERTICES].Offset);
    
    /* Loop for all chunks */
    for (s32 i = 0; i < ChunkCount; ++i)
    {
        
        /* Read the vertex data */
        File_->readBuffer(&Vertex, sizeof(Vertex));
        
        /* Swap to the left-handed coordinat system */
        swapVector(Vertex);
        
        /* Add the vertex to the list */
        VerticesList_.push_back(Vertex);
        
    } // next chunk
    
    #ifdef QUAKE1DRAWRAW
    q1VerticesList = VerticesList_;
    #endif
    
}

void SceneLoaderBSP1::readLumpVisData()
{
    
}

void SceneLoaderBSP1::readLumpNodes()
{
    
}

void SceneLoaderBSP1::readLumpTexInfo()
{
    
    /* Temporary variables */
    const s32 ChunkSize     = sizeof(STextureInfoBSP);
    const s32 ChunkCount    = Header_.DirEntries[BSP_LUMP_TEXINFO].Length / ChunkSize;
    
    STextureInfoBSP ChunkData;
    
    /* Set the offset */
    File_->setSeek(Header_.DirEntries[BSP_LUMP_TEXINFO].Offset);
    
    /* Loop for all chunks */
    for (s32 i = 0; i < ChunkCount; ++i)
    {
        
        /* Read the texture info data */
        File_->readBuffer(&ChunkData, sizeof(ChunkData));
        
        /* Add the texture info to the list */
        TextureInfoList_.push_back(ChunkData);
        
    } // next chunk
    
}

void SceneLoaderBSP1::readLumpFaces()
{
    
    /* Temporary variables */
    const s32 ChunkSize     = sizeof(SFaceBSP);
    const s32 ChunkCount    = Header_.DirEntries[BSP_LUMP_FACES].Length / ChunkSize;
    
    SFaceBSP ChunkData;
    
    s32 i, j;
    
    std::vector<s16> EdgesIndicesList;
    std::vector<SEdgeBSP> PairedEdges;
    std::vector<s16> Indices;
    
    /* Set the offset */
    File_->setSeek(Header_.DirEntries[BSP_LUMP_FACES].Offset);
    
    /* Loop for all chunks */
    for (i = 0; i < ChunkCount; ++i)
    {
        
        /* Read the vertex data */
        File_->readBuffer(&ChunkData, sizeof(ChunkData));
        
        /* Add the face to the list */
        FacesList_.push_back(ChunkData);
        
        /* Set the current face */
        CurFace_ = ChunkData;
        
        /* Set the current texture information */
        CurTexInfo_ = TextureInfoList_[ChunkData.TexInfoID];
        
        /* Full the edge-indices-list */
        for (j = 0; j < ChunkData.CountEdges; ++j)
            EdgesIndicesList.push_back(EdgesIndicesList_[ChunkData.FirstEdge + j]);
        
        /* Pair the edges */
        PairedEdges = pairEdges(EdgesIndicesList);
        
        /* Pair the triangles */
        Indices = pairTriangles(PairedEdges);
        
        /* Create the polyong (the triangle list) */
        createPolygon(Indices);
        
        /* Clear the lists */
        EdgesIndicesList.clear();
        PairedEdges.clear();
        Indices.clear();
        
    } // next chunk
    
}

void SceneLoaderBSP1::readLumpLightMaps()
{
    
    #if 0
    
    /* Temporary variables */
    const s32 size = Header_.DirEntries[BSP_LUMP_LIGHTMAPS].Length;
    
    /* Set the offset */
    File_->setSeek(Header_.DirEntries[BSP_LUMP_LIGHTMAPS].Offset);
    
    /* Allocate new lightmap raw data */
    LightMap_ = new u8[size];
    
    /* Read the light map raw data */
    File_->readBuffer(LightMap_, size);
    
    while (spDevice->updateEvent() && !spControl->isKeyHit(KEY_RETURN))
    {
        spDriver->clearBuffers();
        spDriver->beginDrawing2D();
        
        spDriver->draw2DRectangle(dim::rect2di(0, 0, 800, 600));
        
        for (s32 x, y = 0, i = 0, s = 2; i < size; ++y)
        {
            for (x = 0; x < 256 && i < size; ++x, ++i)
            {
                spDriver->draw2DRectangle(
                    dim::rect2di(x*s, y*s, (x+1)*s, (y+1)*s),
                    video::color(LightMap_[i])
                );
            }
        }
        
        spDriver->endDrawing2D();
        spDriver->flipBuffers();
    }
    
    #endif
    
}

void SceneLoaderBSP1::readLumpClipNodes()
{
    
}

void SceneLoaderBSP1::readLumpLeafs()
{
    
}

void SceneLoaderBSP1::readLumpFacesList()
{
    
}

void SceneLoaderBSP1::readLumpEdges()
{
    
    /* Temporary variables */
    const s32 ChunkSize     = sizeof(SEdgeBSP);
    const s32 ChunkCount    = Header_.DirEntries[BSP_LUMP_EDGES].Length / ChunkSize;
    
    SEdgeBSP ChunkData;
    
    /* Set the offset */
    File_->setSeek(Header_.DirEntries[BSP_LUMP_EDGES].Offset);
    
    /* Loop for all chunks */
    for (s32 i = 0; i < ChunkCount; ++i)
    {
        
        /* Read the vertex data */
        File_->readBuffer(&ChunkData, sizeof(ChunkData));
        
        /* Add the edge to the list */
        EdgesList_.push_back(ChunkData);
        
    } // next chunk
    
    #ifdef QUAKE1DRAWRAW
    q1EdgesList.resize(EdgesList_.size());
    for (s32 i = 0; i < EdgesList_.size(); ++i)
        q1EdgesList[i].i1 = EdgesList_[i].VertexStart, q1EdgesList[i].i2 = EdgesList_[i].VertexEnd;
    #endif
    
}

void SceneLoaderBSP1::readLumpEdgesList()
{
    
    /* Temporary variables */
    const s32 ChunkSize     = sizeof(s16);
    const s32 ChunkCount    = Header_.DirEntries[BSP_LUMP_EDGESLIST].Length / ChunkSize;
    
    s16 Index;
    
    /* Set the offset */
    File_->setSeek(Header_.DirEntries[BSP_LUMP_EDGESLIST].Offset);
    
    /* Loop for all chunks */
    for (s32 i = 0; i < ChunkCount; ++i)
    {
        
        /* Read the vertex data */
        File_->readBuffer(&Index, sizeof(Index));
        
        /* Add the edge to the list */
        if (Index > 1 || Index < -1)
            EdgesIndicesList_.push_back(Index);
        
    } // next chunk
    
}

void SceneLoaderBSP1::readLumpModels()
{
    
}

/*
 * The "readTexturesRAW" function tries to find all textures which this map need
 * It loads the texture files via the WAD loader class
 */

void SceneLoaderBSP1::readTexturesRAW()
{
    #ifdef SP_COMPILE_WITH_TEXLOADER_WAD
    /* Temporary variables */
    std::list<video::Texture*> TempList;
    
    io::stringc CurFileName;
    s32 CurLen;
    
    /* Allocate a new WAD loader */
    video::ImageLoaderWAD* WADLoader = new video::ImageLoaderWAD();
    
    /* Check if the texture-list has been declared in the texture-path */
    if (FileSys_.findFile(TexturePath_))
    {
        /* Read the WAD texture list */
        TextureListRAW_ = WADLoader->loadTextureList(TexturePath_);
        
        /* Change the texture path */
        TexturePath_ = TexturePath_.getPathPart();
    }
    
    /* Check if the script WAD list are empty */
    if (!ScriptWADList_.size())
    {
        /* Add the standard files */
        ScriptWADList_.push_back("valve/halflife.wad");
        ScriptWADList_.push_back("cstrike/cstrike.wad");
    }
    
    /* Loop for all script-loaded WAD files */
    for (std::vector<io::stringc>::iterator it = ScriptWADList_.begin(); it != ScriptWADList_.end(); ++it)
    {
        /* Replace the unused symboles */
        *it = it->replace("\\", "/");
        
        /* Reset the length */
        CurLen = 0;
        
        /* Loop while searching the correct texture list */
        while (1)
        {
            /* Find the next texture path part */
            findNextTexturePath(*it, CurLen);
            
            /* Set the current file */
            CurFileName = TexturePath_ + it->right(CurLen);
            
            /* Check if the file does exist */
            if (FileSys_.findFile(CurFileName))
            {
                /* Read the WAD texture list & exit the loop */
                TempList = WADLoader->loadTextureList(CurFileName);
                break;
            }
            
            /* Break when no file was found */
            if (CurLen >= static_cast<s32>(it->size()) - 1)
                break;
        }
        
        /* Add the loaded texture list to the main list */
        for (std::list<video::Texture*>::iterator itTex = TempList.begin(); itTex != TempList.end(); ++itTex)
            TextureListRAW_.push_back(*itTex);
    }
    
    /* Delete the WAD loader */
    MemoryManager::deleteMemory(WADLoader);
    #else
    io::printError("Cannot load textures because the engine is compiled without WAD texture loader");
    #endif
}

void SceneLoaderBSP1::findNextTexturePath(io::stringc Filename, s32 &Pos)
{
    /* Resettings */
    Pos = Filename.size() - Pos - 1;
    
    s32 cur = 0, old = 0;
    
    /* Search for the '/' charcter */
    while (cur != -1 && cur < Pos)
    {
        old = cur;
        cur = Filename.find("/", cur + 1);
    }
    
    /* Set the new position */
    if (old == -1)
        Pos = Filename.size();
    else
        Pos = Filename.size() - old - 1;
}

/*
 * The "examineScript" function analyses the whole script of the map
 * It also creates pivots for using and handling the respawn point, items etc.
 */

void SceneLoaderBSP1::examineScript(std::vector<io::stringc> &ScriptData)
{
    
    /* Temporary variables */
    io::stringc Spec, Arg;
    
    s32 pos1, pos2;
    
    SScriptBlockBSP Block;
    
    /* Loop for all lines of the script */
    for (std::vector<io::stringc>::iterator it = ScriptData.begin(); it != ScriptData.end(); ++it)
    {
        
        /* Check if a block begins */
        if (*it == "{")
        {
            Block.clear();
            continue;
        }
        
        /* Check if a block ends */
        if (*it == "}")
        {
            /* Execute the script block functions */
            executeScriptBlock(Block);
            continue;
        }
        
        /* Find the first splitting positions */
        pos1 = it->find("\"", 0) + 1;
        pos2 = it->find("\"", pos1);
        
        /* Read the specification */
        Spec = it->section(pos1, pos2);
        
        /* Find the second splitting positions */
        pos1 = it->find("\"", pos2 + 1) + 1;
        pos2 = it->find("\"", pos1);
        
        /* Read the argument */
        Arg = it->section(pos1, pos2);
        
        /* Analyse the block */
        if (Spec == "wad")
        {
            ScriptWADList_ = readScriptSpecWAD(Arg);
        }
        else if (Spec == "classname")
        {
            Block.ClassName = Arg;
        }
        else if (Spec == "origin")
        {
            Block.Origin = convertVector(Arg);
            swapVector(Block.Origin);
        }
        else if (Spec == "angles")
        {
            Block.Angles = convertVector(Arg);
        }
        else if (Spec == "_light")
        {
            Block.Color = convertColor(Arg);
        }
        
    } // next line
    
}

void SceneLoaderBSP1::executeScriptBlock(const SScriptBlockBSP &Block)
{
    scene::SceneNode* NodeObject = 0;
    
    if (Block.ClassName == "info_player_start" || Block.ClassName == "info_player_deathmatch" ||
        Block.ClassName == "light" || Block.ClassName == "trigger_camera")
    {
        NodeObject = __spSceneManager->createNode();
    }
    
    if (NodeObject)
    {
        if (Block.ClassName == "trigger_camera")
            NodeObject->setRotation(dim::vector3df(Block.Angles.X, 180 - Block.Angles.Y, Block.Angles.Z));
        
        NodeObject->setPosition(Block.Origin);
        NodeObject->setName(Block.ClassName);
        NodeObject->setParent(MeshBase_);
    }
}

dim::vector3df SceneLoaderBSP1::convertVector(io::stringc &Arg)
{
    /* Temporary variables */
    dim::vector3df Vec;
    
    s32 pos1, pos2;
    
    /* Find the splitting positions */
    pos1 = Arg.find(" ", 0);
    pos2 = Arg.find(" ", pos1 + 1);
    
    /* Set the vector coordinate */
    Vec.X = Arg.left(pos1).val<f32>();
    Vec.Y = Arg.section(pos1+1, pos2).val<f32>();
    Vec.Z = Arg.right(Arg.size() - pos2 - 1).val<f32>();
    
    /* Return the vector */
    return Vec;
}

video::color SceneLoaderBSP1::convertColor(io::stringc &Arg)
{
    /* Temporary variables */
    video::color Color;
    
    s32 pos1, pos2, pos3;
    
    /* Find the splitting positions */
    pos1 = Arg.find(" ", 0);
    pos2 = Arg.find(" ", pos1 + 1);
    pos3 = Arg.find(" ", pos2 + 1);
    
    /* Set the vector coordinate */
    Color.Alpha = Arg.left(pos1).val<u8>();
    Color.Red   = Arg.section(pos1+1, pos2).val<u8>();
    Color.Green = Arg.section(pos2+1, pos3).val<u8>();
    Color.Blue  = Arg.right(Arg.size() - pos2 - 1).val<u8>();
    
    /* Return the vector */
    return Color;
}

std::vector<io::stringc> SceneLoaderBSP1::readScriptSpecWAD(io::stringc &Arg)
{
    /* Temporary variables */
    std::vector<io::stringc> WADList;
    
    /* Check if the argument is not empty */
    if (!Arg.size())
        return WADList;
    
    s32 pos1 = 0, pos2;
    
    /* Loop while searching WAD filenames */
    while (pos1 < static_cast<s32>(Arg.size()))
    {
        /* Find the next position */
        pos2 = Arg.find(";", pos1 + 1);
        
        /* Check if the string is at the end */
        if (pos2 == -1)
            pos2 = Arg.size() - 1;
        
        /* Add the new filename to the list */
        WADList.push_back(Arg.section(pos1, pos2));
        
        /* Set the first position */
        pos1 = pos2 + 1;
    }
    
    /* Return the WAD filename list */
    return WADList;
}

/*
 * The "pairEdges" function just pairs the whole edges to have the edges in dependet of
 * the edge-indices-list for the surfaces in a new memorized list
 */

std::vector<SceneLoaderBSP1::SEdgeBSP> SceneLoaderBSP1::pairEdges(std::vector<s16> &EdgesIndicesList)
{
    /* Temporary variables */
    std::vector<SEdgeBSP> NewEdgeList;
    SEdgeBSP NewEdge;
    
    /* Loop for all new edges */
    for (std::vector<s16>::iterator it = EdgesIndicesList.begin(); it != EdgesIndicesList.end(); ++it)
    {
        /* Check the sense of the edge */
        if (*it < 0)
        {
            NewEdge = EdgesList_[-*it];
            NewEdge.swap();
            NewEdgeList.push_back(NewEdge);
        }
        else
            NewEdgeList.push_back(EdgesList_[*it]);
    } // next edge
    
    /* Return the resulting list */
    return NewEdgeList;
}

/*
 * The "pairTriangles" function pairs the indices of the new edge list for easy creating
 * the surfaces (polygons). This indices list must be formed for simple triangle-lists
 * so the function converts the triangle-fans into triangle-lists
 */

std::vector<s16> SceneLoaderBSP1::pairTriangles(std::vector<SEdgeBSP> &EdgesList)
{
    /* Temporary variables */
    std::vector<s16> Indices;
    
    /* For all triangles */
    for (u32 i = 1, r = 0; i < EdgesList.size(); ++i)
    {
        /* Check if edge is senseless (triangle with the indices a, b, a) */
        if (i != r && EdgesList[r].VertexStart == EdgesList[i].VertexEnd)
            continue;
        
        /* Add the three indices for the triangle */
        Indices.push_back(EdgesList[r].VertexStart);
        Indices.push_back(EdgesList[i].VertexStart);
        Indices.push_back(EdgesList[i].VertexEnd);
        
        /* Check if the root index for the triangle-fan has been changed */
        if (i < (EdgesList.size() - 1) && EdgesList[i].VertexEnd != EdgesList[i + 1].VertexStart)
            r = ++i;
    } // next edge
    
    /* Return the resulting indices */
    return Indices;
}

/*
 * The "createPolygon" function creates a new indices list with the smaller indices
 * because the whole vertices list bust be devided into the count of surfaces (polygons)
 * so this function changes the index values to the new "SoftPixel Engine"- vertex indcies
 * to create the triangles correctly. The function also creates the polygon in a form
 * of one triangle-list
 */

void SceneLoaderBSP1::createPolygon(std::vector<s16> &IndicesList)
{
    /* Check if the indices lis is empty */
    if (!IndicesList.size())
        return;
    
    /* Temporary variables */
    std::list<s16> UniqueList;
    std::vector<s16> FullList = IndicesList;
    
    dim::vector3df VertexPos;
    dim::point2df TexCoord;
    
    s16 NewIndex;
    u32 i, j;
    
    video::Texture* Tex = (
        (CurTexInfo_.TextureID < TextureList_.size()) ? MemoryManager::getElement(TextureList_, CurTexInfo_.TextureID) : 0
    );

    /* Determine which mesh must be used */
    if (Tex && CurTexInfo_.TextureID < TextureList_.size() && Tex->getFormat() == video::PIXELFORMAT_RGBA)
        Mesh_ = MeshTrans_;
    else
        Mesh_ = MeshBase_;
    
    /* Create a new surface */
    Surface_ = Mesh_->createMeshBuffer();
    
    /* Add the texture */
    if (Tex)
        Surface_->addTexture(Tex);
    
    /* Fill the new lists */
    for (i = 0; i < IndicesList.size(); ++i)
        UniqueList.push_back(IndicesList[i]);
    
    /* Make the new indices list unique */
    UniqueList.unique();
    
    //#define COLOR_PAINT
    #ifdef COLOR_PAINT
    #   ifdef COLOR_NO_TEXTURE
    video::color c( q1ColorList[ TextureInfoList_[ CurFace_.TexInfoID ].TextureID ] );
    #   else
    video::color c( video::color(math::Rnd(255), math::Rnd(255), math::Rnd(255)) );
    #   endif
    #endif
    
    /* Loop for all vertices */
    for (std::list<s16>::iterator it = UniqueList.begin(); it != UniqueList.end(); ++it)
    {
        
        /* Get the vertex coordiante */
        VertexPos = VerticesList_[*it];
        
        /* Calculate the texture coordinate */
        TexCoord = calcTexCoord(VertexPos, CurTexInfo_);
        
        /* Add a new vertex */
        NewIndex = Surface_->addVertex(
            VertexPos, TexCoord
            #ifdef COLOR_PAINT
            ,c
            #endif
        );
        
        /* Reset the old indices */
        for (j = 0; j < FullList.size(); ++j)
        {
            if (*it == FullList[j])
                FullList[j] = NewIndex;
        }
        
    } // next vertex
    
    /* Loop for all indices (listed as triangle-list) */
    for (i = 0; i < FullList.size() - 2; i += 3)
    {
        
        /* Add a new triangle */
        Surface_->addTriangle(
            FullList[i+0],
            FullList[i+1],
            FullList[i+2]
        );
        
    } // next triangle
}

dim::point2df SceneLoaderBSP1::calcTexCoord(const dim::vector3df &Pos, const STextureInfoBSP &TexInfo)
{
    /* Temporary variables */
    dim::point2df Map;
    
    dim::vector3df VecS(TexInfo.VectorS);
    dim::vector3df VecT(TexInfo.VectorT);
    
    swapVector(VecS);
    swapVector(VecT);
    
    Map.X = Pos.dot(VecS) + TexInfo.DistS;
    Map.Y = Pos.dot(VecT) + TexInfo.DistT;
    
    Map.X /= WallTextureList_[TexInfo.TextureID].Width;
    Map.Y /= WallTextureList_[TexInfo.TextureID].Height;
    
    /* Return the texture coordiante */
    return Map;
}

void SceneLoaderBSP1::buildModel()
{
    /* Optimize the surfaces */
    MeshBase_->optimizeMeshBuffers();
    MeshTrans_->optimizeMeshBuffers();
    
    /* Update the model and build it finally */
    MeshBase_->updateMeshBuffer();
    MeshTrans_->updateMeshBuffer();
}


} // /namespace scene

} // /namespace sp


#endif



// ==========================================================================
