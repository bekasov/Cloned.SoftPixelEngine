/*
 * Terrain scene node file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "SceneGraph/spSceneTerrain.hpp"
#include "Platform/spSoftPixelDeviceOS.hpp"
#include "RenderSystem/spTextureBase.hpp"


namespace sp
{

extern video::RenderSystem* GlbRenderSys;
extern scene::SceneGraph* GlbSceneGraph;

namespace scene
{


#if 0

/*
 * Internal functions
 */

void clbTerrainTreeNodeDestructor(TreeNode* Node)
{
    Terrain::STreeNodeData* NodeData = (Terrain::STreeNodeData*)Node->getUserData();
    
    if (NodeData)
        MemoryManager::deleteMemory(NodeData);
}


/*
 * Terrain class
 */

Terrain::Terrain(
    const video::SHeightMapTexture &HeightMap, const dim::size2di &Resolution, s32 GeoMIPLevels) :
    MaterialNode        (NODE_TERRAIN   ),
    HeightMap_          (HeightMap      ),
    MeshResolution_     (Resolution     ),
    GeoMIPLevels_       (GeoMIPLevels   ),
    RootTreeNode_       (0              ),
    MeshTexReference_   (0              ),
    RenderModeListSize_ (0              )
{
    setupTerrain();
}
Terrain::~Terrain()
{
    HeightMap_.clearBuffer();
    
    delete RootTreeNode_;
    delete MeshTexReference_;
}

void Terrain::render()
{
    /* Matrix transformation */
    loadTransformation();
    
    GlobalTerrainTransformation_    = GlbRenderSys->getWorldMatrix();
    GlobalCamPosition_              = GlbSceneGraph->getActiveCamera()->getPosition(true);
    
    /* Update the render matrix */
    GlbRenderSys->updateModelviewMatrix();
    
    /* Setup material states */
    GlbRenderSys->setupMaterialStates(getMaterial());
    GlbRenderSys->setupShaderClass(this, getShaderClass());
    
    /* Select each tree-node to be rendered */
    selectTreeNodeMesh(RootTreeNode_);
    
    /* Render each selected tree-node */
    for (s32 i = 0; i < RenderModeListSize_; ++i)
        renderTreeNodeMesh(RenderNodeList_[i]);
    
    /* Disable the selection status */
    for (s32 i = 0; i < RenderModeListSize_; ++i)
        static_cast<STreeNodeData*>(RenderNodeList_[i]->getUserData())->Selected = false;
    
    RenderModeListSize_ = 0;
    
    /* Unbinding the shader */
    GlbRenderSys->unbindShaders();
}

void Terrain::changeHeightMap(
    const video::SHeightMapTexture &HeightMap, const dim::size2di &Resolution, s32 GeoMIPLevels)
{
    /* Basic settings */
    HeightMap_.clearBuffer();
    HeightMap_      = HeightMap;
    Resolution_     = 1 << (GeoMIPLevels - 1);
    MeshResolution_ = Resolution;
    GeoMIPLevels_   = GeoMIPLevels;
    
    /* Recreate the heightmap */
    MemoryManager::deleteMemory(RootTreeNode_);
    
    setupTerrain();
}


/*
 * ======= Private: =======
 */

void Terrain::setupTerrain()
{
    if (!MeshTexReference_)
        MeshTexReference_ = new video::MeshBuffer();
    
    Resolution_ = 1 << (GeoMIPLevels_ - 1);
    
    RootTreeNode_ = new QuadTreeNode();
    
    s32 MIPLevel = 0;
    createQuadTree(RootTreeNode_, MIPLevel, 0);
    
    RenderNodeList_.resize(RenderModeListSize_);
    RenderModeListSize_ = 0;
}

void Terrain::createQuadTree(QuadTreeNode* Node, s32 &MIPLevel, dim::point2di CurPos)
{
    /* Create the node data */
    STreeNodeData* NodeData = new STreeNodeData;
    {
        /* General node's data */
        NodeData->MIPLevel          = MIPLevel;
        NodeData->Center.X          = (f32)CurPos.X / Resolution_.Width - 0.5f;
        NodeData->Center.Y          = (f32)CurPos.Y / Resolution_.Width - 0.5f;
        NodeData->Resolution.Width  = Resolution_.Width >> MIPLevel;
        NodeData->Resolution.Height = Resolution_.Height >> MIPLevel;
        
        NodeData->EdgeVerticesHeight.resize( (MeshResolution_.Width - 1)*2 + (MeshResolution_.Height - 1)*2 );
        
        NodeData->Center.X += (f32)NodeData->Resolution.Width / Resolution_.Width / 2;
        NodeData->Center.Y += (f32)NodeData->Resolution.Height / Resolution_.Height / 2;
        
        /* Compute node's bounding box */
        NodeData->BoundBox.Min      = dim::vector3df(
            -0.5f + (f32)CurPos.X / Resolution_.Width,
            0.0f,
            -0.5f + (f32)CurPos.Y / Resolution_.Height
        );
        NodeData->BoundBox.Max      = dim::vector3df(
            -0.5f + (f32)CurPos.X / Resolution_.Width + (f32)NodeData->Resolution.Width / Resolution_.Width,
            1.0f,
            -0.5f + (f32)CurPos.Y / Resolution_.Height + (f32)NodeData->Resolution.Height / Resolution_.Height
        );
        
        NodeData->Mesh.setTexturesReference(MeshTexReference_);
    }
    Node->setUserData(NodeData);
    
    Node->setDestructorCallback(clbTerrainTreeNodeDestructor);
    
    ++RenderModeListSize_;
    
    /* Create the mesh data */
    createTreeNodeMesh(Node, NodeData, CurPos);
    
    /* Create the next MIP level */
    ++MIPLevel;
    
    if (MIPLevel < GeoMIPLevels_ && NodeData->Resolution.getArea() > 1)
    {
        Node->addChildren();
        
        dim::point2di Adjustment(
            NodeData->Resolution.Width/2,
            NodeData->Resolution.Height/2
        );
        
        createQuadTree(
            Node->getChild(0), MIPLevel, CurPos
        );
        createQuadTree(
            Node->getChild(1), MIPLevel, CurPos + dim::point2di(0, Adjustment.Y)
        );
        createQuadTree(
            Node->getChild(2), MIPLevel, CurPos + dim::point2di(Adjustment.X, Adjustment.Y)
        );
        createQuadTree(
            Node->getChild(3), MIPLevel, CurPos + dim::point2di(Adjustment.X, 0)
        );
    }
    
    --MIPLevel;
}

void Terrain::createTreeNodeMesh(
    QuadTreeNode* Node, STreeNodeData* NodeData, const dim::point2di &CurPos)
{
    /* Temporary variables */
    s32 x, y, i, j, k;
    dim::vector3df pos, vert, horz;
    
    NodeData->Mesh.addVertices((MeshResolution_ + 1).getArea());
    NodeData->Mesh.addTriangles(MeshResolution_.getArea()*2);
    
    /* Set each vertex position, normal and texture coordinates */
    for (y = i = 0; y <= MeshResolution_.Height; ++y)
    {
        for (x = 0; x <= MeshResolution_.Width; ++x, ++i)
        {
            /* Compute the vertex position */
            pos.X = (f32)x * NodeData->Resolution.Width / Resolution_.Width / MeshResolution_.Width +
                (f32)CurPos.X / Resolution_.Width;
            pos.Z = (f32)y * NodeData->Resolution.Height / Resolution_.Height / MeshResolution_.Height +
                (f32)CurPos.Y / Resolution_.Height;
            
            pos.Y = HeightMap_.getHeightValue(dim::point2df(pos.X, pos.Z));
            
            NodeData->Mesh.setVertexNormal(i, HeightMap_.getNormal(
                dim::point2df(pos.X, pos.Z),
                dim::point2df(
                    (f32)NodeData->Resolution.Width / Resolution_.Width / MeshResolution_.Width,
                    (f32)NodeData->Resolution.Height / Resolution_.Height / MeshResolution_.Height
                )
            ));
            
            for (j = 0; j < MAX_COUNT_OF_TEXTURES; ++j)
                NodeData->Mesh.setVertexTexCoord(i, dim::point2df(pos.X, pos.Z), j);
            
            pos.X -= 0.5f;
            pos.Z -= 0.5f;
            
            NodeData->Mesh.setVertexCoord(i, pos);
            NodeData->Mesh.setVertexColor(i, 255);
        }
    }
    
    /* Set each triangle indices */
    for (y = i = 0; y < MeshResolution_.Height; ++y)
    {
        for (x = 0; x < MeshResolution_.Width; ++x, i += 2)
        {
            NodeData->Mesh.setPrimitiveIndex(i*3 + 0, getVertexIndex(x    , y    ));
            NodeData->Mesh.setPrimitiveIndex(i*3 + 1, getVertexIndex(x    , y + 1));
            NodeData->Mesh.setPrimitiveIndex(i*3 + 2, getVertexIndex(x + 1, y + 1));
            
            NodeData->Mesh.setPrimitiveIndex((i + 1)*3 + 0, getVertexIndex(x    , y    ));
            NodeData->Mesh.setPrimitiveIndex((i + 1)*3 + 1, getVertexIndex(x + 1, y + 1));
            NodeData->Mesh.setPrimitiveIndex((i + 1)*3 + 2, getVertexIndex(x + 1, y    ));
        }
    }
    
    /* Fill the edge vertices height (horizontal) */
    for (x = 1, i = 0; x < MeshResolution_.Width; ++i, ++x)
    {
        j = i;
        k = i + 1;
        NodeData->EdgeVerticesHeight[j] = NodeData->Mesh.getVertexCoord(k).Y;
        
        j = i + MeshResolution_.Width - 1;
        k = i + (MeshResolution_.Width + 1) * MeshResolution_.Height + 1;
        NodeData->EdgeVerticesHeight[j] = NodeData->Mesh.getVertexCoord(k).Y;
    }
    
    /* Fill the edge vertices height (vertical) */
    for (y = 1, i = 0; y < MeshResolution_.Height; ++i, ++y)
    {
        j = i + (MeshResolution_.Width - 1)*2;
        k = (i + 1) * (MeshResolution_.Width + 1);
        NodeData->EdgeVerticesHeight[j] = NodeData->Mesh.getVertexCoord(k).Y;
        
        j = i + (MeshResolution_.Width - 1)*3;
        k = (i + 1) * (MeshResolution_.Width + 1) + MeshResolution_.Width;
        NodeData->EdgeVerticesHeight[j] = NodeData->Mesh.getVertexCoord(k).Y;
    }
    
    /* Update the mesh buffer */
    NodeData->Mesh.updateMeshBuffer();
}

void Terrain::selectTreeNodeMesh(QuadTreeNode* Node)
{
    /* Temporary variables */
    STreeNodeData* NodeData = (STreeNodeData*)Node->getUserData();
    
    #if 1
    const f32 d = math::getDistance(
        GlobalCamPosition_,
        dim::vector3df(NodeData->Center.X, 0.0f, NodeData->Center.Y) * Transform_.getScale() + Transform_.getPosition()
    );
    
    /* Add to the render node list */
    if (d > 7.0f * (GeoMIPLevels_ - NodeData->MIPLevel) || Node->isLeaf() ||
        !checkFurstumCulling(GlbSceneGraph->getActiveCamera()->getViewFrustum(), GlobalTerrainTransformation_, NodeData->BoundBox))
    #endif
    {
        RenderNodeList_[RenderModeListSize_++] = Node;
        NodeData->Selected = true;
        return;
    }
    
    /* Process the children */
    if (!Node->isLeaf())
    {
        for (s32 i = 0; i < 4; ++i)
            selectTreeNodeMesh(Node->getChild(i));
    }
}

void Terrain::renderTreeNodeMesh(QuadTreeNode* Node)
{
    /* Temporary variables */
    STreeNodeData* NodeData = (STreeNodeData*)Node->getUserData();
    
    /* Compute the directions */
    f32 x = static_cast<f32>(NodeData->Resolution.Width) / Resolution_.Width;
    f32 z = static_cast<f32>(NodeData->Resolution.Height) / Resolution_.Height;
    
    /* Deform the node mesh */
    NodeData->recreateBottom( deformTreeNodeMesh(Node, NodeData->Center + dim::point2df( 0, -z)), MeshResolution_ );
    NodeData->recreateTop   ( deformTreeNodeMesh(Node, NodeData->Center + dim::point2df( 0,  z)), MeshResolution_ );
    NodeData->recreateLeft  ( deformTreeNodeMesh(Node, NodeData->Center + dim::point2df(-x,  0)), MeshResolution_ );
    NodeData->recreateRight ( deformTreeNodeMesh(Node, NodeData->Center + dim::point2df( x,  0)), MeshResolution_ );
    
    /* Render the mesh buffer using the unique texture list */
    GlbRenderSys->drawMeshBuffer(&NodeData->Mesh);
}

bool Terrain::deformTreeNodeMesh(QuadTreeNode* Node, const dim::point2df &Pos)
{
    return getNodeLevel(RootTreeNode_, Pos) < static_cast<STreeNodeData*>(Node->getUserData())->MIPLevel;
}

s32 Terrain::getNodeLevel(const QuadTreeNode* Node, const dim::point2df &Pos)
{
    /* Temporary variables */
    STreeNodeData* NodeData = reinterpret_cast<STreeNodeData*>(Node->getUserData());
    
    /* Check if the node is selected */
    if (!Node->isLeaf() && !NodeData->Selected)
    {
        s32 i;
        
        if (Pos.X < NodeData->Center.X)
            i = (Pos.Y < NodeData->Center.Y ? 0 : 1);
        else
            i = (Pos.Y < NodeData->Center.Y ? 3 : 2);
        
        return getNodeLevel(Node->getChild(i), Pos);
    }
    
    return NodeData->MIPLevel;
}


/*
 * STreeNodeData structure
 */

Terrain::STreeNodeData::STreeNodeData() :
    MIPLevel(0      ),
    Selected(false  )
{
    Mesh.createMeshBuffer();
}
Terrain::STreeNodeData::~STreeNodeData()
{
    Mesh.deleteMeshBuffer();
}

// !!! TODO: less spaghetti code !!!

void Terrain::STreeNodeData::recreateBottom(bool Lower, const dim::size2di &Resolution)
{
    if (Lower == EdgesLower.Bottom)
        return;
    
    EdgesLower.Bottom = Lower;
    
    s32 i, j, k;
    dim::vector3df pos;
    
    if (Lower)
    {
        for (i = 0; i < Resolution.Width - 1; i += 2)
        {
            j = i;
            k = i + 1;
            
            pos = Mesh.getVertexCoord(k);
            pos.Y = ( Mesh.getVertexCoord(k - 1).Y + Mesh.getVertexCoord(k + 1).Y ) * 0.5f;
            Mesh.setVertexCoord(k, pos);
        }
    }
    else
    {
        for (i = 0; i < Resolution.Width - 1; i += 2)
        {
            j = i;
            k = i + 1;
            
            pos = Mesh.getVertexCoord(k);
            pos.Y = EdgeVerticesHeight[j];
            Mesh.setVertexCoord(k, pos);
        }
    }
    
    Mesh.updateVertexBuffer();
}

void Terrain::STreeNodeData::recreateTop(bool Lower, const dim::size2di &Resolution)
{
    if (Lower == EdgesLower.Top)
        return;
    
    EdgesLower.Top = Lower;
    
    s32 i, j, k;
    dim::vector3df pos;
    
    if (Lower)
    {
        for (i = 0; i < Resolution.Width - 1; i += 2)
        {
            j = i + Resolution.Width - 1;
            k = i + (Resolution.Width + 1) * Resolution.Height + 1;
            
            pos = Mesh.getVertexCoord(k);
            pos.Y = ( Mesh.getVertexCoord(k - 1).Y + Mesh.getVertexCoord(k + 1).Y ) * 0.5f;
            Mesh.setVertexCoord(k, pos);
        }
    }
    else
    {
        for (i = 0; i < Resolution.Width - 1; i += 2)
        {
            j = i + Resolution.Width - 1;
            k = i + (Resolution.Width + 1) * Resolution.Height + 1;
            
            pos = Mesh.getVertexCoord(k);
            pos.Y = EdgeVerticesHeight[j];
            Mesh.setVertexCoord(k, pos);
        }
    }
    
    Mesh.updateVertexBuffer();
}

void Terrain::STreeNodeData::recreateLeft(bool Lower, const dim::size2di &Resolution)
{
    if (Lower == EdgesLower.Left)
        return;
    
    EdgesLower.Left = Lower;
    
    s32 i, j, k;
    dim::vector3df pos;
    
    if (Lower)
    {
        for (i = 0; i < Resolution.Height - 1; i += 2)
        {
            j = i + (Resolution.Width - 1)*2;
            k = (i + 1) * (Resolution.Width + 1);
            
            pos = Mesh.getVertexCoord(k);
            pos.Y = ( Mesh.getVertexCoord(k - (Resolution.Width + 1)).Y +
                      Mesh.getVertexCoord(k + (Resolution.Width + 1)).Y ) * 0.5f;
            Mesh.setVertexCoord(k, pos);
        }
    }
    else
    {
        for (i = 0; i < Resolution.Height - 1; i += 2)
        {
            j = i + (Resolution.Width - 1)*2;
            k = (i + 1) * (Resolution.Width + 1);
            
            pos = Mesh.getVertexCoord(k);
            pos.Y = EdgeVerticesHeight[j];
            Mesh.setVertexCoord(k, pos);
        }
    }
    
    Mesh.updateVertexBuffer();
}

void Terrain::STreeNodeData::recreateRight(bool Lower, const dim::size2di &Resolution)
{
    if (Lower == EdgesLower.Right)
        return;
    
    EdgesLower.Right = Lower;
    
    s32 i, j, k;
    dim::vector3df pos;
    
    if (Lower)
    {
        for (i = 0; i < Resolution.Height - 1; i += 2)
        {
            j = i + (Resolution.Width - 1)*3;
            k = (i + 1) * (Resolution.Width + 1) + Resolution.Width;
            
            pos = Mesh.getVertexCoord(k);
            pos.Y = ( Mesh.getVertexCoord(k - (Resolution.Width + 1)).Y +
                      Mesh.getVertexCoord(k + (Resolution.Width + 1)).Y ) * 0.5f;
            Mesh.setVertexCoord(k, pos);
        }
    }
    else
    {
        for (i = 0; i < Resolution.Height - 1; i += 2)
        {
            j = i + (Resolution.Width - 1)*3;
            k = (i + 1) * (Resolution.Width + 1) + Resolution.Width;
            
            pos = Mesh.getVertexCoord(k);
            pos.Y = EdgeVerticesHeight[j];
            Mesh.setVertexCoord(k, pos);
        }
    }
    
    Mesh.updateVertexBuffer();
}


#else

Terrain::Terrain() :
    MaterialNode    (NODE_TERRAIN   ),
    HeightMap_      (0              ),
    GridSize_       (0              ),
    GeoMIPLevels_   (0              )
{
}
Terrain::~Terrain()
{
}

void Terrain::render()
{
    if (!GlbSceneGraph->getActiveCamera())
        return;
    
    const dim::vector3df GlobalCamPos = GlbSceneGraph->getActiveCamera()->getPosition(true);
    const dim::matrix4f Transform = getTransformMatrix(true);
    
    GlbRenderSys->bindMeshBuffer(&MeshBuffer_);
    {
        drawChunk(GlobalCamPos, Transform, GeoMIPLevels_);
    }
    GlbRenderSys->unbindMeshBuffer();
}

bool Terrain::generate(u32 GridSize, u8 GeoMIPLevels)
{
    if (GridSize == 0 || GeoMIPLevels == 0)
        return false;
    if (GridSize_ == GridSize && GeoMIPLevels_ == GeoMIPLevels)
        return true;
    
    /* Store new settings */
    GridSize_ = GridSize;
    GeoMIPLevels_ = GeoMIPLevels;
    
    InvGridSize_ = 1.0f / static_cast<f32>(GridSize_);
    
    /* Delete previous mesh buffer */
    MeshBuffer_.deleteVertexBuffer();
    
    /* Create new vertex buffer */
    MeshBuffer_.setIndexBufferEnable(false);
    MeshBuffer_.createVertexBuffer();
    
    /* Create grid chunks */
    createChunkBase();
    
    for (u32 i = 0; i < 4; ++i)
        createChunkEdge(static_cast<EChunkTypes>(CHUNK_LEFT + i));
    
    for (u32 i = 0; i < 4; ++i)
        createChunkCorner(static_cast<EChunkTypes>(CHUNK_LEFT_TOP + i));
    
    /* Complete mesh creation */
    MeshBuffer_.updateVertexBuffer();
    
    return true;
}


/*
 * ======= Protected: =======
 */

void Terrain::createChunkBase()
{
    SGridChunk Chunk;
    
    VertexPos Pos;
    
    for (Pos.Y = 0; Pos.Y < GridSize_; ++Pos.Y)
    {
        for (Pos.X = 0; Pos.X < GridSize_; ++Pos.X)
            createQuad(Pos.X, Pos.Y);
    }
    
    Chunk.NumVertices = MeshBuffer_.getIndexOffset();
    
    GridChunks_[CHUNK_BASE] = Chunk;
}

void Terrain::createChunkEdge(const EChunkTypes Type)
{
    SGridChunk Chunk;
    
    Chunk.StartOffset = MeshBuffer_.getIndexOffset();
    
    VertexPos Pos;
    
    for (Pos.Y = 0; Pos.Y < GridSize_; ++Pos.Y)
    {
        for (Pos.X = 0; Pos.Y < GridSize_; ++Pos.X)
        {
            if (isPosEdge(Pos, Type))
                createEdge(Pos.X, Pos.Y, Type);
            else
                createQuad(Pos.X, Pos.Y);
        }
    }
    
    Chunk.NumVertices = MeshBuffer_.getIndexOffset() - Chunk.StartOffset;
    
    GridChunks_[Type] = Chunk;
}

void Terrain::createChunkCorner(const EChunkTypes Type)
{
    SGridChunk Chunk;
    
    VertexPos Pos;
    
    Chunk.StartOffset = MeshBuffer_.getIndexOffset();
    
    for (Pos.Y = 0; Pos.Y < GridSize_; ++Pos.Y)
    {
        for (Pos.X = 0; Pos.X < GridSize_; ++Pos.X)
        {
            if (isPosCorner(Pos, Type))
                createCorner(Pos.X, Pos.Y, Type);
            else if (isPosEdge(Pos, getChunkEdgeType(Pos, Type)))
                createEdge(Pos.X, Pos.Y, getChunkEdgeType(Pos, Type));
            else
                createQuad(Pos.X, Pos.Y);
        }
    }
    
    Chunk.NumVertices = MeshBuffer_.getIndexOffset() - Chunk.StartOffset;
    
    GridChunks_[Type] = Chunk;
}

void Terrain::createVertex(u32 x, u32 y)
{
    dim::vector3df Coord;
    
    Coord.X = InvGridSize_ * static_cast<f32>(x) * 0.5f;
    Coord.Z = InvGridSize_ * static_cast<f32>(y) * 0.5f;
    
    MeshBuffer_.addVertex(Coord, Coord);
}

void Terrain::createQuad(u32 x, u32 y)
{
    createVertex(x    , y    );
    createVertex(x    , y + 2);
    createVertex(x + 2, y + 2);
    createVertex(x    , y + 2);
    
    MeshBuffer_.addTriangle(0, 1, 2);
    MeshBuffer_.addTriangle(0, 2, 3);
    
    MeshBuffer_.addIndexOffset(4);
}

void Terrain::createEdge(u32 x, u32 y, const EChunkTypes Type)
{
    switch (Type)
    {
        case CHUNK_LEFT:
            createVertex(x    , y + 2);
            createVertex(x + 2, y + 2);
            createVertex(x    , y + 1);
            createVertex(x + 2, y    );
            createVertex(x    , y    );
            break;
            
        case CHUNK_RIGHT:
            createVertex(x + 2, y    );
            createVertex(x    , y    );
            createVertex(x + 2, y + 1);
            createVertex(x    , y + 2);
            createVertex(x + 2, y + 2);
            break;
            
        case CHUNK_TOP:
            createVertex(x + 2, y + 2);
            createVertex(x + 2, y    );
            createVertex(x + 1, y + 2);
            createVertex(x    , y    );
            createVertex(x    , y + 2);
            break;
            
        case CHUNK_BOTTOM:
            createVertex(x    , y    );
            createVertex(x    , y + 2);
            createVertex(x + 1, y    );
            createVertex(x + 2, y + 2);
            createVertex(x + 2, y    );
            break;
            
        default:
            return;
    }
    
    MeshBuffer_.addTriangle(0, 1, 2);
    MeshBuffer_.addTriangle(2, 1, 3);
    MeshBuffer_.addTriangle(3, 4, 2);
    
    MeshBuffer_.addIndexOffset(5);
}

void Terrain::createCorner(u32 x, u32 y, const EChunkTypes Type)
{
    switch (Type)
    {
        case CHUNK_LEFT_TOP:
            createVertex(x + 2, y    );
            createVertex(x    , y + 1);
            createVertex(x + 1, y + 2);
            createVertex(x    , y    );
            createVertex(x    , y + 2);
            createVertex(x + 2, y + 2);
            break;
            
        case CHUNK_LEFT_BOTTOM:
            createVertex(x + 2, y + 2);
            createVertex(x + 1, y    );
            createVertex(x    , y + 1);
            createVertex(x + 2, y    );
            createVertex(x    , y    );
            createVertex(x    , y + 2);
            break;
            
        case CHUNK_RIGHT_TOP:
            createVertex(x    , y    );
            createVertex(x + 1, y + 2);
            createVertex(x + 2, y + 1);
            createVertex(x    , y + 2);
            createVertex(x + 2, y + 2);
            createVertex(x + 2, y    );
            break;
            
        case CHUNK_RIGHT_BOTTOM:
            createVertex(x    , y + 2);
            createVertex(x + 2, y + 1);
            createVertex(x + 1, y    );
            createVertex(x + 2, y + 2);
            createVertex(x + 2, y    );
            createVertex(x    , y    );
            break;
            
        default:
            return;
    }
    
    MeshBuffer_.addTriangle(0, 1, 2);
    MeshBuffer_.addTriangle(0, 3, 1);
    MeshBuffer_.addTriangle(1, 4, 2);
    MeshBuffer_.addTriangle(0, 2, 5);
    
    MeshBuffer_.addIndexOffset(6);
}

bool Terrain::isPosCorner(const VertexPos &Pos, const EChunkTypes CornerType) const
{
    switch (CornerType)
    {
        case CHUNK_LEFT_TOP:
            return Pos.X == 0 && Pos.Y + 1 == GridSize_;
        case CHUNK_LEFT_BOTTOM:
            return Pos.X == 0 && Pos.Y == 0;
        case CHUNK_RIGHT_TOP:
            return Pos.X + 1 == GridSize_ && Pos.Y + 1 == GridSize_;
        case CHUNK_RIGHT_BOTTOM:
            return Pos.X + 1 == GridSize_ && Pos.Y == 0;
        default:
            break;
    }
    return false;
}

bool Terrain::isPosEdge(const VertexPos &Pos, const EChunkTypes EdgeType) const
{
    switch (EdgeType)
    {
        case CHUNK_LEFT:
            return Pos.X == 0;
        case CHUNK_RIGHT:
            return Pos.X + 1 == GridSize_;
        case CHUNK_TOP:
            return Pos.Y + 1 == GridSize_;
        case CHUNK_BOTTOM:
            return Pos.Y == 0;
        default:
            break;
    }
    return false;
}

Terrain::EChunkTypes Terrain::getChunkEdgeType(const VertexPos &Pos, const EChunkTypes CornerType) const
{
    switch (CornerType)
    {
        case CHUNK_LEFT_TOP:
            return Pos.X == 0 ? CHUNK_LEFT : CHUNK_TOP;
        case CHUNK_LEFT_BOTTOM:
            return Pos.X == 0 ? CHUNK_LEFT : CHUNK_BOTTOM;
        case CHUNK_RIGHT_TOP:
            return Pos.X + 1 == GridSize_ ? CHUNK_RIGHT : CHUNK_TOP;
        case CHUNK_RIGHT_BOTTOM:
            return Pos.X + 1 == GridSize_ ? CHUNK_RIGHT : CHUNK_BOTTOM;
        default:
            break;
    }
    return CHUNK_BASE;
}

void Terrain::drawChunk(const dim::vector3df &GlobalCamPos, dim::matrix4f Transform, u8 GeoMIPLevel)
{
    
    //todo ...
    
}


#endif


} // /namespace scene

} // /namespace sp



// ================================================================================
