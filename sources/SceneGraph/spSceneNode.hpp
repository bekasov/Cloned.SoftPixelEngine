/*
 * Scene node header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_SCENE_SCENENODE_H__
#define __SP_SCENE_SCENENODE_H__


#include "Base/spStandard.hpp"
#include "Base/spDimension.hpp"
#include "Base/spInputOutputLog.hpp"
#include "Base/spMaterialStates.hpp"
#include "Base/spGeometryStructures.hpp"
#include "Base/spNode.hpp"
#include "Base/spMath.hpp"
#include "SceneGraph/spBoundingVolume.hpp"
#include "RenderSystem/spShaderProgram.hpp"

#include <list>


namespace sp
{
namespace scene
{


class Animation;

/*
 * Global members
 */

extern dim::matrix4f spProjectionMatrix;
extern dim::matrix4f spViewMatrix;
extern dim::matrix4f spViewInvMatrix;
extern dim::matrix4f spWorldMatrix;
extern dim::matrix4f spTextureMatrix[MAX_COUNT_OF_TEXTURES];
extern dim::matrix4f spColorMatrix;


/*
 * Enumerations
 */

//! Scene node types.
enum ENodeTypes
{
    NODE_BASICNODE,     //!< Basic scene node.
    NODE_CUSTOM,        //!< Custom scene node.
    NODE_SCENEGRAPH,    //!< Scene graph tree node.
    NODE_CAMERA,        //!< View camera.
    NODE_LIGHT,         //!< Light source.
    NODE_MESH,          //!< 3D mesh object.
    NODE_BILLBOARD,     //!< Billboard/ particle/ sprite.
    NODE_TERRAIN,       //!< Terrain object.
};


/**
Nodes are the root of each object. This is the parent class of Entity, Camera, Light, Sprite and Terrain objects.
Here you can find all the functions for locating the object in position, rotation and scaling. Some basic
information about visibility, user data (void*) etc. are implemented in this class, too.
*/
class SP_EXPORT SceneNode : public Node
{
    
    public:
        
        /* === Header === */
        
        SceneNode(const ENodeTypes Type = NODE_BASICNODE);
        virtual ~SceneNode();
        
        /* === Matrix transformations === */
        
        inline void setPositionMatrix(const dim::matrix4f &Position)
        {
            Position_ = Position.getPosition();
        }
        inline dim::matrix4f getPositionMatrix() const
        {
            return dim::getPositionMatrix(Position_);
        }
        
        inline void setRotationMatrix(const dim::matrix4f &Rotation)
        {
            Rotation_ = Rotation;
        }
        inline dim::matrix4f getRotationMatrix() const
        {
            return Rotation_;
        }
        
        inline void setScaleMatrix(const dim::matrix4f &Scale)
        {
            Scale_ = Scale.getScale();
        }
        inline dim::matrix4f getScaleMatrix() const
        {
            return dim::getScaleMatrix(Scale_);
        }
        
        /**
        Sets the object's position matrix.
        \param Position: Matrix which is to be used for the position transformation.
        \param isGlobal: Only useful when the object is a child of an other.
        If so and "isGlobal" is true the transformation will be processed global.
        */
        virtual void setPositionMatrix(const dim::matrix4f &Position, bool isGlobal);
        virtual dim::matrix4f getPositionMatrix(bool isGlobal) const;
        
        virtual void setRotationMatrix(const dim::matrix4f &Rotation, bool isGlobal);
        virtual dim::matrix4f getRotationMatrix(bool isGlobal) const;
        
        virtual void setScaleMatrix(const dim::matrix4f &Scale, bool isGlobal);
        virtual dim::matrix4f getScaleMatrix(bool isGlobal) const;
        
        virtual void setPosition(const dim::vector3df &Position, bool isGlobal = false);
        virtual dim::vector3df getPosition(bool isGlobal = false) const;
        
        /**
        Sets the object's rotation. A typically rotation in the engine is performed by
        the follwoing combination: Y, X, Z. This is the order in which the rotation axles are
        processed. If you want to perform an individual rotation use "setRotationMatrix"
        with your own matrix transformation.
        */
        virtual void setRotation(const dim::vector3df &Rotation, bool isGlobal = false);
        virtual dim::vector3df getRotation(bool isGlobal = false) const;
        
        virtual void setScale(const dim::vector3df &Scale, bool isGlobal = false);
        virtual dim::vector3df getScale(bool isGlobal = false) const;
        
        /* === Summarized matrix transformations === */
        
        virtual void lookAt(const dim::vector3df &Position, bool isGlobal = false);
        
        inline dim::vector3df getDirection(const dim::vector3df &upVector = dim::vector3df(0, 0, 1)) const
        {
            return Rotation_ * upVector;
        }
        
        /* === Movement === */
        
        //! Moves the object in the specified direction. This is dependent on the current rotation transformation.
        virtual void move(const dim::vector3df &Direction);
        
        //! Turns the object with the specified rotation.
        virtual void turn(const dim::vector3df &Rotation);
        
        //! Moves the object in the specified direction. This is independent on the current rotation transformation.
        virtual void translate(const dim::vector3df &Direction);
        
        //! Transforms the object with the specified size.
        virtual void transform(const dim::vector3df &Size);
        
        /* === Collision === */
        
        bool checkContact(Collision* CollisionHandle);
        bool getContact(SCollisionContactData &NextContact, Collision* CollisionHandle);
        bool getNextContact(SCollisionContactData &NextContact);
        
        inline std::list<SCollisionContactData>& getContactList()
        {
            return CollisionContactList_;
        }
        
        /* === Identification === */
        
        //! Returns the scene node type. Use this to cast the object to a mesh (NODE_MESH), camerar (NODE_CAMERA) etc.
        inline ENodeTypes getType() const
        {
            return Type_;
        }
        
        inline void setBoundingVolume(const BoundingVolume &BoundVolume)
        {
            BoundVolume_ = BoundVolume;
        }
        
        inline BoundingVolume& getBoundingVolume()
        {
            return BoundVolume_;
        }
        inline const BoundingVolume& getBoundingVolume() const
        {
            return BoundVolume_;
        }
        
        /* === Animation === */
        
        /**
        Adds the specified animation to this scene node or rather connects the animation
        with this scene node. Normally "Animation::addSceneNode" is used but this function
        has the same effect.
        */
        virtual void addAnimation(Animation* Anim);
        
        //! Removes the specified animation.
        virtual void removeAnimation(Animation* Anim);
        
        //! Clears the animation list.
        virtual void clearAnimations();
        
        //! Returns a pointer to the specified animation object or null if that animation does not exist.
        virtual Animation* getAnimation(u32 Index = 0);
        
        //! Returns a pointer to the animation with the specified name or null if there is no animation with that name.
        virtual Animation* findAnimation(const io::stringc &Name);
        
        //! Returns the whole animation list.
        inline std::list<Animation*> getAnimationList() const
        {
            return AnimationList_;
        }
        
        /* === Parents === */
        
        /**
        \param isGlobal: If true the visibility uses the parent hierarchy.
        \return true if this scene node and all scene nodes in its parent hierarchy are visible. Otherwise false.
        */
        virtual bool getVisible(bool isGlobal = false) const;
        
        /**
        Sets the parent object. With parents object transformations become much more easier. If the parent object
        will be moved, rotated or scaled this child object will be transformed in dependency of its parent.
        e.g. a car has four children even their tires. When the car moves around the scene you don't need to handle
        the tire transformation. But don't forget to disable parents when the parent object has been deleted but the
        children further exist!
        \param ParentNode: Pointer to the new parent Node object. If you want to disable the parent set it to 0.
        \param isGlobal: If true the current object transformation looks the same. The transformation will be performed
        in global space. Otherwise in object space.
        */
        void setParent(SceneNode* Parent, bool isGlobal);
        
        //! Sets the parent object only in object space (this is a little bit faster).
        inline void setParent(SceneNode* Parent)
        {
            SceneParent_ = Parent;
        }
        inline SceneNode* getParent() const
        {
            return SceneParent_;
        }
        
        /* === Children === */
        
        //! Adds the specified child.
        virtual void addChild(SceneNode* Child);
        
        //! Adds the specified children.
        virtual void addChildren(const std::list<SceneNode*> &Children);
        
        //! Removes the specified child and returns true if the child could be removed.
        virtual bool removeChild(SceneNode* Child);
        
        //! Removes the first child and returns true if a child could be removed.
        virtual bool removeChild();
        
        //! Removes the specified children and returns the count of removed children.
        virtual u32 removeChildren(const std::list<SceneNode*> &Children);
        
        //! Removes all children.
        virtual void removeChildren();
        
        //! Returns the children list.
        inline const std::list<SceneNode*> &getSceneChildren() const
        {
            return SceneChildren_;
        }
        inline std::list<SceneNode*> &getSceneChildren()
        {
            return SceneChildren_;
        }
        
        /* === Parent system === */
        
        //! Setups the object transformation.
        inline void setupTransformation(bool isGlobal)
        {
            Transformation_ = getTransformation(isGlobal);
        }
        
        //! Updates the objects transformation.
        virtual void updateTransformation();
        virtual void updateTransformationBase(const dim::matrix4f &BaseMatrix);
        
        dim::matrix4f getTransformation() const;
        dim::matrix4f getTransformation(bool isGlobal) const;
        
        void setTransformation(const dim::matrix4f &Matrix);
        
        //! Loads the transformation into the render system which has been updated previously.
        virtual void loadTransformation();
        
        /* === Extra functions === */
        
        SceneNode* copy() const;
        
    protected:
        
        friend class CollisionDetector;
        friend class Animation;
        
        /* === Functions === */
        
        void copyRoot(SceneNode* NewNode) const;
        
        /* === Members === */
        
        dim::vector3df Position_;       //!< Local position.
        dim::matrix4f Rotation_;        //!< Local quaternion rotation.
        dim::vector3df Scale_;          //!< Local scaling.
        
        dim::matrix4f Transformation_;  //!< Final transformation (Position * Rotation * Scale)
        
        SceneNode* SceneParent_;
        std::list<SceneNode*> SceneChildren_;
        
        std::list<SCollisionContactData> CollisionContactList_; //!< \deprecated
        std::list<Animation*> AnimationList_;
        
        BoundingVolume BoundVolume_;
        
    private:
        
        /* === Members === */
        
        ENodeTypes Type_;
        
};


} // /namespace scene

} // /namespace sp


#endif



// ================================================================================
