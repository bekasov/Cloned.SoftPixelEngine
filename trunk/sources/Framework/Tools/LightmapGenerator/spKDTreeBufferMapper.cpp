/*
 * kd-Tree buffer mapper file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "Framework/Tools/LightmapGenerator/spKDTreeBufferMapper.hpp"

#ifdef SP_COMPILE_WITH_LIGHTMAPGENERATOR


#include "Base/spInputOutputLog.hpp"
#include "Base/spDimensionVector3D.hpp"
#include "Base/spTreeNodeKD.hpp"
#include "RenderSystem/spShaderResource.hpp"
#include "SceneGraph/Collision/spCollisionMesh.hpp"

#include <boost/foreach.hpp>
#include <map>


namespace sp
{
namespace tool
{


namespace KDTreeBufferMapper
{


#ifdef _DEB_NEW_KDTREE_
#   error 'KDTreeBufferMapper' currently incompatible with new kd-Tree data type
#endif


/*
 * Structures
 */

#if defined(_MSC_VER)
#   pragma pack(push, packing)
#   pragma pack(1)
#   define SP_PACK_STRUCT
#elif defined(__GNUC__)
#   define SP_PACK_STRUCT __attribute__((packed))
#else
#   define SP_PACK_STRUCT
#endif

struct STriangleSR
{
    dim::float3 A, B, C;
}
SP_PACK_STRUCT;

struct SKDTreeNodeSR
{
    s32 Axis;
    f32 Distance;
    u32 TriangleStart;
    u32 NumTriangles;
    u32 ChildIds[2];
}
SP_PACK_STRUCT;

#ifdef _MSC_VER
#   pragma pack(pop, packing)
#endif

#undef SP_PACK_STRUCT

//! ID offset key structure for triangle ID hash-map.
struct SIdOffsetKey
{
    SIdOffsetKey() :
        MeshObj(0),
        Surface(0)
    {
    }
    SIdOffsetKey(const scene::Mesh* Obj, u32 SurfaceIndex) :
        MeshObj(Obj         ),
        Surface(SurfaceIndex)
    {
    }
    SIdOffsetKey(const SIdOffsetKey &Other) :
        MeshObj(Other.MeshObj),
        Surface(Other.Surface)
    {
    }
    ~SIdOffsetKey()
    {
    }
    
    /* Operators */
    inline bool operator < (const SIdOffsetKey &Other) const
    {
        if (MeshObj != Other.MeshObj)
        {
            #ifdef SP_COMPILER_VC
            return reinterpret_cast<uintptr_t>(MeshObj) < reinterpret_cast<uintptr_t>(Other.MeshObj);
            #else
            return reinterpret_cast<u32>(MeshObj) < reinterpret_cast<u32>(Other.MeshObj);
            #endif
        }
        return Surface < Other.Surface;
    }
    
    /* Members */
    const scene::Mesh* MeshObj;
    u32 Surface;
};

typedef std::map<SIdOffsetKey, u32> IdOffsetMapType;


/*
 * Declarations
 */

static bool copyTriangleList(
    const scene::CollisionMesh* CollisionObject, video::ShaderResource* BufferTriangleList, IdOffsetMapType &IdOffsetMap
);

static bool copyTreeNodeList(
    const scene::CollisionMesh* CollisionObject,
    video::ShaderResource* BufferNodeList, video::ShaderResource* BufferTriangleIdList,
    const IdOffsetMapType &IdOffsetMap
);


/*
 * Global functions
 */

SP_EXPORT bool copyTreeHierarchy(
    const scene::CollisionMesh* CollisionObject, video::ShaderResource* BufferNodeList,
    video::ShaderResource* BufferTriangleIdList, video::ShaderResource* BufferTriangleList)
{
    /* Valid input parameters */
    if (!CollisionObject || !BufferNodeList || !BufferTriangleIdList || !BufferTriangleList)
    {
        #ifdef SP_DEBUGMODE
        io::Log::debug("KDTreeBufferMapper::copyTreeHierarchy");
        #endif
        return false;
    }
    
    /* Map buffers to shader resources */
    IdOffsetMapType IdOffsetMap; 
    
    if (!copyTriangleList(CollisionObject, BufferTriangleList, IdOffsetMap))
    {
        io::Log::error("Mapping triangle list to shader resource failed");
        return false;
    }
    
    if (!copyTreeNodeList(CollisionObject, BufferNodeList, BufferTriangleIdList, IdOffsetMap))
    {
        io::Log::error("Mapping tree-node list and triangle-ID list to shader resources failed");
        return false;
    }
    
    return true;
}


/*
 * Internal functions
 */

static bool copyTriangleList(
    const scene::CollisionMesh* CollisionObject, video::ShaderResource* BufferTriangleList, IdOffsetMapType &IdOffsetMap)
{
    /* Get number of triangles */
    u32 NumTriangles = 0;
    
    foreach (scene::Mesh* MeshObj, CollisionObject->getMeshList())
        NumTriangles += MeshObj->getTriangleCount();
    
    /* Create CPU buffer */
    std::vector<STriangleSR> LocalBuffer(NumTriangles);
    
    u32 IdIndex = 0;
    u32 SurfIndex = 0;
    dim::triangle3df Tri;
    
    foreach (scene::Mesh* MeshObj, CollisionObject->getMeshList())
    {
        foreach (video::MeshBuffer* Surf, MeshObj->getMeshBufferList())
        {
            /* Store triangle start index in ID offset map */
            IdOffsetMap[SIdOffsetKey(MeshObj, SurfIndex)] = IdIndex;
            
            for (u32 j = 0; j < Surf->getTriangleCount(); ++j)
            {
                /* Get triangle coordinates */
                Tri = Surf->getTriangleCoords(j);
                
                /* Setup triangel data for buffer */
                STriangleSR& TriSR = LocalBuffer[IdIndex++];
                
                TriSR.A = Tri.PointA;
                TriSR.B = Tri.PointB;
                TriSR.C = Tri.PointC;
            }
            
            ++SurfIndex;
        }
    }
    
    /* Commit buffer to GPU */
    if (NumTriangles == 0)
    {
        io::Log::error("Can not create triangle buffer with zero triangles");
        return false;
    }
    
    BufferTriangleList->setupBuffer<STriangleSR>(NumTriangles, &LocalBuffer[0]);
    
    return true;
}

static bool copyTreeNodeListSub(
    const scene::KDTreeNode* Node, std::vector<SKDTreeNodeSR> &NodeBuffer,
    std::vector<u32> &IdBuffer, u32 &NodeIndex, u32 &IdIndex, const IdOffsetMapType &IdOffsetMap)
{
    typedef scene::CollisionMesh::TreeNodeDataType* NodeDataPtr;
    
    static const u32 ID_NONE = 0xFFFFFFFF;
    
    /* Store tree node information in node buffer */
    SKDTreeNodeSR& NodeEntry = NodeBuffer[NodeIndex];
    
    NodeEntry.Axis = static_cast<s32>(Node->getAxis());
    NodeEntry.Distance = Node->getDistance();
    
    /* Increment node index for further buffer access */
    ++NodeIndex;
    
    if (Node->isLeaf())
    {
        /* Initialize children node IDs */
        NodeEntry.ChildIds[0] = ID_NONE;
        NodeEntry.ChildIds[1] = ID_NONE;
        
        /* Get triangle information from tree node */
        NodeDataPtr TriangleList = reinterpret_cast<NodeDataPtr>(Node->getUserData());
        
        if (!TriangleList)
        {
            /* Initialize triangle information */
            NodeEntry.TriangleStart = 0;
            NodeEntry.NumTriangles = 0;
            return true;
        }
        
        /* Store triangle information */
        NodeEntry.TriangleStart = IdIndex;
        NodeEntry.NumTriangles = TriangleList->size();
        
        /* Resize ID buffer for following triangle IDs */
        IdBuffer.resize(IdBuffer.size() + TriangleList->size());
        
        /* Store ID for each triangle in the current tree node */
        foreach (scene::SCollisionFace* Face, *TriangleList)
        {
            /* Find ID in offset map */
            const SIdOffsetKey Key(Face->Mesh, Face->Surface);
            IdOffsetMapType::const_iterator it = IdOffsetMap.find(Key);
            
            if (it == IdOffsetMap.end())
            {
                /* Exit with error */
                io::Log::error("ID offset map corrupted during kd-Tree insertion for shader resource");
                return false;
            }
            
            /* Store triangle ID: start offset plus triangle index */
            IdBuffer[IdIndex++] = it->second + Face->Index;
        }
    }
    else
    {
        /* Initialize triangle information */
        NodeEntry.TriangleStart = 0;
        NodeEntry.NumTriangles = 0;
        
        /* Traverse child nodes */
        const scene::KDTreeNode* Children[2] =
        {
            dynamic_cast<const scene::KDTreeNode*>(Node->getChildNear()),
            dynamic_cast<const scene::KDTreeNode*>(Node->getChildFar())
        };
        
        if (Children[0] != 0 && Children[1] != 0)
        {
            /* Store chilren node IDs */
            NodeEntry.ChildIds[0] = NodeIndex;
            if (!copyTreeNodeListSub(Children[0], NodeBuffer, IdBuffer, NodeIndex, IdIndex, IdOffsetMap))
                return false;
            
            NodeEntry.ChildIds[1] = NodeIndex;
            if (!copyTreeNodeListSub(Children[1], NodeBuffer, IdBuffer, NodeIndex, IdIndex, IdOffsetMap))
                return false;
        }
    }
    
    return true;
}

static bool KDTreeBufferMapper::copyTreeNodeList(
    const scene::CollisionMesh* CollisionObject, video::ShaderResource* BufferNodeList,
    video::ShaderResource* BufferTriangleIdList, const IdOffsetMapType &IdOffsetMap)
{
    /* Get kd-Tree root node */
    const scene::KDTreeNode* RootTreeNode = CollisionObject->getRootTreeNode();
    
    /* Create CPU buffers */
    const u32 NumNodes = RootTreeNode->getNumChildren() + 1;
    
    std::vector<SKDTreeNodeSR> LocalNodeBuffer(NumNodes);
    std::vector<u32> LocalIdBuffer;
    
    /* Fill the buffer recursive */
    u32 NodeIndex = 0;
    u32 IdIndex = 0;
    
    if (!copyTreeNodeListSub(RootTreeNode, LocalNodeBuffer, LocalIdBuffer, NodeIndex, IdIndex, IdOffsetMap))
        return false;
    
    /* Commit buffers to GPU */
    if (LocalIdBuffer.empty())
    {
        io::Log::error("Can not create triangle ID buffer with zero IDs");
        return false;
    }
    
    BufferNodeList->setupBuffer<SKDTreeNodeSR>(NumNodes, &LocalNodeBuffer[0]);
    BufferTriangleIdList->setupBuffer<u32>(LocalIdBuffer.size(), &LocalIdBuffer[0]);
    
    return true;
}

} // /namespace KDTreeBufferMapper


} // /namespace tool

} // /namespace sp


#endif



// ================================================================================
