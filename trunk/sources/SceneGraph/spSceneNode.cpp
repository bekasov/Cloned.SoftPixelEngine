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


//! \todo Redesign this part. Don't use global matrices anymore!
dim::matrix4f spProjectionMatrix;
dim::matrix4f spViewMatrix;
dim::matrix4f spViewInvMatrix;
dim::matrix4f spWorldMatrix;
dim::matrix4f spTextureMatrix[MAX_COUNT_OF_TEXTURES];
dim::matrix4f spColorMatrix; // \deprecated


SceneNode::SceneNode(const ENodeTypes Type) :
    Node        (       ),
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
        Transform_.setRotation(SceneParent_->getRotationMatrix(true).getInverse() * Rotation);
    else
        Transform_.setRotation(Rotation);
}
dim::matrix4f SceneNode::getRotationMatrix(bool isGlobal) const
{
    if (isGlobal && SceneParent_)
        return SceneParent_->getRotationMatrix(true) * Transform_.getRotationMatrix();
    return Transform_.getRotationMatrix();
}

void SceneNode::setScaleMatrix(const dim::matrix4f &Scale, bool isGlobal)
{
    if (isGlobal && SceneParent_)
        Transform_.setScale((SceneParent_->getScaleMatrix(true).getInverse() * Scale).getScale());
    else
        Transform_.setScale(Scale.getScale());
}
dim::matrix4f SceneNode::getScaleMatrix(bool isGlobal) const
{
    if (isGlobal && SceneParent_)
        return SceneParent_->getScaleMatrix(true) * dim::getScaleMatrix(Transform_.getScale());
    return dim::getScaleMatrix(Transform_.getScale());
}

void SceneNode::setPosition(const dim::vector3df &Position, bool isGlobal)
{
    if (isGlobal && SceneParent_)
        Transform_.setPosition(SceneParent_->getTransformation().getInverseMatrix() * Position);
    else
        Transform_.setPosition(Position);
}
dim::vector3df SceneNode::getPosition(bool isGlobal) const
{
    return isGlobal ? getTransformation(true).getPosition() : Transform_.getPosition();
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
    return isGlobal ? getTransformation(true).getScale() : Transform_.getScale();
}

void SceneNode::lookAt(const dim::vector3df &Position, bool isGlobal)
{
    /* Temporary variables */
    const dim::vector3df Pos(getPosition(isGlobal));
    dim::vector3df Rot;
    
    /* Calculate rotation */
    if (!math::equal(Position.Y, Pos.Y))
        Rot.X = math::ASin( (Position.Y - Pos.Y) / math::getDistance(Pos, Position) );
    if (!math::equal(Position.X, Pos.X))
        Rot.Y = -math::ASin( (Position.X - Pos.X) / math::getDistance(dim::point2df(Pos.X, Pos.Z), dim::point2df(Position.X, Position.Z)) );
    
    if (Pos.Z < Position.Z)
        Rot.Y = 180.0f - Rot.Y;
    
    setRotation(Rot, isGlobal);
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
    std::vector<Animation*> AnimList = AnimationList_;
    
    foreach (Animation* Anim, AnimList)
        Anim->removeSceneNode(this);
    
    AnimationList_.clear();
}

Animation* SceneNode::getFirstAnimation() const
{
    return AnimationList_.empty() ? 0 : AnimationList_.front();
}

Animation* SceneNode::getAnimation(u32 Index) const
{
    u32 i = 0;
    
    foreach (Animation* Anim, AnimationList_)
    {
        if (Index == i++)
            return Anim;
    }
    
    return 0;
}

Animation* SceneNode::findAnimation(const io::stringc &Name) const
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
        const dim::vector3df    Pos(getPosition         (true));
        const dim::matrix4f     Rot(getRotationMatrix   (true));
        const dim::vector3df    Scl(getScale            (true));
        
        SceneParent_ = Parent;
        
        setPosition         (Pos, true);
        setRotationMatrix   (Rot, true);
        setScale            (Scl, true);
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
    FinalWorldMatrix_ = BaseMatrix * FinalWorldMatrix_;
}

void SceneNode::loadTransformation()
{
    if (__spSceneManager->hasChildTree())
        spWorldMatrix *= FinalWorldMatrix_;
    else
        spWorldMatrix = FinalWorldMatrix_;
}

Transformation SceneNode::getTransformation(bool isGlobal) const
{
    if (isGlobal && SceneParent_)
        return SceneParent_->getTransformation(true) * getTransformation();
    return getTransformation();
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
    for (std::vector<SceneNode*>::const_iterator it = SceneChildren_.begin(); it != SceneChildren_.end(); ++it)
    {
        SceneChildren_.push_back(*it);
        __spSceneManager->removeRootNode(*it);
    }
}

bool SceneNode::removeChild(SceneNode* Child)
{
    for (std::vector<SceneNode*>::iterator it = SceneChildren_.begin(); it != SceneChildren_.end(); ++it)
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
    for (std::vector<SceneNode*>::iterator it = SceneChildren_.begin(); it != SceneChildren_.end(); ++it)
        __spSceneManager->addRootNode(*it);
    SceneChildren_.clear();
}


/*
 * ======= Protected: =======
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
    NewNode->Transform_         = Transform_;
    NewNode->FinalWorldMatrix_  = FinalWorldMatrix_;
    NewNode->Parent_            = Parent_;
    NewNode->SceneParent_       = SceneParent_;
    NewNode->Type_              = Type_;
}


} // /namespace scene

} // /namespace sp



// ================================================================================
