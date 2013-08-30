/*
 * kd-Tree buffer mapper header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_KDTREE_BUFFER_MAPPER_H__
#define __SP_KDTREE_BUFFER_MAPPER_H__


#include "Base/spStandard.hpp"

#ifdef SP_COMPILE_WITH_LIGHTMAPGENERATOR


#include "Base/spTreeNodeKD.hpp"


namespace sp
{
namespace scene
{
    class CollisionMesh;
}
namespace video
{
    class ShaderResource;
}
namespace tool
{


/**
kd-Tree buffer mapper used for hardware-accelerated lightmap generation.
It maps a kd-Tree hierarchy and their data into a shader resource buffer.
\since Version 3.3
*/
namespace KDTreeBufferMapper
{

/**
Maps the specified kd-Tree and its data into the specified shader resources.
\param[in] CollisionObject Constant pointer to the collision mesh object.
\param[out] BufferNodeList Pointer to the shader resource where the kd-Tree node are to be stored.
This must be a structured buffer with the following structure:
\code
struct SKDTreeNode
{
    int Axis;            // kd-Tree axis (0 -> X, 1 -> Y, 2 -> Z).
    float Distance;      // Axis splitting value.
    uint TriangleStart;  // Triangle start index (0xFFFFFFFF means no triangle data, in this case the children IDs must be valid).
    uint NumTriangles;   // Number of triangle IDs.
    uint ChildIds[2];    // Pointers (or rather indices) to the child nodes.
};
\endcode
\param[out] BufferTriangleIdList Pointer to the shader resource where the triangle IDs for each kd-Tree node are to be stored.
This must be a standard buffer with 32-bit unsigned integers.
\param[out] BufferTriangleList Pointer to the shader resource where the triangles are to be stored.
This must be a structured buffer with the following structure:
\code
struct STriangle
{
    float3 A, B, C; // 3D coordinates for the three vertices A, B and C.
};
\endcode
\return True if mapping was successful.
\see scene::KDTreeNode
\see video::ShaderResource
\see scene::CollisionMesh
*/
bool SP_EXPORT copyTreeHierarchy(
    const scene::CollisionMesh* CollisionObject,
    video::ShaderResource* BufferNodeList,
    video::ShaderResource* BufferTriangleIdList,
    video::ShaderResource* BufferTriangleList
);

} // /namespace KDTreeBufferMapper


} // /namespace tool

} // /namespace sp


#endif

#endif



// ================================================================================
