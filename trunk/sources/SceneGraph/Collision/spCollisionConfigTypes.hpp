/*
 * Collision configuration types header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_COLLISION_CONFIG_TYPES_H__
#define __SP_COLLISION_CONFIG_TYPES_H__


#include "Base/spStandard.hpp"
#include "Base/spMeshBuffer.hpp"
#include "SceneGraph/spSceneMesh.hpp"


namespace sp
{
namespace scene
{


/*
 * Pre-declerations
 */

class Collision; // !deprecated!
class CollisionDetector; // !deprecated!
class CollisionMaterial;
class CollisionNode;
class CollisionSphere;
class CollisionCapsule;
class CollisionCylinder;
class CollisionCone;
class CollisionBox;
class CollisionPlane;
class CollisionMesh;


/*
 * Enumerations
 */

//! Collision models.
enum ECollisionModels
{
    COLLISION_SPHERE,   //!< Collision sphere with position and radius.
    COLLISION_CAPSULE,  //!< Collision capsule with position, rotation, radius and height.
    COLLISION_CYLINDER, //!< Collision cylinder with position, rotation, radius and height.
    COLLISION_CONE,     //!< Collision cone with position, rotation, radius and height.
    COLLISION_BOX,      //!< Collision box with position, rotation and axis-alined-bounding-box.
    COLLISION_PLANE,    //!< Collision plane with position and normal vector.
    COLLISION_MESH,     //!< Collision mesh using kd-Tree.
};

//! Flags for collision detection.
enum ECollisionFlags
{
    COLLISIONFLAG_NONE              = 0x00,                             //!< No collision detection, resolving and intersection tests will be performed.
    
    COLLISIONFLAG_DETECTION         = 0x01,                             //!< Collision detection is performed.
    COLLISIONFLAG_RESOLVE           = 0x02 | COLLISIONFLAG_DETECTION,   //!< Collision resolving is performed.
    COLLISIONFLAG_INTERSECTION      = 0x04,                             //!< Intersection tests are performed.
    COLLISIONFLAG_PERMANENT_UPDATE  = 0x08,                             //!< Collision detection will be performed every frame. Otherwise only when the object has been moved.
    
    //! Collision resolving and intersection tests are performed.
    COLLISIONFLAG_FULL =
        COLLISIONFLAG_RESOLVE |
        COLLISIONFLAG_INTERSECTION |
        COLLISIONFLAG_PERMANENT_UPDATE,
};

//! Flags for collision detection support to rival collision nodes.
enum ECollisionSupportFlags
{
    COLLISIONSUPPORT_NONE       = 0x00, //!< Collision to no rival collision node is supported.
    
    COLLISIONSUPPORT_SPHERE     = 0x01, //!< Collision to sphere is supported.
    COLLISIONSUPPORT_CAPSULE    = 0x02, //!< Collision to capsule is supported.
    COLLISIONSUPPORT_CYLINDER   = 0x04, //!< Collision to cylinder is supported.
    COLLISIONSUPPORT_CONE       = 0x08, //!< Collision to cone is supported.
    COLLISIONSUPPORT_BOX        = 0x10, //!< Collision to box is supported.
    COLLISIONSUPPORT_PLANE      = 0x20, //!< Collision to plane is supported.
    COLLISIONSUPPORT_MESH       = 0x40, //!< Collision to mesh is supported.
    
    COLLISIONSUPPORT_ALL        = ~0,   //!< COllision to any rival collision node is supported.
};


/*
 * Structures
 */

struct SCollisionFace
{
    SCollisionFace() :
        Mesh    (0),
        Surface (0),
        Index   (0)
    {
    }
    ~SCollisionFace()
    {
    }
    
    /* === Functions === */
    
    //! Returns true if the specified inverse point is on the back side of this face's triangle.
    inline bool isBackFaceCulling(const video::EFaceTypes CollFace, const dim::vector3df &InversePoint) const
    {
        if (CollFace != video::FACE_BOTH)
        {
            bool isPointFront = dim::plane3df(Triangle).isPointFrontSide(InversePoint);
            
            if ( ( CollFace == video::FACE_FRONT && !isPointFront ) ||
                 ( CollFace == video::FACE_BACK  &&  isPointFront ) )
            {
                return true;
            }
        }
        return false;
    }
    
    //! Returns true if the specified inverse point is on the back side of this face's triangle.
    inline bool isBackFaceCulling(const video::EFaceTypes CollFace, const dim::line3df &InverseLine) const
    {
        if (CollFace != video::FACE_BOTH)
        {
            bool isPointFrontA = dim::plane3df(Triangle).isPointFrontSide(InverseLine.Start);
            bool isPointFrontB = dim::plane3df(Triangle).isPointFrontSide(InverseLine.End);
            
            if ( ( CollFace == video::FACE_FRONT && !isPointFrontA && !isPointFrontB ) ||
                 ( CollFace == video::FACE_BACK  &&  isPointFrontA &&  isPointFrontB ) )
            {
                return true;
            }
        }
        return false;
    }
    
    /* Members */
    scene::Mesh* Mesh;
    u32 Surface;                //!< Surface index.
    u32 Index;                  //!< Triangle index.
    dim::triangle3df Triangle;  //!< Triangle face construction.
};

struct SContactBase
{
    SContactBase() :
        Impact  (0.0f   ),
        Face    (0      )
    {
    }
    virtual ~SContactBase()
    {
    }
    
    /* Members */
    dim::vector3df Point;       //!< Contact point onto the rival collision-node.
    dim::vector3df Normal;      //!< Contact normal. Normal vector from the rival collision-node to the source collision-node.
    f32 Impact;                 //!< Contact impact distance.
    
    dim::triangle3df Triangle;  //!< Triangle face construction. Only used for mesh contacts.
    SCollisionFace* Face;       //!< Contact triangle. Only used for mesh contacts.
};

struct SIntersectionContact : public SContactBase
{
    SIntersectionContact() :
        SContactBase(       ),
        Object      (0      ),
        DistanceSq  (0.0f   )
    {
    }
    ~SIntersectionContact()
    {
    }
    
    /* Operators */
    inline bool operator == (const SIntersectionContact &Other) const
    {
        return Object == Other.Object && Face == Other.Face;
    }
    
    /* Members */
    const CollisionNode* Object;    //!< Constant collision object.
    f32 DistanceSq;                 //!< Squared distance used for internal sorting.
};

struct SCollisionContact : public SContactBase
{
    SCollisionContact() :
        SContactBase()
    {
    }
    ~SCollisionContact()
    {
    }
    
    /* Operators */
    inline bool operator == (const SCollisionContact &Other) const
    {
        return Face == Other.Face;
    }
};

#if 1 // !deprecated!

struct SP_EXPORT SCollisionSystemObject
{
    SCollisionSystemObject();
    virtual ~SCollisionSystemObject();
    
    /* Structures */
    struct STriangleData
    {
        u32 Surface;
        u32 Index;
        dim::ptriangle3df Triangle;
    };
    
    /* Functions */
    void createTriangleData();
    void deleteTriangleData();
    
    /* Members */
    scene::SceneNode* Object;
    scene::Mesh* Mesh;
    
    f32 Radius; // dim::vector3df Radius;
    dim::aabbox3df BoundingBox;
    video::EFaceTypes FaceType;
    
    STriangleData* TriangleList;
    u32 TriangleCount;
    
    bool IgnoreVisibility;
};

struct SCollisionObject : public SCollisionSystemObject
{
    SCollisionObject();
    ~SCollisionObject();
    
    /* Members */
    Collision* CollisionHandle;
    dim::vector3df LastPosition;
    bool isAutoFreeze;
    bool isCollidable;
};

struct SPickingObject : public SCollisionSystemObject
{
    SPickingObject();
    ~SPickingObject();
    
    /* Members */
    EPickingTypes Type;
};

#endif


} // /namespace scene

} // /namespace sp


#endif



// ================================================================================
