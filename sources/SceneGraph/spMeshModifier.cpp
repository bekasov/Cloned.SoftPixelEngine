/*
 * Mesh modifier file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "SceneGraph/spMeshModifier.hpp"
#include "SceneGraph/spSceneMesh.hpp"
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

SP_EXPORT void meshFit(Mesh &Obj, const dim::vector3df &Position, const dim::vector3df &Size)
{
    const f32 Max = 999999.0f;
    
    dim::vector3df MinPos(Max, Max, Max);
    dim::vector3df MaxPos(-Max, -Max, -Max);
    dim::vector3df Pos;
    
    /* Find the dimensions */
    for (u32 s = 0; s < Obj.getOrigMeshBufferCount(); ++s)
    {
        video::MeshBuffer* Surface = Obj.getOrigMeshBuffer(s);
        
        for (u32 i = 0; i < Surface->getVertexCount(); ++i)
        {
            Pos = Surface->getVertexCoord(i);
            
            if (Pos.X < MinPos.X) MinPos.X = Pos.X;
            if (Pos.Y < MinPos.Y) MinPos.Y = Pos.Y;
            if (Pos.Z < MinPos.Z) MinPos.Z = Pos.Z;
            
            if (Pos.X > MaxPos.X) MaxPos.X = Pos.X;
            if (Pos.Y > MaxPos.Y) MaxPos.Y = Pos.Y;
            if (Pos.Z > MaxPos.Z) MaxPos.Z = Pos.Z;
        }
    }
    
    /* Fit the mesh */
    for (u32 s = 0; s < Obj.getOrigMeshBufferCount(); ++s)
    {
        video::MeshBuffer* Surface = Obj.getOrigMeshBuffer(s);
        
        for (u32 i = 0; i < Surface->getVertexCount(); ++i)
        {
            Pos = Surface->getVertexCoord(i);
            
            Pos -= MinPos;
            Pos /= (MaxPos - MinPos);
            Pos *= Size;
            Pos += Position;
            
            Surface->setVertexCoord(i, Pos);
        }
        Surface->updateNormals(Obj.getMaterial()->getShading());
    }
}

SP_EXPORT void meshSpherify(Mesh &Obj, f32 Factor)
{
    math::Clamp(Factor, -1.0f, 1.0f);
    
    dim::vector3df OrigPos, NormPos;
    
    for (u32 s = 0; s < Obj.getOrigMeshBufferCount(); ++s)
    {
        video::MeshBuffer* Surface = Obj.getOrigMeshBuffer(s);
        
        for (u32 i = 0; i < Surface->getVertexCount(); ++i)
        {
            OrigPos = Surface->getVertexCoord(i);
            NormPos = OrigPos;
            NormPos.normalize();
            
            Surface->setVertexCoord(i, OrigPos * (1.0f - Factor) + NormPos * Factor);
        }
        Surface->updateNormals(Obj.getMaterial()->getShading());
    }
}

SP_EXPORT void meshTwist(Mesh &Obj, f32 Rotation)
{
    dim::matrix4f Mat;
    dim::vector3df Pos;
    f32 Factor;
    
    const dim::aabbox3df BoundBox(Obj.getMeshBoundingBox());
    
    const f32 MinHeight = BoundBox.Min.Y;
    const f32 MaxHeight = BoundBox.Max.Y - MinHeight;
    
    for (u32 s = 0; s < Obj.getOrigMeshBufferCount(); ++s)
    {
        video::MeshBuffer* Surface = Obj.getOrigMeshBuffer(s);
        
        for (u32 i = 0; i < Surface->getVertexCount(); ++i)
        {
            Pos     = Surface->getVertexCoord(i);
            Factor  = (Pos.Y - MinHeight) / MaxHeight;
            
            Mat.reset();
            Mat.rotateY(Rotation * Factor);
            
            Surface->setVertexCoord(i, Mat * Pos);
        }
        Surface->updateNormals(Obj.getMaterial()->getShading());
    }
}

} // /namespace MeshModifier


} // /namespace scene

} // /namespace sp



// ================================================================================
