/*
 * Animation base structures header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_ANIMATION_BASESTRUCTURES_H__
#define __SP_ANIMATION_BASESTRUCTURES_H__


#include "Base/spStandard.hpp"
#include "Base/spMath.hpp"
#include "Base/spMeshBuffer.hpp"
#include "Base/spTransformation3D.hpp"


namespace sp
{
namespace scene
{


class AnimationJoint;


//! Describes how a vertex will be influenved by an AnimationJoint.
struct SP_EXPORT SVertexGroup
{
    SVertexGroup();
    SVertexGroup(
        video::MeshBuffer* MeshSurface, u32 VertexIndex,
        f32 VertexWeight = 1.0f
    );
    SVertexGroup(
        video::MeshBuffer* MeshSurface, u32 VertexIndex,
        u8 TangentTexLayer, u8 BinormalTexLayer,
        f32 VertexWeight = 1.0f
    );
    ~SVertexGroup();
    
    /* Functions */
    void setupVertex();
    void setupVertex(u8 TangentTexLayer, u8 BinormalTexLayer);
    
    /* Members */
    video::MeshBuffer* Surface; //!< Mesh buffer object.
    u32 Index;                  //!< Vertex index.
    f32 Weight;                 //!< Vertex weight factor.
    
    dim::vector3df Position;    //!< Original vertex position.
    dim::vector3df Normal;      //!< Original vertex normal.
    dim::vector3df Tangent;     //!< Original vertex tangent.
    dim::vector3df Binormal;    //!< Original vertex binormal.
};

//! Stores the keyframe coordinate and normal for a vertex.
struct SVertexKeyframe
{
    SVertexKeyframe()
    {
    }
    SVertexKeyframe(const dim::vector3df &VertexPosition, const dim::vector3df &VertexNormal) :
        Position(VertexPosition ),
        Normal  (VertexNormal   )
    {
    }
    ~SVertexKeyframe()
    {
    }
    
    /* Members */
    dim::vector3df Position;
    dim::vector3df Normal;
};

//! Describes how a vertex will be influenced by a MorphTargetAnimation.
struct SMorphTargetVertex
{
    SMorphTargetVertex() : Surface(0), Index(0)
    {
    }
    SMorphTargetVertex(
        video::MeshBuffer* MeshSurface, u32 VertexIndex,
        const std::vector<SVertexKeyframe> &VertexKeyframes) :
        Surface     (MeshSurface    ),
        Index       (VertexIndex    ),
        Keyframes   (VertexKeyframes)
    {
    }
    ~SMorphTargetVertex()
    {
    }
    
    /* Members */
    video::MeshBuffer* Surface;             //!< Mesh buffer object.
    u32 Index;                              //!< Vertex index.
    
    std::vector<SVertexKeyframe> Keyframes;
};

//! Stores the transformation and duration for a node animation keyframe.
struct SNodeKeyframe
{
    SNodeKeyframe(const Transformation &Trans, u64 FrameDuration) :
        Transform   (Trans                                          ),
        Duration    (math::Max(static_cast<u64>(1), FrameDuration)  )
    {
    }
    ~SNodeKeyframe()
    {
    }
    
    /* Members */
    Transformation Transform;   //!< Transformation of this keyframe.
    u64 Duration;               //!< Duration of this keyframe (in milliseconds).
};


} // /namespace scene

} // /namespace sp


#endif



// ================================================================================
