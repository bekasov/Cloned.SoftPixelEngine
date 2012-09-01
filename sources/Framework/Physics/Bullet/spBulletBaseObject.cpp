/*
 * Bullet base object file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "Framework/Physics/Bullet/spBulletBaseObject.hpp"

#ifdef SP_COMPILE_WITH_BULLET


#include "Base/spMemoryManagement.hpp"

#include <boost/foreach.hpp>


namespace sp
{
namespace physics
{


BulletBaseObject::BulletBaseObject() :
    BtBody_     (0),
    BtShape_    (0),
    BtMeshData_ (0),
    MotionState_(0)
{
}
BulletBaseObject::~BulletBaseObject()
{
    MemoryManager::deleteMemory(MotionState_);
    MemoryManager::deleteMemory(BtMeshData_);
    MemoryManager::deleteMemory(BtShape_);
    MemoryManager::deleteMemory(BtBody_);
}

void BulletBaseObject::setTransformation(const dim::matrix4f &Transformation)
{
    MotionState_->setWorldTransform(BulletMotionState::getTransform(Transformation));
}
dim::matrix4f BulletBaseObject::getTransformation() const
{
    btTransform Trans;
    MotionState_->getWorldTransform(Trans);
    return BulletMotionState::getMatrix(Trans);
}


/*
 * ======= Protected: =======
 */

btCollisionShape* BulletBaseObject::createShape(const ERigidBodies Type, const SRigidBodyConstruction &Construct)
{
    switch (Type)
    {
        case RIGIDBODY_BOX:
            return new btBoxShape(btVector3(Construct.Size.X, Construct.Size.Y, Construct.Size.Z));
        case RIGIDBODY_CONE:
            return new btConeShape(Construct.Radius, Construct.Height);
        case RIGIDBODY_SPHERE:
            return new btSphereShape(Construct.Radius);
        case RIGIDBODY_CYLINDER:
            return new btCylinderShape(btVector3(Construct.Radius, Construct.Height, Construct.Radius));
        case RIGIDBODY_CAPSULE:
            return new btCapsuleShape(Construct.Radius, Construct.Height);
        default:
            break;
    }
    return 0;
}

btCollisionShape* BulletBaseObject::createShape(scene::Mesh* Mesh)
{
    if (!Mesh || !Mesh->getTriangleCount())
        return 0;
    
    BtMeshData_ = new btTriangleMesh();
    
    foreach (video::MeshBuffer* Surface, Mesh->getMeshBufferList())
    {
        for (u32 i = 0; i < Surface->getTriangleCount(); ++i)
        {
            u32 Indices[3];
            Surface->getTriangleIndices(i, Indices);
            
            const dim::vector3df A(Surface->getVertexCoord(Indices[0]));
            const dim::vector3df B(Surface->getVertexCoord(Indices[1]));
            const dim::vector3df C(Surface->getVertexCoord(Indices[2]));
            
            BtMeshData_->addTriangle(
                btVector3(A.X, A.Y, A.Z),
                btVector3(B.X, B.Y, B.Z),
                btVector3(C.X, C.Y, C.Z),
                false
            );
        }
    }
    
    return new btBvhTriangleMeshShape(BtMeshData_, true);
}

void BulletBaseObject::createBody(
    f32 Mass, bool isDynamic, scene::SceneNode* Node, const ERigidBodies Type, const SRigidBodyConstruction &Construct)
{
    /* Create collision shape */
    if (Type == RIGIDBODY_MESH && Node->getType() == scene::NODE_MESH && !isDynamic)
        BtShape_ = createShape(static_cast<scene::Mesh*>(Node));
    else
        BtShape_ = createShape(Type, Construct);
    
    if (!BtShape_)
    {
        io::Log::error("Specified physics object is not supported");
        return;
    }
    
    /* Get local inertia */
    btVector3 LocalInertia(0, 0, 0);
    
    if (isDynamic)
    {
        BtShape_->calculateLocalInertia(Mass, LocalInertia);
        LocalInertia_ = dim::vector3df(LocalInertia.x(), LocalInertia.y(), LocalInertia.z());
    }
    else
        Mass = 0.0f;
    
    /* Create motion state */
    MotionState_ = new BulletMotionState(
        BulletMotionState::getTransform(Node->getTransformMatrix(true)), Node
    );
    
    /* Create rigid body */
    BtBody_ = new btRigidBody(Mass, MotionState_, BtShape_, LocalInertia);
}


} // /namespace physics

} // /namespace sp


#endif



// ================================================================================
