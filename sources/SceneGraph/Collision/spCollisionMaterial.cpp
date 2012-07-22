/*
 * Collision material file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "SceneGraph/Collision/spCollisionMaterial.hpp"
#include "Base/spMemoryManagement.hpp"

#include <boost/foreach.hpp>


namespace sp
{
namespace scene
{


CollisionMaterial::CollisionMaterial() :
    BaseObject()
{
}
CollisionMaterial::~CollisionMaterial()
{
    foreach (CollisionNode* Node, CollNodes_)
        Node->Material_ = 0;
}

void CollisionMaterial::addRivalCollisionMaterial(CollisionMaterial* Rival)
{
    if (Rival)
        RivalCollMaterials_.push_back(Rival);
}
void CollisionMaterial::removeRivalCollisionMaterial(CollisionMaterial* Rival)
{
    MemoryManager::removeElement(RivalCollMaterials_, Rival);
}


/*
 * ======= Private: =======
 */

void CollisionMaterial::addCollisionNode(CollisionNode* Node)
{
    if (Node)
        CollNodes_.push_back(Node);
}
void CollisionMaterial::removeCollisionNode(CollisionNode* Node)
{
    MemoryManager::removeElement(CollNodes_, Node);
}


} // /namespace scene

} // /namespace sp



// ================================================================================
