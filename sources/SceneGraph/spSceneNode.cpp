/*
 * Scene node file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "SceneGraph/spSceneNode.hpp"
#include "Platform/spSoftPixelDeviceOS.hpp"

#include <boost/foreach.hpp>


namespace sp
{

extern video::RenderSystem* __spVideoDriver;
extern scene::SceneGraph* __spSceneManager;

namespace scene
{


/*
 * ======= Internal matrices =======
 */

dim::matrix4f spProjectionMatrix;
dim::matrix4f spViewMatrix;
dim::matrix4f spViewInvMatrix;
dim::matrix4f spWorldMatrix;
dim::matrix4f spTextureMatrix[MAX_COUNT_OF_TEXTURES];
dim::matrix4f spColorMatrix;


/*
 * ======= Constructor & destructor =======
 */

SceneNode::SceneNode(const ENodeTypes Type) :
    Node        (       ),
    Scale_      (1      ),
    SceneParent_(0      ),
    Type_       (Type   )
{
}
SceneNode::~SceneNode()
{
    clearAnimations();
}


/* === Detailed localisation === */

void SceneNode::setPositionMatrix(const dim::matrix4f &Position, bool isGlobal)
{
    setPosition(Position.getPosition(), isGlobal);
}
dim::matrix4f SceneNode::getPositionMatrix(bool isGlobal) const
{
    return dim::getPositionMatrix(getPosition(isGlobal));
}

void SceneNode::setRotationMatrix(const dim::matrix4f &Rotation, bool isGlobal)
{
    if (isGlobal && SceneParent_)
        Rotation_ = SceneParent_->getRotationMatrix(true).getInverse() * Rotation;
    else
        Rotation_ = Rotation;
}
dim::matrix4f SceneNode::getRotationMatrix(bool isGlobal) const
{
    if (isGlobal && SceneParent_)
        return SceneParent_->getRotationMatrix(true) * Rotation_;
    return Rotation_;
}

void SceneNode::setScaleMatrix(const dim::matrix4f &Scale, bool isGlobal)
{
    if (isGlobal && SceneParent_)
        Scale_ = (SceneParent_->getScaleMatrix(true).getInverse() * Scale).getScale();
    else
        Scale_ = Scale.getScale();
}
dim::matrix4f SceneNode::getScaleMatrix(bool isGlobal) const
{
    if (isGlobal && SceneParent_)
        return SceneParent_->getScaleMatrix(true) * dim::getScaleMatrix(Scale_);
    return dim::getScaleMatrix(Scale_);
}

void SceneNode::setPosition(const dim::vector3df &Position, bool isGlobal)
{
    if (isGlobal && SceneParent_)
        Position_ = SceneParent_->getTransformation().getInverse() * Position;
    else
        Position_ = Position;
}
dim::vector3df SceneNode::getPosition(bool isGlobal) const
{
    return isGlobal ? getTransformation(true).getPosition() : Position_;
}

void SceneNode::setRotation(const dim::vector3df &Rotation, bool isGlobal)
{
    setRotationMatrix(dim::getRotationMatrix(Rotation), isGlobal);
}
dim::vector3df SceneNode::getRotation(bool isGlobal) const
{
    return getRotationMatrix(isGlobal).getRotation();
}

void SceneNode::setScale(const dim::vector3df &Scale, bool isGlobal)
{
    setScaleMatrix(dim::getScaleMatrix(Scale), isGlobal);
}
dim::vector3df SceneNode::getScale(bool isGlobal) const
{
    return isGlobal ? getTransformation(true).getScale() : Scale_;
}

void SceneNode::move(const dim::vector3df &Direction)
{
    Position_ += (Rotation_ * Direction);
}
void SceneNode::turn(const dim::vector3df &Rotation)
{
    dim::matrix4f Mat;
    Mat.setRotation(Rotation);
    Rotation_ *= Mat;
}
void SceneNode::translate(const dim::vector3df &Direction)
{
    Position_ += Direction;
}
void SceneNode::transform(const dim::vector3df &Size)
{
    Scale_ += Size;
}

void SceneNode::lookAt(const dim::vector3df &Position, bool isGlobal)
{
    /* Temporary variables */
    dim::vector3df Pos(getPosition(isGlobal));
    dim::vector3df Rot;
    
    /* Calculate rotation */
    if (!math::Equal(Position.Y, Pos.Y))
        Rot.X = math::ASin( (Position.Y - Pos.Y) / math::getDistance(Pos, Position) );
    if (!math::Equal(Position.X, Pos.X))
        Rot.Y = -math::ASin( (Position.X - Pos.X) / math::getDistance(dim::point2df(Pos.X, Pos.Z), dim::point2df(Position.X, Position.Z)) );
    
    if (Pos.Z < Position.Z)
        Rot.Y = 180.0f - Rot.Y;
    
    setRotation(Rot, isGlobal);
}


/* === Collision === */

bool SceneNode::checkContact(Collision* CollisionHandle)
{
    for (std::list<SCollisionContactData>::iterator it = CollisionContactList_.begin(); it != CollisionContactList_.end(); ++it)
    {
        if (it->CollisionHandle == CollisionHandle)
            return true;
    }
    return false;
}

bool SceneNode::getContact(SCollisionContactData &NextContact, Collision* CollisionHandle)
{
    for (std::list<SCollisionContactData>::iterator it = CollisionContactList_.begin(); it != CollisionContactList_.end(); ++it)
    {
        if (it->CollisionHandle == CollisionHandle)
        {
            NextContact = *it;
            CollisionContactList_.erase(it);
            return true;
        }
    }
    return false;
}

bool SceneNode::getNextContact(SCollisionContactData &NextContact)
{
    if (CollisionContactList_.size())
    {
        NextContact = *CollisionContactList_.rbegin();
        CollisionContactList_.pop_back();
        return true;
    }
    return false;
}


/* === Animation === */

void SceneNode::addAnimation(Animation* Anim)
{
    if (Anim)
        Anim->addSceneNode(this);
}
void SceneNode::removeAnimation(Animation* Anim)
{
    if (Anim)
        Anim->removeSceneNode(this);
}
void SceneNode::clearAnimations()
{
    std::list<Animation*> AnimList = AnimationList_;
    
    foreach (Animation* Anim, AnimList)
        Anim->removeSceneNode(this);
    
    AnimationList_.clear();
}

Animation* SceneNode::getAnimation(u32 Index)
{
    u32 i = 0;
    
    foreach (Animation* Anim, AnimationList_)
    {
        if (Index == i++)
            return Anim;
    }
    
    return 0;
}

Animation* SceneNode::findAnimation(const io::stringc &Name)
{
    foreach (Animation* Anim, AnimationList_)
    {
        if (Anim->getName() == Name)
            return Anim;
    }
    return 0;
}


/* === Parent system === */

bool SceneNode::getVisible(bool isGlobal) const
{
    if (!isVisible_)
        return false;
    if (isGlobal && SceneParent_)
        return SceneParent_->getVisible(true);
    return true;
}

void SceneNode::setParent(SceneNode* Parent, bool isGlobal)
{
    if (isGlobal)
    {
        const dim::matrix4f MatPos = getPositionMatrix(true);
        const dim::matrix4f MatRot = getRotationMatrix(true);
        const dim::matrix4f MatScl = getScaleMatrix(true);
        
        SceneParent_ = Parent;
        
        setPositionMatrix(MatPos, true);
        setRotationMatrix(MatRot, true);
        setScaleMatrix(MatScl, true);
    }
    else
        SceneParent_ = Parent;
}

void SceneNode::updateTransformation()
{
    setupTransformation(!__spSceneManager->hasChildTree());
}
void SceneNode::updateTransformationBase(const dim::matrix4f &BaseMatrix)
{
    updateTransformation();
    Transformation_ = BaseMatrix * Transformation_;
}

void SceneNode::loadTransformation()
{
    if (__spSceneManager->hasChildTree())
        spWorldMatrix *= Transformation_;
    else
        spWorldMatrix = Transformation_;
}

dim::matrix4f SceneNode::getTransformation() const
{
    dim::matrix4f Transformation(Rotation_);
    Transformation.setPosition(Position_);
    Transformation.setScale(Scale_);
    return Transformation;
}

dim::matrix4f SceneNode::getTransformation(bool isGlobal) const
{
    if (isGlobal && SceneParent_)
        return SceneParent_->getTransformation(true) * getTransformation();
    return getTransformation();
}

void SceneNode::setTransformation(const dim::matrix4f &Matrix)
{
    /* Setup position and rotation */
    Position_   = Matrix.getPosition();
    Rotation_   = Matrix.getRotationMatrix();
    Scale_      = Matrix.getScale();
}

SceneNode* SceneNode::copy() const
{
    SceneNode* NewNode = new SceneNode(NODE_BASICNODE);
    copyRoot(NewNode);
    return NewNode;
}


/* === Children === */

void SceneNode::addChild(SceneNode* Child)
{
    SceneChildren_.push_back(Child);
    __spSceneManager->removeRootNode(Child);
}

void SceneNode::addChildren(const std::list<SceneNode*> &Children)
{
    for (std::list<SceneNode*>::const_iterator it = SceneChildren_.begin(); it != SceneChildren_.end(); ++it)
    {
        SceneChildren_.push_back(*it);
        __spSceneManager->removeRootNode(*it);
    }
}

bool SceneNode::removeChild(SceneNode* Child)
{
    for (std::list<SceneNode*>::iterator it = SceneChildren_.begin(); it != SceneChildren_.end(); ++it)
    {
        if (*it == Child)
        {
            __spSceneManager->addRootNode(Child);
            SceneChildren_.erase(it);
            return true;
        }
    }
    return false;
}

bool SceneNode::removeChild()
{
    if (!SceneChildren_.empty())
    {
        __spSceneManager->addRootNode(*SceneChildren_.begin());
        SceneChildren_.erase(SceneChildren_.begin());
        return true;
    }
    return false;
}

u32 SceneNode::removeChildren(const std::list<SceneNode*> &Children)
{
    u32 CountRemoved = 0;
    
    for (std::list<SceneNode*>::const_iterator it = Children.begin(); it != Children.end(); ++it)
    {
        if (removeChild(*it))
            ++CountRemoved;
    }
    
    return CountRemoved;
}

void SceneNode::removeChildren()
{
    for (std::list<SceneNode*>::iterator it = SceneChildren_.begin(); it != SceneChildren_.end(); ++it)
        __spSceneManager->addRootNode(*it);
    SceneChildren_.clear();
}


/*
 * ======= Protected functions =======
 */

void SceneNode::copyRoot(SceneNode* NewNode) const
{
    if (!NewNode)
        return;
    
    /* Copy base object  */
    NewNode->setUserData        (getUserData        ());
    NewNode->setName            (getName            ());
    
    /* Copy node object */
    NewNode->setVisible         (getVisible         ());
    
    /* Copy bounding volume */
    NewNode->setBoundingVolume  (getBoundingVolume  ());
    
    /* Copy scene node */
    NewNode->Position_          = Position_;
    NewNode->Rotation_          = Rotation_;
    NewNode->Scale_             = Scale_;
    NewNode->Transformation_    = Transformation_;
    NewNode->Parent_            = Parent_;
    NewNode->SceneParent_       = SceneParent_;
    NewNode->Type_              = Type_;
}


} // /namespace scene

} // /namespace sp



// ================================================================================
