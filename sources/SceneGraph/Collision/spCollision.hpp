/*
 * Collision header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_COLLISION_H__
#define __SP_COLLISION_H__


#include "Base/spStandard.hpp"
#include "Base/spDimension.hpp"
#include "Base/spMath.hpp"
#include "Base/spMathCollisionLibrary.hpp"
#include "SceneGraph/spSceneMesh.hpp"
#include "SceneGraph/Collision/spCollisionConfigTypes.hpp"

#include <list>


namespace sp
{
namespace scene
{


/*
 * Collision class
 */

class SP_EXPORT Collision : public BaseObject
{
    
    public:
        
        Collision();
        ~Collision();
        
        /* Functions */
        
        /**
        Adds a new collision relationship. This Collision will be tested with the opposit DestCollision.
        If the DestCollision is also to be tested with this Collision it needs to get its own CollsionMaterial.
        \param DestCollision: Opposit collision which is to be tested for collision detections.
        \param Type: Type of collision. Supported are: sphere-to-sphere (COLLISION_SPHERE_TO_SPHERE),
        sphere-to-box (COLLISION_SPHERE_TO_BOX) and sphere-to-polygon (COLLISION_SPHERE_TO_POLYGON).
        */
        void addCollisionMaterial(Collision* DestCollision, const ECollisionTypes Type);
        void removeCollisionMaterial(Collision* DestCollision);
        
    private:
        
        friend class CollisionDetector;
        
        /* === Structures === */
        
        struct SCollisionMaterial
        {
            Collision* DestCollision;
            ECollisionTypes Type;
        };
        
        /* === Members === */
        
        /*
         * list of each collision material
         * a collision material is a combination of two collisions and its type of collision (e.g. sphere-to-polygon)
         */
        std::list<SCollisionMaterial> CollMaterialList_;
        
        /* list of each object which is using this collision */
        std::list<SCollisionObject*> ObjectList_;
        
};


} // /namespace scene

} // /namespace sp


#endif



// ================================================================================
