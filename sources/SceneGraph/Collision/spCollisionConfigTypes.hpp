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
    COLLISION_BOX,      //!< Collision box with position, rotation and axis-alined-bounding-box.
    COLLISION_PLANE,    //!< Collision plane with position and normal vector.
    COLLISION_MESH,     //!< Collision mesh using kd-Tree.
};

//! Flags for collision detection.
enum ECollisionFlags
{
    COLLISIONFLAG_NONE          = 0x00,                                                 //!< Neither collision resolving nore intersection tests are performed.
    COLLISIONFLAG_RESOLVE       = 0x01,                                                 //!< Collision resolving is performed.
    COLLISIONFLAG_INTERSECTION  = 0x02,                                                 //!< Intersection tests are performed.
    COLLISIONFLAG_BOTH          = COLLISIONFLAG_RESOLVE | COLLISIONFLAG_INTERSECTION,   //!< Collision resolving and intersection tests are performed.
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
    
    /* Members */
    scene::Mesh* Mesh;
    u32 Surface;                //!< Surface index.
    u32 Index;                  //!< Triangle index.
    dim::triangle3df Triangle;  //!< Triangle face construction.
};

struct SContactBase
{
    SContactBase() : Face(0)
    {
    }
    virtual ~SContactBase()
    {
    }
    
    /* Members */
    dim::vector3df Point;       //!< Contact point.
    dim::vector3df Normal;      //!< Contact normal.
    
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
        SContactBase(   ),
        Object      (0  )/*,
        Material    (0  )*/
    {
    }
    ~SCollisionContact()
    {
    }
    
    /* Operators */
    inline bool operator == (const SCollisionContact &Other) const
    {
        return Object == Other.Object && Face == Other.Face;
    }
    
    /* Members */
    CollisionNode* Object;  //!< Collision object.
    //CollisionMaterial* Material;
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
