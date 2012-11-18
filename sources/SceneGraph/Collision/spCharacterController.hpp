/*
 * Character controller header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_COLLISION_CHARACTER_CONTROLLER_H__
#define __SP_COLLISION_CHARACTER_CONTROLLER_H__


#include "Base/spStandard.hpp"
#include "SceneGraph/Collision/spCollisionCapsule.hpp"
#include "SceneGraph/Collision/spBaseCollisionPhysicsObject.hpp"


namespace sp
{
namespace scene
{


class CharacterController;


/**
Collision contact callback interface for a character controller.
\param Controller: Pointer to the character controller of the detected contact.
\param Rival: Pointer to the rival collision node which caused the collision contact.
\param Contact: Holds the information details about the collision contact.
\return True if the collision is about to be resolved. Otherwise the collision will be ignored and no collision-resolving will be performed.
*/
typedef boost::function<bool (CharacterController* Controller, const CollisionNode* Rival, const SCollisionContact &Contact)> CharacterContactCallback;


/**
The CharacterController class is to be used - as the name implies - to control a character in your game.
Although the engine's collision sytem foundation does not support any kind of physics simulation,
this character controller holds a gravity vector and a mass value. This is used to
compute a 'physics-simulation like' jump action.
\ingroup group_collision
*/
class SP_EXPORT CharacterController : public BaseCollisionPhysicsObject, public BaseObject
{
    
    public:
        
        CharacterController(CollisionMaterial* Material, SceneNode* Node, f32 Radius, f32 Height);
        virtual ~CharacterController();
        
        /* === Functions === */
        
        /**
        Updates the character controller behaviour. This includes: gravity appliance.
        */
        virtual void update();
        
        /**
        Adds the specified direction as a move force to the character object.
        The maximal move speed will not be overtraversed.
        \param Direction: 
        */
        virtual void move(const dim::point2df &Direction, f32 MaxMoveSpeed);
        
        /**
        Adds the specified force to the character object.
        \param Force: Specifies the force which will be added to the upper vector.
        This is equivalent to the following code:
        \code
        CharCtrl->addForce(CharCtrl->getOrientation() * dim::vector3df(0, Force, 0));
        \endcode
        */
        virtual void jump(f32 Force);
        
        void setRadius(f32 Radius);
        void setHeight(f32 Height);
        
        void setMaxStepHeight(f32 MaxHeight);
        
        /* === Inline functions === */
        
        //! Returns a pointer to the character's collision model (capsule).
        inline CollisionCapsule* getCollisionModel()
        {
            return &CollModel_;
        }
        //! Returns a constant pointer to the character's collision model (capsule).
        inline const CollisionCapsule* getCollisionModel() const
        {
            return &CollModel_;
        }
        
        //! Returns a pointer to the character's step detector.
        inline CollisionCapsule* getStepDetector()
        {
            return &CollStepDetector_;
        }
        /**
        Returns a constant pointer to the character's step detector.
        This can not be modified and is used internally to detect when the
        character needs to take a stair-case step.
        */
        inline const CollisionCapsule* getStepDetector() const
        {
            return &CollStepDetector_;
        }
        
        /**
        Sets the character's view rotation. This is used for further movement.
        \see move
        */
        inline void setViewRotation(f32 Rotation)
        {
            ViewRotation_ = Rotation;
        }
        //! Returns the character's view rotation. By default 0.0.
        inline f32 getViewRotation() const
        {
            return ViewRotation_;
        }
        
        /**
        Returns the maximal step height. By default the initial radius.
        \see setMaxStepHeight
        */
        inline f32 getMaxStepHeight() const
        {
            return MaxStepHeight_;
        }
        
        //! Returns true if the character is on the ground.
        inline bool stayOnGround() const
        {
            return StayOnGround_;
        }
        
        //! Sets the new character's orientation. This specifies in which direction the character will jump and move.
        inline void setOrientation(const dim::matrix3f &Orientation)
        {
            Orientation_ = Orientation;
        }
        //! Returns the orientation. By default the identity 3x3 matrix.
        inline dim::matrix3f getOrientation() const
        {
            return Orientation_;
        }
        
        //! Sets the collision contact callback function. Use this to determine when a collision has been detected.
        inline void setContactCallback(const CharacterContactCallback &Callback)
        {
            CollContactCallback_ = Callback;
        }
        inline CharacterContactCallback getContactCallback() const
        {
            return CollContactCallback_;
        }
        
    protected:
        
        /* === Functions === */
        
        
        /* === Members === */
        
        f32 ViewRotation_;
        f32 MaxStepHeight_;
        
        dim::matrix3f Orientation_;
        
    private:
        
        friend bool ChCtrlCollisionMaterial(
            CollisionMaterial* Material, CollisionNode* Node,
            const CollisionNode* Rival, const SCollisionContact &Contact
        );
        
        /* === Members === */
        
        CollisionCapsule CollModel_;
        CollisionCapsule CollStepDetector_;
        
        CharacterContactCallback CollContactCallback_;
        
        bool StayOnGround_;
        
};


} // /namespace scene

} // /namespace sp


#endif



// ================================================================================
