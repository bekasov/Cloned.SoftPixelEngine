/*
 * Terrain scene node file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "SceneGraph/spSceneTerrain.hpp"
#include "Platform/spSoftPixelDeviceOS.hpp"


namespace sp
{

extern video::RenderSystem* __spVideoDriver;
extern scene::SceneGraph* __spSceneManager;

namespace scene
{


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
    init();
}
Terrain::~Terrain()
{
    clear();
}

void Terrain::render()
{
    /* Matrix transformation */
    loadTransformation();
    
    GlobalTerrainTransformation_    = spWorldMatrix;
    GlobalCamPosition_              = __spSceneManager->getActiveCamera()->getPosition(true);
    
    /* Update the render matrix */
    __spVideoDriver->updateModelviewMatrix();
    
    /* Setup material states */
    if (EnableMaterial_)
        __spVideoDriver->setupMaterialStates(getMaterial());
    __spVideoDriver->setupShaderClass(this, getShaderClass());
    
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
    __spVideoDriver->unbindShaders();
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
    
    init();
}


/*
 * ======= Private: =======
 */

void Terrain::init()
{
    if (!MeshTexReference_)
        MeshTexReference_ = new video::MeshBuffer();
    
    s32 MIPLevel    = 0;
    
    Resolution_     = 1 << (GeoMIPLevels_ - 1);
    
    RootTreeNode_   = new QuadTreeNode();
    
    createQuadTree(RootTreeNode_, MIPLevel, 0);
    
    RenderNodeList_.resize(RenderModeListSize_);
    RenderModeListSize_ = 0;
}
void Terrain::clear()
{
    HeightMap_.clearBuffer();
    MemoryManager::deleteMemory(RootTreeNode_);
    MemoryManager::deleteMemory(MeshTexReference_);
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
    
    if (MIPLevel < GeoMIPLevels_ && NodeData->Resolution > 1)
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
        dim::vector3df(NodeData->Center.X, 0.0f, NodeData->Center.Y) * Scale_ + Position_
    );
    
    /* Add to the render node list */
    if (d > 7.0f * (GeoMIPLevels_ - NodeData->MIPLevel) || Node->isLeaf() ||
        !checkFurstumCulling(__spSceneManager->getActiveCamera()->getViewFrustum(), GlobalTerrainTransformation_, NodeData->BoundBox))
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
    f32 x = (f32)NodeData->Resolution.Width / Resolution_.Width;
    f32 z = (f32)NodeData->Resolution.Height / Resolution_.Height;
    
    /* Deform the node mesh */
    NodeData->recreateBottom( deformTreeNodeMesh(Node, NodeData->Center + dim::point2df( 0, -z)), MeshResolution_ );
    NodeData->recreateTop   ( deformTreeNodeMesh(Node, NodeData->Center + dim::point2df( 0,  z)), MeshResolution_ );
    NodeData->recreateLeft  ( deformTreeNodeMesh(Node, NodeData->Center + dim::point2df(-x,  0)), MeshResolution_ );
    NodeData->recreateRight ( deformTreeNodeMesh(Node, NodeData->Center + dim::point2df( x,  0)), MeshResolution_ );
    
    /* Render the mesh buffer using the unique texture list */
    __spVideoDriver->drawMeshBuffer(&NodeData->Mesh);
}

bool Terrain::deformTreeNodeMesh(QuadTreeNode* Node, const dim::point2df &Pos)
{
    return getNodeLevel(RootTreeNode_, Pos) < static_cast<STreeNodeData*>(Node->getUserData())->MIPLevel;
}

s32 Terrain::getNodeLevel(const QuadTreeNode* Node, const dim::point2df &Pos)
{
    /* Temporary variables */
    STreeNodeData* NodeData = (STreeNodeData*)Node->getUserData();
    
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

Terrain::STreeNodeData::STreeNodeData() : MIPLevel(0), Selected(false)
{
    Mesh.createMeshBuffer();
}
Terrain::STreeNodeData::~STreeNodeData()
{
    Mesh.deleteMeshBuffer();
}

// !!! TODO: less LoC !!!

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


} // /namespace scene

} // /namespace sp



// ================================================================================
