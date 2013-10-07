/*
 * Terrain scene node file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "SceneGraph/spSceneTerrain.hpp"
#include "Platform/spSoftPixelDeviceOS.hpp"
#include "RenderSystem/spTextureBase.hpp"
#include "Base/spMathCollisionLibrary.hpp"


namespace sp
{

extern video::RenderSystem* GlbRenderSys;
extern scene::SceneGraph* GlbSceneGraph;

namespace scene
{


Terrain::Terrain() :
    MaterialNode    (NODE_TERRAIN   ),
    GridSize_       (0              ),
    GeoMIPLevels_   (0              ),
    VertIndex_      (0              )
{
}
Terrain::~Terrain()
{
}

void Terrain::render()
{
    if (!GlbSceneGraph->getActiveCamera())
        return;
    
    /* Setup material states */
    GlbRenderSys->setupMaterialStates(getMaterial());
    GlbRenderSys->setupShaderClass(this, getShaderClass());
    
    /* Get global camera position and initial mesh buffer patch transformation */
    const dim::vector3df GlobalCamPos = GlbSceneGraph->getActiveCamera()->getPosition(true);
    dim::matrix4f Transform = getTransformMatrix(true);
    
    /* Render terrain mesh buffer patches */
    GlbRenderSys->bindMeshBuffer(&MeshBuffer_);
    {
        drawChunk(GlobalCamPos, Transform, dim::matrix4f::IDENTITY, GeoMIPLevels_);
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
    MeshBuffer_.createMeshBuffer();
    
    /* Create grid chunks */
    createChunkBase();
    
    for (u32 i = 0; i < 4; ++i)
        createChunkEdge(static_cast<EChunkTypes>(CHUNK_LEFT + i));
    
    for (u32 i = 0; i < 4; ++i)
        createChunkCorner(static_cast<EChunkTypes>(CHUNK_LEFT_TOP + i));
    
    /* Complete mesh creation */
    MeshBuffer_.updateMeshBuffer();
    
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
            createQuad(Pos.X*2, Pos.Y*2);
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
        for (Pos.X = 0; Pos.X < GridSize_; ++Pos.X)
        {
            if (isPosEdge(Pos, Type))
                createEdge(Pos.X*2, Pos.Y*2, Type);
            else
                createQuad(Pos.X*2, Pos.Y*2);
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
                createCorner(Pos.X*2, Pos.Y*2, Type);
            else if (isPosEdge(Pos, getChunkEdgeType(Pos, Type)))
                createEdge(Pos.X*2, Pos.Y*2, getChunkEdgeType(Pos, Type));
            else
                createQuad(Pos.X*2, Pos.Y*2);
        }
    }
    
    Chunk.NumVertices = MeshBuffer_.getIndexOffset() - Chunk.StartOffset;
    
    GridChunks_[Type] = Chunk;
}

void Terrain::addVertex(u32 x, u32 y)
{
    if (VertIndex_ < 6)
    {
        Verts_[VertIndex_].X = InvGridSize_ * static_cast<f32>(x) * 0.5f;
        Verts_[VertIndex_].Z = InvGridSize_ * static_cast<f32>(y) * 0.5f;
        ++VertIndex_;
    }
}

void Terrain::resetVertexIndex()
{
    VertIndex_ = 0;
}

void Terrain::createTriangle(u32 i0, u32 i1, u32 i2)
{
    MeshBuffer_.addVertex(Verts_[i0], dim::point2df(Verts_[i0].X, Verts_[i0].Z));
    MeshBuffer_.addVertex(Verts_[i1], dim::point2df(Verts_[i1].X, Verts_[i1].Z));
    MeshBuffer_.addVertex(Verts_[i2], dim::point2df(Verts_[i2].X, Verts_[i2].Z));
    
    MeshBuffer_.addIndexOffset(3);
}

void Terrain::createQuad(u32 x, u32 y)
{
    addVertex(x    , y    );
    addVertex(x    , y + 2);
    addVertex(x + 2, y + 2);
    addVertex(x + 2, y    );
    
    createTriangle(0, 1, 2);
    createTriangle(0, 2, 3);
    
    resetVertexIndex();
}

void Terrain::createEdge(u32 x, u32 y, const EChunkTypes Type)
{
    switch (Type)
    {
        case CHUNK_LEFT:
            addVertex(x    , y + 2);
            addVertex(x + 2, y + 2);
            addVertex(x    , y + 1);
            addVertex(x + 2, y    );
            addVertex(x    , y    );
            break;
            
        case CHUNK_RIGHT:
            addVertex(x + 2, y    );
            addVertex(x    , y    );
            addVertex(x + 2, y + 1);
            addVertex(x    , y + 2);
            addVertex(x + 2, y + 2);
            break;
            
        case CHUNK_TOP:
            addVertex(x + 2, y + 2);
            addVertex(x + 2, y    );
            addVertex(x + 1, y + 2);
            addVertex(x    , y    );
            addVertex(x    , y + 2);
            break;
            
        case CHUNK_BOTTOM:
            addVertex(x    , y    );
            addVertex(x    , y + 2);
            addVertex(x + 1, y    );
            addVertex(x + 2, y + 2);
            addVertex(x + 2, y    );
            break;
            
        default:
            return;
    }
    
    createTriangle(0, 1, 2);
    createTriangle(2, 1, 3);
    createTriangle(3, 4, 2);
    
    resetVertexIndex();
}

void Terrain::createCorner(u32 x, u32 y, const EChunkTypes Type)
{
    switch (Type)
    {
        case CHUNK_LEFT_TOP:
            addVertex(x + 2, y    );
            addVertex(x    , y + 1);
            addVertex(x + 1, y + 2);
            addVertex(x    , y    );
            addVertex(x    , y + 2);
            addVertex(x + 2, y + 2);
            break;
            
        case CHUNK_LEFT_BOTTOM:
            addVertex(x + 2, y + 2);
            addVertex(x + 1, y    );
            addVertex(x    , y + 1);
            addVertex(x + 2, y    );
            addVertex(x    , y    );
            addVertex(x    , y + 2);
            break;
            
        case CHUNK_RIGHT_TOP:
            addVertex(x    , y    );
            addVertex(x + 1, y + 2);
            addVertex(x + 2, y + 1);
            addVertex(x    , y + 2);
            addVertex(x + 2, y + 2);
            addVertex(x + 2, y    );
            break;
            
        case CHUNK_RIGHT_BOTTOM:
            addVertex(x    , y + 2);
            addVertex(x + 2, y + 1);
            addVertex(x + 1, y    );
            addVertex(x + 2, y + 2);
            addVertex(x + 2, y    );
            addVertex(x    , y    );
            break;
            
        default:
            return;
    }
    
    createTriangle(0, 1, 2);
    createTriangle(0, 3, 1);
    createTriangle(1, 4, 2);
    createTriangle(0, 2, 5);
    
    resetVertexIndex();
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

void Terrain::drawChunk(
    const dim::vector3df &GlobalCamPos, dim::matrix4f GlobalTransform,
    dim::matrix4f LocalTransform, u8 GeoMIPLevel, const ETranslateDirections Translate)
{
    /* Translate transformation */
    switch (Translate)
    {
        case TRANSLATE_TOP:
            GlobalTransform .translate(dim::vector3df(0, 0, 1));
            LocalTransform  .translate(dim::vector3df(0, 0, 1));
            break;
        case TRANSLATE_RIGHT:
            GlobalTransform .translate(dim::vector3df(1, 0, 0));
            LocalTransform  .translate(dim::vector3df(1, 0, 0));
            break;
        case TRANSLATE_RIGHT_TOP:
            GlobalTransform .translate(dim::vector3df(1, 0, 1));
            LocalTransform  .translate(dim::vector3df(1, 0, 1));
            break;
        default:
            break;
    }
    
    /* Check view-frustum culling */
    if ( GlbSceneGraph && GlbSceneGraph->getActiveCamera() &&
        !GlbSceneGraph->getActiveCamera()->getViewFrustum().isBoundBoxInside(dim::aabbox3df(0.0f, 1.0f), GlobalTransform) )
    {
        return;
    }
    
    if (GeoMIPLevel > 0 && getSubDivision(GlobalCamPos, GlobalTransform) < GeoMIPLevel)
    {
        /* Scale down transformation */
        GlobalTransform .scale(dim::vector3df(0.5f, 1.0f, 0.5f));
        LocalTransform  .scale(dim::vector3df(0.5f, 1.0f, 0.5f));
        
        /* Draw sub chunks */
        drawChunk(GlobalCamPos, GlobalTransform, LocalTransform, GeoMIPLevel - 1, TRANSLATE_NONE        );
        drawChunk(GlobalCamPos, GlobalTransform, LocalTransform, GeoMIPLevel - 1, TRANSLATE_TOP         );
        drawChunk(GlobalCamPos, GlobalTransform, LocalTransform, GeoMIPLevel - 1, TRANSLATE_RIGHT       );
        drawChunk(GlobalCamPos, GlobalTransform, LocalTransform, GeoMIPLevel - 1, TRANSLATE_RIGHT_TOP   );
    }
    else
    {
        drawChunkLeaf(
            GlobalTransform, LocalTransform, getChunkType(GlobalCamPos, GlobalTransform, GeoMIPLevel)
        );
    }
}

void Terrain::drawChunkLeaf(
    const dim::matrix4f &GlobalTransform, const dim::matrix4f &LocalTransform, const EChunkTypes Type)
{
    /* Update world matrix transformation */
    GlbRenderSys->setWorldMatrix(GlobalTransform);
    GlbRenderSys->setTextureMatrix(LocalTransform);
    
    GlbRenderSys->updateModelviewMatrix();
    
    /* Get grid chunk */
    SGridChunk& Chunk = GridChunks_[Type];
    
    /* Draw mesh buffer chunk */
    GlbRenderSys->drawMeshBufferPart(&MeshBuffer_, Chunk.StartOffset, Chunk.NumVertices);
}

dim::vector3df Terrain::getCenter(dim::matrix4f Transform) const
{
    Transform.translate(dim::vector3df(0.5f, 0, 0.5f));
    return Transform.getPosition();
}

u8 Terrain::getSubDivision(const dim::vector3df &GlobalCamPos, const dim::matrix4f &Transform) const
{
    dim::obbox3df Box = Transform * dim::obbox3df(0.0f, 1.0f);
    
    f32 Dist = math::CollisionLibrary::getPointBoxDistance(Box, GlobalCamPos);
    
    f32 LOD = sqrt(Dist) / (static_cast<f32>(GeoMIPLevels_) * 0.5f);
    
    return static_cast<u8>(math::MinMax(static_cast<s32>(LOD), 0, static_cast<s32>(GeoMIPLevels_)));
}

u8 Terrain::getNeighbourMIPLevel(
    const dim::vector3df &GlobalCamPos, dim::matrix4f Transform, const ETranslateDirections Translate) const
{
    switch (Translate)
    {
        case TRANSLATE_LEFT:
            Transform.translate(dim::vector3df(-1, 0, 0));
            break;
        case TRANSLATE_RIGHT:
            Transform.translate(dim::vector3df(1, 0, 0));
            break;
        case TRANSLATE_TOP:
            Transform.translate(dim::vector3df(0, 0, 1));
            break;
        case TRANSLATE_BOTTOM:
            Transform.translate(dim::vector3df(0, 0, -1));
            break;
        default:
            break;
    }
    
    return getSubDivision(GlobalCamPos, Transform);
}

Terrain::EChunkTypes Terrain::getChunkType(
    const dim::vector3df &GlobalCamPos, const dim::matrix4f &Transform, u8 GeoMIPLevel) const
{
    u8 MIPLevel[4] =
    {
        getNeighbourMIPLevel(GlobalCamPos, Transform, TRANSLATE_LEFT),
        getNeighbourMIPLevel(GlobalCamPos, Transform, TRANSLATE_RIGHT),
        getNeighbourMIPLevel(GlobalCamPos, Transform, TRANSLATE_TOP),
        getNeighbourMIPLevel(GlobalCamPos, Transform, TRANSLATE_BOTTOM)
    };
    
    bool LevelHigher[4] =
    {
        (MIPLevel[0] < GeoMIPLevel),
        (MIPLevel[1] < GeoMIPLevel),
        (MIPLevel[2] < GeoMIPLevel),
        (MIPLevel[3] < GeoMIPLevel)
    };
    
    if (LevelHigher[0])
    {
        if (LevelHigher[2])
            return CHUNK_LEFT_TOP;
        else if (LevelHigher[3])
            return CHUNK_LEFT_BOTTOM;
        else
            return CHUNK_LEFT;
    }
    else if (LevelHigher[1])
    {
        if (LevelHigher[2])
            return CHUNK_RIGHT_TOP;
        else if (LevelHigher[3])
            return CHUNK_RIGHT_BOTTOM;
        else
            return CHUNK_RIGHT;
    }
    else if (LevelHigher[2])
        return CHUNK_TOP;
    else if (LevelHigher[3])
        return CHUNK_BOTTOM;
    
    return CHUNK_BASE;
}


} // /namespace scene

} // /namespace sp



// ================================================================================
