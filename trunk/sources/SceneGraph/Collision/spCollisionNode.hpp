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
\ingroup group_collision
*/
class SP_EXPORT CollisionNode : public BaseObject
{
    
    public:
        
        virtual ~CollisionNode();
        
        /* === Functions === */
        
        /*
        Retunrs the flags for collision support to rival collision nodes.
        \see ECollisionSupportFlags
        */
        virtual s32 getSupportFlags() const = 0;
        
        /**
        Returns the maximal movement (e.g. for a sphere it's the radius).
        This is used internally. A collision node can still be moved faster than this value.
        */
        virtual f32 getMaxMovement() const = 0;
        
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
        virtual bool checkIntersection(const dim::line3df &Line, bool ExcludeCorners = false) const;
        
        //! Checks for a collision between this collision object and the rival object.
        virtual bool checkCollision(const CollisionNode* Rival, SCollisionContact &Contact) const;
        
        //! Checks if this collision node has any collision with the given rival collision node.
        virtual bool checkCollision(const CollisionNode* Rival) const;
        
        //! Checks for a collision between this collision object and the rival object and performs collision resolving as well.
        virtual void performCollisionResolving(const CollisionNode* Rival);
        
        /**
        Updates the collision-node's transformation. Call this after you have changed the scene-node's transformation.
        Or only use the transformation functions of the collision-node itself.
        \code
        // Use this:
        MySceneNode->translate(dim::vector3df(0, 0, 1));
        MyCollisionNode->updateTransformation();
        
        // Or this:
        MyCollisionNode->translate(dim::vector3df(0, 0, 1));
        \endcode
        */
        void updateTransformation();
        
        //! Sets the global position of the scene node.
        void setPosition(const dim::vector3df &Position, bool UpdatePrevPosition = true);
        
        //! Sets the transformation offset. This matrix will be multiplied to the scene-node's global transformation.
        void setOffset(const dim::matrix4f &Matrix, bool Enable = true);
        
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
        //! Returns teh flags for collision detection. By default COLLISIONFLAG_FULL.
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
        
        //! Returns the global position of the scene node.
        inline dim::vector3df getPosition() const
        {
            return Trans_.getPosition();
        }
        
        //! Wrapper function for SceneNode::translate.
        inline void translate(const dim::vector3df &Direction)
        {
            Node_->translate(Direction);
            updateTransformation();
        }
        //! Wrapper function for SceneNode::move.
        inline void move(const dim::vector3df &Direction)
        {
            Node_->move(Direction);
            updateTransformation();
        }
        //! Wrapper function for SceneNode::turn.
        inline void turn(const dim::vector3df &Rotation)
        {
            Node_->turn(Rotation);
            updateTransformation();
        }
        
        //! Sets the global rotation of the scene node.
        inline void setRotation(const dim::matrix4f &Rotation)
        {
            Node_->setRotationMatrix(Rotation, true);
            updateTransformation();
        }
        //! Returns the global rotation of the scene node.
        inline dim::matrix4f getRotation() const
        {
            return Trans_.getRotationMatrix();
        }
        
        //! Sets the global scaling of the scene node.
        inline void setScale(const dim::vector3df &Scale)
        {
            return Node_->setScale(Scale, true);
            updateTransformation();
        }
        //! Returns the global scaling of the scene node.
        inline dim::vector3df getScale() const
        {
            return Trans_.getScale();
        }
        
        //! Returns the global node's transformation.
        inline dim::matrix4f getTransformation() const
        {
            return Trans_;
        }
        //! Returns the global node's inverse transformation.
        inline dim::matrix4f getInverseTransformation() const
        {
            return InvTrans_;
        }
        
        //! Returns the transformation offset.
        inline void getOffset(dim::matrix4f &Matrix, bool &Enable) const
        {
            Matrix = OffsetTrans_;
            Enable = UseOffsetTrans_;
        }
        
        //! Returns the previous position stored in the previous frame.
        inline dim::vector3df getPrevPosition() const
        {
            return PrevPosition_;
        }
        
    protected:
        
        friend class CollisionGraph;
        
        /* === Functions === */
        
        CollisionNode(CollisionMaterial* Material, SceneNode* Node, const ECollisionModels Type);
        
        virtual bool checkCollisionToSphere     (const CollisionSphere*     Rival, SCollisionContact &Contact) const;
        virtual bool checkCollisionToCapsule    (const CollisionCapsule*    Rival, SCollisionContact &Contact) const;
        virtual bool checkCollisionToCylinder   (const CollisionCylinder*   Rival, SCollisionContact &Contact) const;
        virtual bool checkCollisionToCone       (const CollisionCone*       Rival, SCollisionContact &Contact) const;
        virtual bool checkCollisionToBox        (const CollisionBox*        Rival, SCollisionContact &Contact) const;
        virtual bool checkCollisionToPlane      (const CollisionPlane*      Rival, SCollisionContact &Contact) const;
        virtual bool checkCollisionToMesh       (const CollisionMesh*       Rival, SCollisionContact &Contact) const;
        
        virtual bool checkAnyCollisionToSphere  (const CollisionSphere*     Rival) const;
        virtual bool checkAnyCollisionToCapsule (const CollisionCapsule*    Rival) const;
        virtual bool checkAnyCollisionToCylinder(const CollisionCylinder*   Rival) const;
        virtual bool checkAnyCollisionToCone    (const CollisionCone*       Rival) const;
        virtual bool checkAnyCollisionToBox     (const CollisionBox*        Rival) const;
        virtual bool checkAnyCollisionToPlane   (const CollisionPlane*      Rival) const;
        virtual bool checkAnyCollisionToMesh    (const CollisionMesh*       Rival) const;
        
        virtual void performCollisionResolvingToSphere  (const CollisionSphere*     Rival);
        virtual void performCollisionResolvingToCapsule (const CollisionCapsule*    Rival);
        virtual void performCollisionResolvingToCylinder(const CollisionCylinder*   Rival);
        virtual void performCollisionResolvingToCone    (const CollisionCone*       Rival);
        virtual void performCollisionResolvingToBox     (const CollisionBox*        Rival);
        virtual void performCollisionResolvingToPlane   (const CollisionPlane*      Rival);
        virtual void performCollisionResolvingToMesh    (const CollisionMesh*       Rival);
        
        void notifyCollisionContact(const CollisionNode* Rival, const SCollisionContact &Contact);
        void performDetectedContact(const CollisionNode* Rival, const SCollisionContact &Contact);
        
        bool checkCornerExlusion(const dim::line3df &Line, const dim::vector3df &Point) const;
        
        void updatePrevPosition();
        
    private:
        
        friend class CollisionMaterial;
        
        /* === Members === */
        
        ECollisionModels Type_;         //!< Collision type (or rather model).
        s32 Flags_;                     //!< Flags for collision detection. \see ECollisionFlags.
        
        SceneNode* Node_;               //!< Scene node object.
        
        CollisionMaterial* Material_;   //!< Collision material object.
        
        dim::matrix4f Trans_;           //!< Global scene-node transformation.
        dim::matrix4f InvTrans_;        //!< Global scene-node inverse transformation.
        dim::matrix4f OffsetTrans_;     //!< Offset transformation.
        
        dim::vector3df PrevPosition_;   //!< Previous position stored in the previous frame.
        
        bool UseOffsetTrans_;           //!< Specifies whether offset transformation is enabled or not.
        
};


} // /namespace scene

} // /namespace sp


#endif



// ================================================================================
