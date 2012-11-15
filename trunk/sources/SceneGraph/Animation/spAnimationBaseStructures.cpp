/*
 * Animation base structures file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "SceneGraph/Animation/spAnimationBaseStructures.hpp"
#include "SceneGraph/spSceneMesh.hpp"


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
    scene::Mesh* BaseMesh, u32 SurfaceIndex, u32 VertexIndex, f32 VertexWeight) :
    Surface (SurfaceIndex   ),
    Index   (VertexIndex    ),
    Weight  (VertexWeight   )
{
    setupVertex(BaseMesh);
}
SVertexGroup::SVertexGroup(
    scene::Mesh* BaseMesh, u32 SurfaceIndex, u32 VertexIndex,
    u8 TangentTexLayer, u8 BinormalTexLayer, f32 VertexWeight) :
    Surface (SurfaceIndex   ),
    Index   (VertexIndex    ),
    Weight  (VertexWeight   )
{
    setupVertex(BaseMesh, TangentTexLayer, BinormalTexLayer);
}
SVertexGroup::~SVertexGroup()
{
}

void SVertexGroup::setupVertex(scene::Mesh* BaseMesh)
{
    if (BaseMesh)
    {
        video::MeshBuffer* Surf = BaseMesh->getMeshBuffer(Surface);
        
        if (Surf)
        {
            Position    = Surf->getVertexCoord  (Index);
            Normal      = Surf->getVertexNormal (Index);
        }
    }
}

void SVertexGroup::setupVertex(scene::Mesh* BaseMesh, u8 TangentTexLayer, u8 BinormalTexLayer)
{
    if (BaseMesh)
    {
        video::MeshBuffer* Surf = BaseMesh->getMeshBuffer(Surface);
        
        if (Surf)
        {
            Position    = Surf->getVertexCoord      (Index                  );
            Normal      = Surf->getVertexNormal     (Index                  );
            Tangent     = Surf->getVertexTexCoord   (Index, TangentTexLayer );
            Binormal    = Surf->getVertexTexCoord   (Index, BinormalTexLayer);
        }
    }
}


} // /namespace scene

} // /namespace sp



// ================================================================================
