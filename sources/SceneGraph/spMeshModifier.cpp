/*
 * Mesh modifier file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "SceneGraph/spMeshModifier.hpp"
#include "Base/spMeshBuffer.hpp"


namespace sp
{
namespace scene
{


namespace MeshModifier
{

SP_EXPORT void meshTranslate(video::MeshBuffer &Surface, const dim::vector3df &Direction)
{
    const u32 VertexCount = Surface.getVertexCount();
    
    for (u32 i = 0; i < VertexCount; ++i)
        Surface.setVertexCoord(i, Direction + Surface.getVertexCoord(i));
    
    Surface.updateVertexBuffer();
}

SP_EXPORT void meshTransform(video::MeshBuffer &Surface, const dim::vector3df &Size)
{
    const u32 VertexCount = Surface.getVertexCount();
    
    for (u32 i = 0; i < VertexCount; ++i)
        Surface.setVertexCoord(i, Size * Surface.getVertexCoord(i));
    
    Surface.updateVertexBuffer();
}

SP_EXPORT void meshTransform(video::MeshBuffer &Surface, const dim::matrix4f &Matrix)
{
    const dim::matrix4f Rotation(dim::getRotationMatrix(Matrix));
    
    const u32 VertexCount = Surface.getVertexCount();
    
    for (u32 i = 0; i < VertexCount; ++i)
        Surface.setVertexCoord(i, Matrix * Surface.getVertexCoord(i));
    
    if (Surface.getVertexFormat()->getFlags() & video::VERTEXFORMAT_NORMAL)
    {
        for (u32 i = 0; i < VertexCount; ++i)
            Surface.setVertexNormal(i, (Rotation * Surface.getVertexNormal(i)).normalize());
    }
    
    Surface.updateVertexBuffer();
}

SP_EXPORT void meshTurn(video::MeshBuffer &Surface, const dim::vector3df &Rotation)
{
    meshTransform(Surface, dim::getRotationMatrix(Rotation));
}

SP_EXPORT void meshFlip(video::MeshBuffer &Surface)
{
    const u32 VertexCount = Surface.getVertexCount();
    
    for (u32 i = 0; i < VertexCount; ++i)
        Surface.setVertexCoord(i, Surface.getVertexCoord(i).getInvert());
    
    if (Surface.getVertexFormat()->getFlags() & video::VERTEXFORMAT_NORMAL)
    {
        for (u32 i = 0; i < VertexCount; ++i)
            Surface.setVertexNormal(i, Surface.getVertexNormal(i).getInvert());
    }
    
    Surface.updateVertexBuffer();
}

SP_EXPORT void meshFlip(video::MeshBuffer &Surface, bool isXAxis, bool isYAxis, bool isZAxis)
{
    if (!isXAxis && !isYAxis && !isZAxis)
        return;
    
    dim::vector3df Pos, Normal;
    
    const u32 VertexCount = Surface.getVertexCount();
    
    for (u32 i = 0; i < VertexCount; ++i)
    {
        Pos     = Surface.getVertexCoord(i);
        Normal  = Surface.getVertexNormal(i);
        
        if (isXAxis)
            Pos.X = -Pos.X, Normal.X = -Normal.X;
        if (isYAxis)
            Pos.Y = -Pos.Y, Normal.Y = -Normal.Y;
        if (isZAxis)
            Pos.Z = -Pos.Z, Normal.Z = -Normal.Z;
        
        Surface.setVertexCoord(i, Pos);
        Surface.setVertexNormal(i, Normal);
    }
    
    Surface.updateVertexBuffer();
}

} // /namespace MeshModifier


} // /namespace scene

} // /namespace sp



// ================================================================================
