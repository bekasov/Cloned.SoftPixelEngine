/*
 * Collision file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "SceneGraph/Collision/spCollision.hpp"


namespace sp
{
namespace scene
{


/*
 * SCollisionSystemObject structure
 */

SCollisionSystemObject::SCollisionSystemObject() :
    Object          (0                  ),
    Mesh            (0                  ),
    Radius          (0.5f               ),
    FaceType        (video::FACE_FRONT  ),
    TriangleList    (0                  ),
    TriangleCount   (0                  ),
    IgnoreVisibility(false              )
{
}
SCollisionSystemObject::~SCollisionSystemObject()
{
}

void SCollisionSystemObject::createTriangleData()
{
    deleteTriangleData();
    
    if (!Mesh)
        return;
    
    /* Check for referenced mesh */
    if (Mesh->isInstanced())
    {
        scene::Mesh* ReferenceMesh = Mesh->getReference();
        
        if (ReferenceMesh->PickRef_)
        {
            TriangleCount   = ReferenceMesh->PickRef_->TriangleCount;
            TriangleList    = ReferenceMesh->PickRef_->TriangleList;
            return;
        }
    }
    
    /* Create triangle data */
    u32 Indices[3];
    
    TriangleCount   = Mesh->getTriangleCount();
    TriangleList    = new STriangleData[TriangleCount];
    
    u32 s, i, j = 0;
    
    for (s = 0; s < Mesh->getMeshBufferCount(); ++s)
    {
        video::MeshBuffer* Surface = Mesh->getMeshBuffer(s);
        
        for (i = 0; i < Surface->getTriangleCount(); ++i, ++j)
        {
            TriangleList[j].Triangle    = Surface->getTriangleReference(i);
            TriangleList[j].Surface     = s;
            TriangleList[j].Index       = i;
        }
    }
}

void SCollisionSystemObject::deleteTriangleData()
{
    if (!Mesh || !Mesh->isInstanced())
    {
        MemoryManager::deleteBuffer(TriangleList);
        TriangleCount = 0;
    }
}

SCollisionObject::SCollisionObject() :
    SCollisionSystemObject  (       ),
    CollisionHandle         (0      ),
    isAutoFreeze            (false  ),
    isCollidable            (true   )
{
}
SCollisionObject::~SCollisionObject()
{
    deleteTriangleData();
    if (Mesh)
        Mesh->CollRef_ = 0;
}

SPickingObject::SPickingObject() :
    SCollisionSystemObject  (               ),
    Type                    (PICKMODE_NONE  )
{
}
SPickingObject::~SPickingObject()
{
    deleteTriangleData();
    if (Mesh)
        Mesh->PickRef_ = 0;
}


/*
 * Collision class
 */

Collision::Collision()
{
}
Collision::~Collision()
{
}

void Collision::addCollisionMaterial(Collision* DestCollision, const ECollisionTypes Type)
{
    SCollisionMaterial DestData;
    {
        DestData.DestCollision  = DestCollision;
        DestData.Type           = Type;
    }
    CollMaterialList_.push_back(DestData);
}

void Collision::removeCollisionMaterial(Collision* DestCollision)
{
    for (std::list<SCollisionMaterial>::iterator it = CollMaterialList_.begin(); it != CollMaterialList_.end(); ++it)
    {
        if (it->DestCollision == DestCollision)
        {
            CollMaterialList_.erase(it);
            return;
        }
    }
}


} // /namespace scene

} // /namespace sp



// ================================================================================
