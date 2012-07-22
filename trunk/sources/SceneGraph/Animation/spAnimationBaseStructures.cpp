/*
 * Animation base structures file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "SceneGraph/Animation/spAnimationBaseStructures.hpp"


namespace sp
{
namespace scene
{


/*
 * SVertexGroup structure
 */

SVertexGroup::SVertexGroup() :
    Surface (0      ),
    Index   (0      ),
    Weight  (0.0f   )
{
}
SVertexGroup::SVertexGroup(
    video::MeshBuffer* MeshSurface, u32 VertexIndex, f32 VertexWeight) :
    Surface (MeshSurface    ),
    Index   (VertexIndex    ),
    Weight  (VertexWeight   )
{
    setupVertex();
}
SVertexGroup::SVertexGroup(
    video::MeshBuffer* MeshSurface, u32 VertexIndex,
    u8 TangentTexLayer, u8 BinormalTexLayer, f32 VertexWeight) :
    Surface (MeshSurface    ),
    Index   (VertexIndex    ),
    Weight  (VertexWeight   )
{
    setupVertex(TangentTexLayer, BinormalTexLayer);
}
SVertexGroup::~SVertexGroup()
{
}

void SVertexGroup::setupVertex()
{
    if (Surface)
    {
        Position    = Surface->getVertexCoord   (Index);
        Normal      = Surface->getVertexNormal  (Index);
    }
}

void SVertexGroup::setupVertex(u8 TangentTexLayer, u8 BinormalTexLayer)
{
    if (Surface)
    {
        Position    = Surface->getVertexCoord   (Index);
        Normal      = Surface->getVertexNormal  (Index);
        Tangent     = Surface->getVertexTexCoord(Index, TangentTexLayer);
        Binormal    = Surface->getVertexTexCoord(Index, BinormalTexLayer);
    }
}


} // /namespace scene

} // /namespace sp



// ================================================================================
