/*
 * Collision node header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_COLLISION_NODE_H__
#define __SP_COLLISION_NODE_H__


#include "Base/spStandard.hpp"
#include "Base/spDimension.hpp"
#include "Base/spMathCollisionLibrary.hpp"
#include "Base/spBaseObject.hpp"
#include "SceneGraph/spSceneNode.hpp"
#include "SceneGraph/Collision/spCollisionConfigTypes.hpp"


namespace sp
{
namespace scene
{


/**
CollisionNode is the root class for all collision models which are sphere, box, mesh and terrain.
Its particular an interface for derived collision models.
*/
class SP_EXPORT CollisionNode : public BaseObject
{
    
    public:
        
        virtual ~CollisionNode();
        
        /* === Functions === */
        
        //! Sets the collision material
        virtual void setMaterial(CollisionMaterial* Material);
        
        /**
        Checks for intersections between this collision object and the given line and stored the result in the specified contact list.
        \param Line: Specifies the line which could intersect this object.
        \param ContactList: Specifies the list where all the intersection results will be stored.
        \note The implementations of this interface function don't sort the resulting contact list.
        */
        virtual void findIntersections(const dim::line3df &Line, std::list<SIntersectionContact> &ContactList) const;
        
        /**
        Checks for an intersection between this collision object and the given line and stores the result in the specified contact structure.
        \param Line: Specifies the line which could intersect this object.
        \param Contact: Specifies the structure where the intersection result will be stored.
        \note The implementations of this interface function normally only store the point and normal vector of an intersection.
        */
        virtual bool checkIntersection(const dim::line3df &Line, SIntersectionContact &Contact) const;
        
        //! Returns true if an intersection between this collision object and the given line exists but does not return any further information.
        virtual bool checkIntersection(const dim::line3df &Line) const;
        
        //! Checks for a collision between this collision object and the rival object.
        virtual bool checkCollision(const CollisionNode* Rival, SCollisionContact &Contact) const;
        
        //! Checks if this collision node has any collision with the given rival collision node.
        virtual bool checkCollision(const CollisionNode* Rival) const;
        
        //! Checks for a collision between this collision object and the rival object and performs collision resolving as well.
        virtual void performCollisionResolving(const CollisionNode* Rival);
        
        /* === Inline functions === */
        
        //! Returns the collision model type.
        inline ECollisionModels getType() const
        {
            return Type_;
        }
        
        /**
        Sets the flags for collision detection.
        \see ECollisionFlags.
        */
        inline void setFlags(s32 Flags)
        {
            Flags_ = Flags;
        }
        //! Returns teh flags for collision detection. By default COLLISIONFLAG_BOTH.
        inline s32 getFlags() const
        {
            return Flags_;
        }
        
        //! Returns a pointer to the SceneNode object.
        inline SceneNode* getNode() const
        {
            return Node_;
        }
        
        //! Returns the collision material.
        inline CollisionMaterial* getMaterial() const
        {
            return Material_;
        }
        
        //! Sets the global position of the scene node.
        inline void setPosition(const dim::vector3df &Position)
        {
            Node_->setPosition(Position, true);
        }
        //! Returns the global position of the scene node.
        inline dim::vector3df getPosition() const
        {
            return Node_->getPosition(true);
        }
        
        //! Sets the global rotation of the scene node.
        inline void setRotation(const dim::matrix4f &Rotation)
        {
            Node_->setRotationMatrix(Rotation, true);
        }
        //! Returns the global rotation of the scene node.
        inline dim::matrix4f getRotation() const
        {
            return Node_->getRotationMatrix(true);
        }
        
        //! Sets the global scaling of the scene node.
        inline void setScale(const dim::vector3df &Scale)
        {
            return Node_->setScale(Scale, true);
        }
        //! Returns the global scaling of the scene node.
        inline dim::vector3df getScale() const
        {
            return Node_->getScale(true);
        }
        
        //! Returns the global node's transformation.
        inline dim::matrix4f getTransformation() const
        {
            return Node_->getTransformation(true);
        }
        //! Returns the global node's inverse transformation.
        inline dim::matrix4f getInverseTransformation() const
        {
            return Node_->getTransformation(true).getInverse();
        }
        
    protected:
        
        /* Functions */
        
        CollisionNode(CollisionMaterial* Material, SceneNode* Node, const ECollisionModels Type);
        
        virtual bool checkCollisionToSphere (const CollisionSphere*     Rival, SCollisionContact &Contact) const;
        virtual bool checkCollisionToCapsule(const CollisionCapsule*    Rival, SCollisionContact &Contact) const;
        virtual bool checkCollisionToBox    (const CollisionBox*        Rival, SCollisionContact &Contact) const;
        virtual bool checkCollisionToPlane  (const CollisionPlane*      Rival, SCollisionContact &Contact) const;
        virtual bool checkCollisionToMesh   (const CollisionMesh*       Rival, SCollisionContact &Contact) const;
        
        virtual bool checkAnyCollisionToMesh(const CollisionMesh* Rival) const;
        
        virtual void performCollisionResolvingToSphere  (const CollisionSphere*     Rival);
        virtual void performCollisionResolvingToCapsule (const CollisionCapsule*    Rival);
        virtual void performCollisionResolvingToBox     (const CollisionBox*        Rival);
        virtual void performCollisionResolvingToPlane   (const CollisionPlane*      Rival);
        virtual void performCollisionResolvingToMesh    (const CollisionMesh*       Rival);
        
        void notifyCollisionContact(const CollisionNode* Rival, const SCollisionContact &Contact);
        
    private:
        
        friend class CollisionMaterial;
        
        /* Members */
        
        ECollisionModels Type_; //!< Collision type (or rather model).
        s32 Flags_;             //!< Flags for collision detection. \see ECollisionFlags.
        
        SceneNode* Node_;       //!< Scene node object.
        
        CollisionMaterial* Material_;
        
};


} // /namespace scene

} // /namespace sp


#endif



// ================================================================================
