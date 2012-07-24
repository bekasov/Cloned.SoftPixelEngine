/*
 * Quake 3 BSP loader file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "FileFormats/Scene/spSceneLoaderBSP3.hpp"

#ifdef SP_COMPILE_WITH_SCENELOADER_BSP3


#include "Platform/spSoftPixelDeviceOS.hpp"


namespace sp
{

extern video::RenderSystem* __spVideoDriver;
extern scene::SceneGraph* __spSceneManager;

namespace scene
{


s32 SceneLoaderBSP3::Tessellation_ = DEF_MESH_TESSELLATION;

SceneLoaderBSP3::SceneLoaderBSP3() : SceneLoader()
{
}
SceneLoaderBSP3::~SceneLoaderBSP3()
{
}

Mesh* SceneLoaderBSP3::loadScene(const io::stringc &Filename, const io::stringc &TexturePath, const s32 Flags)
{
    /* First settings */
    Filename_       = Filename;
    TexturePath_    = TexturePath;
    
    /* Create an empty mesh */
    MeshBase_       = new Mesh();
    MeshBase_->getMaterial()->setLighting(false);
    
    /* Create an empty mesh for transparent textures */
    MeshTrans_   = __spSceneManager->createMesh();
    MeshTrans_->getMaterial()->setLighting(false);
    MeshTrans_->getMaterial()->setBlending(false);
    MeshTrans_->getMaterial()->setAlphaMethod(video::CMPSIZE_GREATER, 0.5);
    MeshTrans_->getMaterial()->setRenderFace(video::FACE_BOTH);
    MeshTrans_->setParent(MeshBase_);
    
    /* Check if the file has been opened successful */
    if (!openForReading())
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

void SceneLoaderBSP3::setTessellation(s32 Tessellation)
{
    Tessellation_ = Tessellation;
}


/*
 * ======= Private: ========
 */

bool SceneLoaderBSP3::readHeader()
{
    
    /* Read the header */
    File_->readBuffer(&Header_, sizeof(Header_));
    
    /* Check the magic identity */
    c8 Magic[5] = { 0 };
    memcpy(Magic, Header_.Magic, 4);
    
    if (io::stringc(Magic) != "IBSP")
    {
        /* Print an error message */
        io::Log::error("BSP (Quake III Arena) file has incorrect magic number");
        
        /* Exit the function and return false for a failure */
        return false;
    }
    
    /* Check the version */
    if (Header_.Version != 0x2E)
    {
        /* Print an error message */
        io::Log::error("BSP (Quake III Arena) file has incorrect version number");
        
        /* Exit the function and return false for a failure */
        return false;
    }
    
    /* Exit the function and return true for success */
    return true;
    
}

void SceneLoaderBSP3::readLumps()
{
    
    /* Read all lumps */
    readLumpEntities();
    readLumpTextures();
    readLumpPlanes();
    readLumpNodes();
    readLumpLeafs();
    readLumpLeafFaces();
    readLumpLeafBrushes();
    readLumpModels();
    readLumpBrushes();
    readLumpBrushSides();
    readLumpVertexes();
    readLumpMeshVerts();
    readLumpEffecs();
    readLumpFaces();
    readLumpLightMaps();
    readLumpLightVols();
    readLumpVisData();
    
}

void SceneLoaderBSP3::readLumpEntities()
{
    
    /* Temporary vairables */
    std::vector<io::stringc> ScriptData;
    
    /* Set the offset */
    File_->setSeek(Header_.DirEntries[BSP_LUMP_ENTITIES].Offset);
    
    /* Allocate temporary memory */
    c8* EntDesc = new c8[ Header_.DirEntries[BSP_LUMP_ENTITIES].Length + 1 ];
    
    /* Read the mesh descriptions */
    File_->readBuffer(EntDesc, Header_.DirEntries[BSP_LUMP_ENTITIES].Length);
    
    /* Set the end of the string */
    EntDesc[Header_.DirEntries[BSP_LUMP_ENTITIES].Length] = 0;
    
    /* Create a script */
    BSPLoaderExtensions::createScript(ScriptData, EntDesc);
    
    /* Examine the script */
    examineScript(ScriptData);
    
    /* Delete the temporary memory */
    MemoryManager::deleteBuffer(EntDesc);
    
}

void SceneLoaderBSP3::readLumpTextures()
{
    
    /* Temporary variables */
    const s32 ChunkSize     = 64 + 2 * sizeof(s32);
    const s32 ChunkCount    = Header_.DirEntries[BSP_LUMP_TEXTURES].Length / ChunkSize;
    
    c8 TextureName[64];
    s32 Flags, Contents;
    
    io::stringc TexFilename;
    io::stringc FileExtension;
    
    /* Set the offset */
    File_->setSeek(Header_.DirEntries[BSP_LUMP_TEXTURES].Offset);
    
    /* Loop for all chunks */
    for (s32 i = 0; i < ChunkCount; ++i)
    {
        
        /* Read the texture name */
        File_->readBuffer(TextureName, sizeof(TextureName));
        
        /* Read the flags */
        Flags = File_->readValue<s32>();
        
        /* Read the contents */
        Contents = File_->readValue<s32>();
        
        /* Set the texture filename */
        TexFilename = TexturePath_ + io::stringc(TextureName);
        
        /* Check for the file extension */
        if (FileSys_.findFile(TexFilename + ".jpg"))
            FileExtension = ".jpg";
        else
            FileExtension = ".tga";
        
        /* Load the texture */
        TextureList_.push_back( __spVideoDriver->loadTexture(TexFilename + FileExtension) );
        
    } // next chunk
    
}

void SceneLoaderBSP3::readLumpPlanes()
{
    
    /* Temporary variables */
    const s32 ChunkSize     = 4 * sizeof(f32);
    const s32 ChunkCount    = Header_.DirEntries[BSP_LUMP_PLANES].Length / ChunkSize;
    
    dim::vector3df Normal;
    f32 Dist;
    
    /* Set the offset */
    File_->setSeek(Header_.DirEntries[BSP_LUMP_PLANES].Offset);
    
    /* Loop for all chunks */
    for (s32 i = 0; i < ChunkCount; ++i)
    {
        
        /* Read the normal */
        Normal.set(
            File_->readValue<f32>(),
            File_->readValue<f32>(),
            File_->readValue<f32>()
        );
        
        /* Read the distance */
        Dist = File_->readValue<f32>();
        
        /* Create a new plane */
        PlaneList_.push_back(dim::plane3df(Normal, Dist));
        
    } // next chunk
    
}

void SceneLoaderBSP3::readLumpNodes()
{
    
    /* Temporary variables */
    const s32 ChunkSize     = sizeof(SNodeBSP);
    const s32 ChunkCount    = Header_.DirEntries[BSP_LUMP_NODES].Length / ChunkSize;
    
    SNodeBSP ChunkData;
    
    /* Set the offset */
    File_->setSeek(Header_.DirEntries[BSP_LUMP_NODES].Offset);
    
    /* Loop for all chunks */
    for (s32 i = 0; i < ChunkCount; ++i)
    {
        
        /* Read the node data */
        File_->readBuffer(&ChunkData, sizeof(ChunkData));
        
        /* ... */
        
    } // next chunk
    
}

void SceneLoaderBSP3::readLumpLeafs()
{
    
    /* Temporary variables */
    const s32 ChunkSize     = sizeof(SLeafBSP);
    const s32 ChunkCount    = Header_.DirEntries[BSP_LUMP_LEAFS].Length / ChunkSize;
    
    SLeafBSP ChunkData;
    
    /* Set the offset */
    File_->setSeek(Header_.DirEntries[BSP_LUMP_LEAFS].Offset);
    
    /* Loop for all chunks */
    for (s32 i = 0; i < ChunkCount; ++i)
    {
        
        /* Read the leaf data */
        File_->readBuffer(&ChunkData, sizeof(ChunkData));
        
        /* ... */
        
    } // next chunk
    
}

void SceneLoaderBSP3::readLumpLeafFaces()
{
    
    /* Temporary variables */
    const s32 ChunkSize     = sizeof(s32);
    const s32 ChunkCount    = Header_.DirEntries[BSP_LUMP_LEAFFACES].Length / ChunkSize;
    
    s32 Face;
    
    /* Set the offset */
    File_->setSeek(Header_.DirEntries[BSP_LUMP_LEAFFACES].Offset);
    
    /* Loop for all chunks */
    for (s32 i = 0; i < ChunkCount; ++i)
    {
        
        /* Read the leaf face data */
        Face = File_->readValue<s32>();
        
        /* ... */
        
    } // next chunk
    
}

void SceneLoaderBSP3::readLumpLeafBrushes()
{
    
    /* Temporary variables */
    const s32 ChunkSize     = sizeof(s32);
    const s32 ChunkCount    = Header_.DirEntries[BSP_LUMP_LEAFBRUSHES].Length / ChunkSize;
    
    s32 Brush;
    
    /* Set the offset */
    File_->setSeek(Header_.DirEntries[BSP_LUMP_LEAFBRUSHES].Offset);
    
    /* Loop for all chunks */
    for (s32 i = 0; i < ChunkCount; ++i)
    {
        
        /* Read the leaf brush data */
        Brush = File_->readValue<s32>();
        
        /* ... */
        
    } // next chunk
    
}

void SceneLoaderBSP3::readLumpModels()
{
    
    /* Temporary variables */
    const s32 ChunkSize     = sizeof(SModelBSP);
    const s32 ChunkCount    = Header_.DirEntries[BSP_LUMP_MODELS].Length / ChunkSize;
    
    SModelBSP ChunkData;
    
    /* Set the offset */
    File_->setSeek(Header_.DirEntries[BSP_LUMP_MODELS].Offset);
    
    /* Loop for all chunks */
    for (s32 i = 0; i < ChunkCount; ++i)
    {
        
        /* Read the model data */
        File_->readBuffer(&ChunkData, sizeof(ChunkData));
        
        /* ... */
        
    } // next chunk
    
}

void SceneLoaderBSP3::readLumpBrushes()
{
    
    /* Temporary variables */
    const s32 ChunkSize     = sizeof(SBrushBSP);
    const s32 ChunkCount    = Header_.DirEntries[BSP_LUMP_BRUSHES].Length / ChunkSize;
    
    SBrushBSP ChunkData;
    
    /* Set the offset */
    File_->setSeek(Header_.DirEntries[BSP_LUMP_BRUSHES].Offset);
    
    /* Loop for all chunks */
    for (s32 i = 0; i < ChunkCount; ++i)
    {
        
        /* Read the brush data */
        File_->readBuffer(&ChunkData, sizeof(ChunkData));
        
        /* ... */
        
    } // next chunk
    
}

void SceneLoaderBSP3::readLumpBrushSides()
{
    
    /* Temporary variables */
    const s32 ChunkSize     = sizeof(SBrushSideBSP);
    const s32 ChunkCount    = Header_.DirEntries[BSP_LUMP_BRUSHSIDES].Length / ChunkSize;
    
    SBrushSideBSP ChunkData;
    
    /* Set the offset */
    File_->setSeek(Header_.DirEntries[BSP_LUMP_BRUSHSIDES].Offset);
    
    /* Loop for all chunks */
    for (s32 i = 0; i < ChunkCount; ++i)
    {
        
        /* Read the brush side data */
        File_->readBuffer(&ChunkData, sizeof(ChunkData));
        
        /* ... */
        
    } // next chunk
    
}

void SceneLoaderBSP3::readLumpVertexes()
{
    
    /* Temporary variables */
    const s32 ChunkSize     = sizeof(SVertexBSP);
    const s32 ChunkCount    = Header_.DirEntries[BSP_LUMP_VERTEXES].Length / ChunkSize;
    
    SVertexBSP ChunkData;
    
    /* Set the offset */
    File_->setSeek(Header_.DirEntries[BSP_LUMP_VERTEXES].Offset);
    
    /* Loop for all chunks */
    for (s32 i = 0; i < ChunkCount; ++i)
    {
        
        /* Read the vertex data */
        File_->readBuffer(&ChunkData, sizeof(ChunkData));
        
        /* Swap the vertex coord */
        dim::vector3df Vector;
        
        Vector = ChunkData.Position / 64;
        
        ChunkData.Position.X = Vector.X;
        ChunkData.Position.Y = Vector.Z;
        ChunkData.Position.Z = Vector.Y;
        
        /* Swap the vertex normal */
        Vector = ChunkData.Normal;
        
        ChunkData.Normal.X = Vector.X;
        ChunkData.Normal.Y = Vector.Z;
        ChunkData.Normal.Z = Vector.Y;
        
        /* Add the vertex to the list */
        VerticesList_.push_back(ChunkData);
        
    } // next chunk
    
}

void SceneLoaderBSP3::readLumpMeshVerts()
{
    
    /* Temporary variables */
    const s32 ChunkSize     = sizeof(s32);
    const s32 ChunkCount    = Header_.DirEntries[BSP_LUMP_MESHVERTS].Length / ChunkSize;
    
    /* Set the offset */
    File_->setSeek(Header_.DirEntries[BSP_LUMP_MESHVERTS].Offset);
    
    /* Loop for all chunks */
    for (s32 i = 0; i < ChunkCount; ++i)
    {
        
        /* Read the offset */
        MeshVertOffsetList_.push_back( File_->readValue<s32>() );
        
    } // next chunk
    
}

void SceneLoaderBSP3::readLumpEffecs()
{
    
    /* Temporary variables */
    const s32 ChunkSize     = sizeof(SEffectBSP);
    const s32 ChunkCount    = Header_.DirEntries[BSP_LUMP_EFFECTS].Length / ChunkSize;
    
    SEffectBSP ChunkData;
    
    /* Set the offset */
    File_->setSeek(Header_.DirEntries[BSP_LUMP_EFFECTS].Offset);
    
    /* Loop for all chunks */
    for (s32 i = 0; i < ChunkCount; ++i)
    {
        
        /* Read the vertex data */
        File_->readBuffer(&ChunkData, sizeof(ChunkData));
        
        /* ... */
        
    } // next chunk
    
}

void SceneLoaderBSP3::readLumpFaces()
{
    
    /* Temporary variables */
    const s32 ChunkSize     = sizeof(SFaceBSP);
    const s32 ChunkCount    = Header_.DirEntries[BSP_LUMP_FACES].Length / ChunkSize;
    
    SFaceBSP ChunkData;
    
    /* Set the offset */
    File_->setSeek(Header_.DirEntries[BSP_LUMP_FACES].Offset);
    
    /* Loop for all chunks */
    for (s32 i = 0; i < ChunkCount; ++i)
    {
        
        /* Read the vertex data */
        File_->readBuffer(&ChunkData, sizeof(ChunkData));
        
        /* Add the face to the list */
        FacesList_.push_back(ChunkData);
        
    } // next chunk
    
}

void SceneLoaderBSP3::readLumpLightMaps()
{
    
    /* Temporary variables */
    const s32 ChunkSize     = sizeof(SLightMapBSP);
    const s32 ChunkCount    = Header_.DirEntries[BSP_LUMP_LIGHTMAPS].Length / ChunkSize;
    
    SLightMapBSP ChunkData;
    
    const s32 ImageBufferSize = 128 * 128 * 3;
    
    const f32 Gamma = 2.5f;
    const f32 ScaleGamma = Gamma / 255;
    
    s32 x, y, j;
    f32 r, g, b;
    f32 scale, temp;
    
    video::Texture* LightMapTexture;
    u8* ImageBuffer = 0;
    
    /* Set the offset */
    File_->setSeek(Header_.DirEntries[BSP_LUMP_LIGHTMAPS].Offset);
    
    /* Loop for all chunks */
    for (s32 i = 0; i < ChunkCount; ++i)
    {
        
        /* Read the vertex data */
        File_->readBuffer(&ChunkData, sizeof(ChunkData));
        
        /*
         * Allocate new image data.
         * it will not deleted in this function,
         * because the Texture-class proceeds this
         */
        ImageBuffer = new u8[ImageBufferSize];
        
        /* Fill the image data */
        for (y = 0, j = 0; y < 128; ++y)
        {
            for (x = 0; x < 128; ++x, j += 3)
            {
                /* Get the lightmap raw data */
                r = ChunkData.LightMap[y][x][0];
                g = ChunkData.LightMap[y][x][1];
                b = ChunkData.LightMap[y][x][2];
                
                /* Scale the temporary color values */
                r *= ScaleGamma;
                g *= ScaleGamma;
                b *= ScaleGamma;
                
                scale = 1.0f;
                
                /* Check for to highest values */
                if (r > 1.0f && (temp = (1.0f/r)) < scale) scale = temp;
                if (g > 1.0f && (temp = (1.0f/g)) < scale) scale = temp;
                if (b > 1.0f && (temp = (1.0f/b)) < scale) scale = temp;
                
                scale *= 255.0f;
                
                /* Scale the temporary color values */
                r *= scale;
                g *= scale;
                b *= scale;
                
                /* Set the final color */
                ImageBuffer[j+0] = (u8)r;
                ImageBuffer[j+1] = (u8)g;
                ImageBuffer[j+2] = (u8)b;
            }
        }
        
        /* Create a new light map */
        LightMapTexture = __spVideoDriver->createTexture(dim::size2di(128, 128), video::PIXELFORMAT_RGB, ImageBuffer);
        
        /* Add the light map to the list */
        LightMapList_.push_back(LightMapTexture);
        
    } // next chunk
    
}

void SceneLoaderBSP3::readLumpLightVols()
{
    
    /* Temporary variables */
    const s32 ChunkSize     = sizeof(SLightVolsBSP);
    const s32 ChunkCount    = Header_.DirEntries[BSP_LUMP_LIGHTVOLS].Length / ChunkSize;
    
    SLightVolsBSP ChunkData;
    
    /* Set the offset */
    File_->setSeek(Header_.DirEntries[BSP_LUMP_LIGHTVOLS].Offset);
    
    /* Loop for all chunks */
    for (s32 i = 0; i < ChunkCount; ++i)
    {
        
        /* Read the vertex data */
        File_->readBuffer(&ChunkData, sizeof(ChunkData));
        
        /* ... */
        
    } // next chunk
    
}

void SceneLoaderBSP3::readLumpVisData()
{
    
    /* Temporary variables */
    SVisDataBSP ChunkData;
    s32 VisDataSize;
    
    /* Set the offset */
    File_->setSeek(Header_.DirEntries[BSP_LUMP_VISDATA].Offset);
    
    /* Read the number of vectors */
    ChunkData.CountVectors = File_->readValue<s32>();
    
    /* Read the size of vectors */
    ChunkData.VectorSize = File_->readValue<s32>();
    
    /* Compute the visbility data size */
    VisDataSize = ChunkData.CountVectors * ChunkData.VectorSize;
    
    /* Allocate enough visibility memory */
    ChunkData.VisibilityData = new u8[VisDataSize];
    
    /* Read the visibility data */
    File_->readBuffer(ChunkData.VisibilityData, VisDataSize);
    
    /* ... */
    
    /* Delete the visibility data */
    delete [] ChunkData.VisibilityData;
    
}

void SceneLoaderBSP3::createNewVertex(SVertexBSP &Vertex, SFaceBSP &Face)
{
    /* Add a new vertex */
    const u32 VertexID = Surface_->addVertex(
        Vertex.Position,
        Vertex.Normal,
        dim::point2df(Vertex.TexCoord[0].X, Vertex.TexCoord[0].Y),
        255,//Vertex.Color
        0.0f
    );
    
    /* Set the second texture map coordinate */
    Surface_->setVertexTexCoord(
        VertexID,
        dim::point2df(Vertex.TexCoord[1].X, Vertex.TexCoord[1].Y),
        1
    );
}

void SceneLoaderBSP3::buildModel()
{
    
    /* Temporary variables */
    SVertexBSP Vertex;
    
    /* Loop for all faces */
    for (std::vector<SFaceBSP>::iterator it = FacesList_.begin(); it != FacesList_.end(); ++it)
    {
        
        /* Determine which mesh must be used */
        Mesh_ = (TextureList_[it->Texture]->getFormat() == video::PIXELFORMAT_RGBA ? MeshTrans_ : MeshBase_);
        
        /* Create a new surface */
        Surface_= Mesh_->createMeshBuffer();
        
        /* Add the texture */
        Surface_->addTexture( TextureList_[it->Texture] );
        
        /* Add the lightmap */
        if (it->LightMap >= 0 && static_cast<u32>(it->LightMap) < LightMapList_.size())
            Surface_->addTexture( LightMapList_[it->LightMap] );
        
        /* Compute the count of vertices/ control points */
        s32 Count = (it->Type == BSP_FACE_MESH ? it->CountMeshVerts : it->CountVertices);
        
        if (Count >= static_cast<s32>(VerticesList_.size()) - it->FirstVertex)
            Count = VerticesList_.size() - it->FirstVertex;
        
        /* Check the type */
        if (it->Type != BSP_FACE_PATCH)
        {
            /* Loop for all vertices for the surface */
            for (s32 i = 0; i < Count; ++i)
            {
                /* Create a new vertex */
                createNewVertex(VerticesList_[it->FirstVertex + i], *it);
            } // next vertex
        }
        
        /* Determine which face type is used */
        switch (it->Type)
        {
            case BSP_FACE_POLYGON:
            {
                
                /* Loop for all traingles for the surface */
                for (s32 i = 1; i < it->CountVertices - 1; ++i)
                {
                    
                    /* Add a new triangle */
                    Surface_->addTriangle(0, i, i+1);
                    
                } // next triangle
                
            }
            break;
            
            case BSP_FACE_PATCH:
            {
                
                /* Temporary vairbales */
                s32 x, y, s, p;
                s32 i, j, k, c;
                
                BiQuadraticPatch Patch;
                
                std::vector<SVertexBSP> Vertices(Count);
                
                dim::size2di biquadSize(
                    (it->PatchSize.Width  - 1) / 2,
                    (it->PatchSize.Height - 1) / 2
                );
                
                /* Translate the created vertices in the temporary vertices list */
                for (i = 0; i < Count; ++i)
                {
                    /* Copy the vertex data */
                    Vertices[i] = VerticesList_[it->FirstVertex + i];
                }
                
                /* Loop for all patches */
                for (y = 0; y < biquadSize.Height; ++y)
                {
                    for (x = 0; x < biquadSize.Width; ++x)
                    {
                        /* Loop for all control points */
                        for (s32 row = 0; row < 3; ++row)
                        {
                            for (s32 point = 0; point < 3; ++point)
                            {
                                /* Get the control points */
                                Patch.ControlPoints[row*3+point] = 
                                    Vertices[ ( y*2*it->PatchSize.Width+x*2 ) + row*it->PatchSize.Width+point ];
                            }
                        }
                        
                        /* Compute the tesselation */
                        Patch.tessellate(Tessellation_);
                    }
                }
                
                /* Loop for all vertices */
                for (std::vector<SVertexBSP>::iterator itPatch = Patch.VerticesList.begin(); itPatch != Patch.VerticesList.end(); ++itPatch)
                {
                    /* Create a new vertex */
                    createNewVertex(*itPatch, *it);
                } // next vertex
                
                /* Loop for all patches */
                for (p = 0, c = 2*(Tessellation_ + 1); p < biquadSize.Height * biquadSize.Width; ++p)
                {
                    /* Loop for all triangles */
                    for (i = 0; i < Tessellation_; ++i)
                    {
                        for (j = 0, s = 0; j < c - 2; ++j)
                        {
                            
                            /* Compute the first index */
                            k = i*c;
                            
                            /* Swap the triangle strip side */
                            s = 1 - s;
                            
                            /* Create a new triangle */
                            if (s)
                                Surface_->addTriangle(Patch.IndicesList[p][k+j+0], Patch.IndicesList[p][k+j+1], Patch.IndicesList[p][k+j+2]);
                            else
                                Surface_->addTriangle(Patch.IndicesList[p][k+j+2], Patch.IndicesList[p][k+j+1], Patch.IndicesList[p][k+j+0]);
                            
                        } // next triangle
                    } // next tesseleation
                } // next patch
                
                /* Update the normals for the current surface */
                Surface_->updateNormals(Mesh_->getMaterial()->getShading());
                
            }
            break;
            
            case BSP_FACE_MESH:
            {
                
                /* Loop for all traingles for the surface */
                for (s32 i = 0; i < it->CountMeshVerts; i += 3)
                {
                    
                    /* Add a new triangle */
                    Surface_->addTriangle(
                        MeshVertOffsetList_[it->FirstMeshVerts + i+0],
                        MeshVertOffsetList_[it->FirstMeshVerts + i+1],
                        MeshVertOffsetList_[it->FirstMeshVerts + i+2]
                    );
                    
                } // next triangle
                
            }
            break;
            
            case BSP_FACE_BILLBOARD:
            {
                
            }
            break;
        }
        
    } // next face
    
    /* Optimize the surfaces */
    MeshBase_->optimizeMeshBuffers();
    MeshTrans_->optimizeMeshBuffers();
    
    /* Update the model and build it finally */
    MeshBase_->updateMeshBuffer();
    MeshTrans_->updateMeshBuffer();
    
}

void SceneLoaderBSP3::examineScript(std::vector<io::stringc> &ScriptData)
{
    
    /* Temporary variables */
    io::stringc Type, Value, ClassName;
    dim::vector3df Vector;
    
    /* Loop for all lines */
    for (std::vector<io::stringc>::iterator it = ScriptData.begin(); it != ScriptData.end(); ++it)
    {
        /* Check special symboles */
        if (*it == "{")
            continue;
        
        /* Check if an block has been end */
        if (*it == "}")
        {
            /* Check which classname has the block */
            if (ClassName.left(7) == "weapon_" || ClassName.left(5) == "item_" || ClassName.left(5) == "ammo_")
            {
                scene::SceneNode* NodeObject = __spSceneManager->createNode();
                NodeObject->setParent(MeshBase_);
                NodeObject->setPosition(dim::vector3df(Vector.X, Vector.Z, Vector.Y) / 64);
                NodeObject->setName(ClassName);
            }
            
            /* Reset the classname */
            ClassName = "";
            
            /* Continue with the next line */
            continue;
        }
        
        /* Get the type */
        Type = getScriptLineType(*it);
        
        /* Get the value */
        Value = getScriptLineValue(*it);
        
        /* First check of the type */
        if (Type == "classname")
            ClassName = Value;
        else if (Type == "origin")
            Vector = getScriptLineVector(*it, Value);
        
    } // next line
    
}

io::stringc SceneLoaderBSP3::getScriptLineType(io::stringc &Line)
{
    return Line.section(1, Line.find("\"", 1));
}

io::stringc SceneLoaderBSP3::getScriptLineValue(io::stringc &Line)
{
    return Line.section(Line.find("\"", 2) + 3, Line.size() - 1);
}

dim::vector3df SceneLoaderBSP3::getScriptLineVector(io::stringc &Line, io::stringc Value)
{
    /* Temporary variables */
    dim::vector3df Vector;
    
    /* Get the blank positions */
    s32 pos1 = Value.find(" ", 1);
    
    if (pos1 == -1)
        return dim::vector3df();
    
    s32 pos2 = Value.find(" ", pos1 + 1);
    
    /* Get the component values */
    Vector.X = Value.left(pos1).val<f32>();
    Vector.Y = Value.section(pos1, pos2).val<f32>();
    Vector.Z = Value.right(Value.size() - pos2).val<f32>();
    
    /* Return the vector */
    return Vector;
}


/*
 * ========== BSP Loader internal class - BiQuadraticPatch ==========
 */

SceneLoaderBSP3::BiQuadraticPatch::BiQuadraticPatch()
{
}
SceneLoaderBSP3::BiQuadraticPatch::~BiQuadraticPatch()
{
}

void SceneLoaderBSP3::BiQuadraticPatch::tessellate(s32 Level)
{
    
    /* Temporary variables */
    f32 a, b;
    s32 i, j, k, c;
    
    SVertexBSP TempVertex[3];
    SVertexBSP Vertex;
    
    /* Loop for the first tesselation */
    for (i = 0; i <= Level; ++i)
    {
        /* Compute the patch x axis */
        a = (f32)i / Level;
        b = 1.0f - a;
        
        /* Set the current vertex data */
        Vertex = 
            ControlPoints[0] * (b * b) +
            ControlPoints[3] * (2 * b * a) +
            ControlPoints[6] * (a * a);
        
        /* Add a new vertex */
        VerticesList.push_back(Vertex);
    }
    
    /* Loop for the second tesselation */
    for (i = 1; i <= Level; ++i)
    {
        /* Compute the patch y axis */
        a = (f32)i / Level;
        b = 1.0f - a;
        
        /* Loop for the three temporary vertices */
        for (j = 0; j < 3; ++j)
        {
            /* Compute the index for the temporary vertices */
            k = j * 3;
            
            /* Compute the temporary vertices */
            TempVertex[j] =
                ControlPoints[k + 0] * (b * b) +
                ControlPoints[k + 1] * (2 * b * a) +
                ControlPoints[k + 2] * (a * a);
        }
        
        /* Loop for the third tesselation */
        for (j = 0; j <= Level; ++j)
        {
            /* Compute the patch x axis */
            a = (f32)j / Level;
            b = 1.0f - a;
            
            /* Set the current vertex data */
            Vertex = 
                TempVertex[0] * (b * b) +
                TempVertex[1] * (2 * b * a) +
                TempVertex[2] * (a * a);
            
            /* Add a new vertex */
            VerticesList.push_back(Vertex);
        }
    }
    
    /* Temporary indices list */
    std::vector<u32> TempList;
    
    /* Compute the last count */
    c = IndicesList.size() * ( ( Level + 1 ) * (Level + 1) );
    
    /* Loop for all indices */
    for (s32 row = 0; row < Level; ++row)
    {
        for (s32 point = 0; point <= Level; ++point)
        {
            /* Compute the next two indices */
            TempList.push_back( c + (row + 1) * (Level + 1) + point );
            TempList.push_back( c + row * (Level + 1) + point );
        }
    }
    
    /* Add a new indices list */
    IndicesList.push_back(TempList);
    
}


/*
 * BSPLoaderExtensions class
 */

BSPLoaderExtensions::BSPLoaderExtensions()
{
}
BSPLoaderExtensions::~BSPLoaderExtensions()
{
}

void BSPLoaderExtensions::createScript(std::vector<io::stringc> &ScriptData, c8* MeshDescription)
{
    
    /* Temporary variables */
    s32 begin = 0, end = 0, i;
    
    /* Loop while search for new lines */
    do
    {
        /* Check if the end of a line has been detected */
        if (MeshDescription[end] == '\n')
        {
            /* Allocate a new line */
            std::string Line(end - begin, ' ');
            
            /* Loop for the detected part */
            for (i = begin; i < end; ++i)
                Line[i - begin] = MeshDescription[i];
            
            /* Add the new line */
            ScriptData.push_back(Line);
            
            /* Set the new start point */
            begin = end + 1;
        }
    }
    while (MeshDescription[end++]);
    
}


} // /namespace scene

} // /namespace sp


#endif



// ==========================================================================
