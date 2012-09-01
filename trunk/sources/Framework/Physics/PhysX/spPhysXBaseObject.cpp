/*
 * PhysX base object file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "Framework/Physics/PhysX/spPhysXBaseObject.hpp"

//#ifdef SP_COMPILE_WITH_PHYSX


#include "Base/spInputOutputLog.hpp"

#include <boost/foreach.hpp>


namespace sp
{
namespace physics
{


PhysXBaseObject::PhysXBaseObject(PhysXMaterial* Material) :
    PxBaseActor_(0),
    PxMaterial_ (0)
{
    if (Material)
        PxMaterial_ = Material->PxMaterial_;
}
PhysXBaseObject::~PhysXBaseObject()
{
    PxBaseActor_->release();
}

void PhysXBaseObject::setTransformation(const dim::matrix4f &Transformation)
{
    PxBaseActor_->setGlobalPose(
        PxTransform(PxMat44(const_cast<f32*>(Transformation.getArray())))
    );
}
dim::matrix4f PhysXBaseObject::getTransformation() const
{
    PxTransform Trans(PxBaseActor_->getGlobalPose());
    
    dim::matrix4f Mat(((dim::quaternion*)&Trans.q)->getMatrix());
    Mat.setPosition(*((dim::vector3df*)&Trans.p));
    
    return Mat;
}


/*
 * ======= Protected: =======
 */

void PhysXBaseObject::addShape(const PxGeometry &Geometry)
{
    PxShape* NewShape = PxBaseActor_->createShape(Geometry, *PxMaterial_);
    
    if (NewShape)
        Shapes_.push_back(NewShape);
    else
        io::Log::error("Could not create PhysX shape");
}

void PhysXBaseObject::createBox(const SRigidBodyConstruction &Construct)
{
    addShape(PxBoxGeometry(Construct.Size.X, Construct.Size.Y, Construct.Size.Z));
}
void PhysXBaseObject::createSphere(const SRigidBodyConstruction &Construct)
{
    addShape(PxSphereGeometry(Construct.Radius));
}
void PhysXBaseObject::createCapsule(const SRigidBodyConstruction &Construct)
{
    addShape(PxCapsuleGeometry(Construct.Radius, Construct.Height));
}

void PhysXBaseObject::createMesh(
    PxPhysics* PxDevice, PxCooking* PxCookDevice, scene::Mesh* Mesh)
{
    if (!PxDevice || !PxCookDevice || !Mesh)
        return;
    
    /* Get mesh triangle data */
    const u32 VertexCount   = Mesh->getVertexCount();
    const u32 TriangleCount = Mesh->getTriangleCount();
    
    dim::vector3df* Vertices = new dim::vector3df[VertexCount];
    u32* Triangles = new u32[TriangleCount*3];
    
    u32 VertIndex = 0, TriIndex = 0;
    u32 Indices[3];
    
    for (u32 s = 0; s < Mesh->getMeshBufferCount(); ++s)
    {
        video::MeshBuffer* Surface = Mesh->getMeshBuffer(s);
        
        for (u32 i = 0; i < Surface->getTriangleCount(); ++i)
        {
            Surface->getTriangleIndices(i, Indices);
            
            Triangles[TriIndex++] = VertIndex + Indices[0];
            Triangles[TriIndex++] = VertIndex + Indices[1];
            Triangles[TriIndex++] = VertIndex + Indices[2];
        }
        
        for (u32 i = 0; i < Surface->getVertexCount(); ++i, ++VertIndex)
            Vertices[VertIndex] = Surface->getVertexCoord(i);
    }
    
    /* Setup triangle mesh description */
    PxTriangleMeshDesc MeshDesc;
    
    MeshDesc.points.count           = VertexCount;
    MeshDesc.points.stride          = sizeof(PxVec3);
    MeshDesc.points.data            = (PxVec3*)Vertices;
    
    MeshDesc.triangles.count        = TriangleCount;
    MeshDesc.triangles.stride       = 3*sizeof(PxU32);
    MeshDesc.triangles.data         = (PxU32*)Triangles;
    
    PxDefaultMemoryOutputStream WriteBuffer;
    
    if (PxCookDevice->cookTriangleMesh(MeshDesc, WriteBuffer))
    {
        /* Create triangle mesh geometry */
        PxDefaultMemoryInputData ReadBuffer(WriteBuffer.getData(), WriteBuffer.getSize());
        addShape(PxTriangleMeshGeometry(PxDevice->createTriangleMesh(ReadBuffer)));
    }
    
    delete [] Vertices;
    delete [] Triangles;
}


} // /namespace physics

} // /namespace sp


//#endif



// ================================================================================
