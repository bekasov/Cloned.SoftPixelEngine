/*
 * Newton static object file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "Framework/Physics/Newton/spNewtonStaticObject.hpp"

#ifdef SP_COMPILE_WITH_NEWTON


#include "Framework/Physics/Newton/spNewtonSimulator.hpp"

#include <newton.h>


namespace sp
{
namespace physics
{


NewtonStaticObject::NewtonStaticObject(scene::Mesh* Mesh) :
    BaseObject          (       ),
    PhysicsBaseObject   (       ),
    StaticPhysicsObject (Mesh   ),
    NewtonBaseObject    (       )
{
    createMesh();
}
NewtonStaticObject::~NewtonStaticObject()
{
}


/*
 * ======= Protected: =======
 */

void NewtonStaticObject::createMesh()
{
    /* Create the collision tree */
    NtCollision_ = NewtonCreateTreeCollision(NewtonSimulator::getNewtonWorld(), 0);
    
    /* Temporary storage */
    u32 Indices[3], s, i;
    dim::vector3df VertexCoords[3];
    
    const dim::matrix4f Scaling(Mesh_->getScaleMatrix());
    
    /* Build the collision tree */
    NewtonTreeCollisionBeginBuild(NtCollision_);
    
    for (s = 0; s < Mesh_->getMeshBufferCount(); ++s)
    {
        video::MeshBuffer* Surface = Mesh_->getMeshBuffer(s);
        
        for (i = 0; i < Surface->getTriangleCount(); ++i)
        {
            Surface->getTriangleIndices(i, Indices);
            
            VertexCoords[0] = Scaling * Surface->getVertexCoord(Indices[0]);
            VertexCoords[1] = Scaling * Surface->getVertexCoord(Indices[1]);
            VertexCoords[2] = Scaling * Surface->getVertexCoord(Indices[2]);
            
            NewtonTreeCollisionAddFace(NtCollision_, 3, &VertexCoords[0].X, sizeof(dim::vector3df), 0);
        }
    }
    
    NewtonTreeCollisionEndBuild(NtCollision_, 1);
    
    /* Update first transformation */
    const dim::matrix4f InitTrans(Mesh_->getPositionMatrix() * Mesh_->getRotationMatrix());
    
    /* Create the body */
    NtBody_ = NewtonCreateBody(
        NewtonSimulator::getNewtonWorld(), NtCollision_, InitTrans.getArray()
    );
    
    /* Compute the collision axis-alined-bounding-box (AABB) */
    f32 BoxP0[3], BoxP1[3], Matrix[16];
    
    NewtonBodyGetMatrix(NtBody_, Matrix);
    NewtonCollisionCalculateAABB(NtCollision_, Matrix, BoxP0, BoxP1);
}


} // /namespace physics

} // /namespace sp


#endif



// ================================================================================
