/*
 * Basic node file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "Base/spNode.hpp"


namespace sp
{


Node::Node() :
    BaseObject  (       ),
    isVisible_  (true   ),
    Parent_     (0      )
{
}
Node::~Node()
{
}

void Node::setVisible(bool isVisible)
{
    isVisible_ = isVisible;
}

void Node::addChild(Node* Child)
{
    Children_.push_back(Child);
}

void Node::addChildren(const std::list<Node*> &Children)
{
    for (std::list<Node*>::const_iterator it = Children.begin(); it != Children.end(); ++it)
        Children_.push_back(*it);
}

bool Node::removeChild(Node* Child)
{
    std::list<Node*>::iterator it = std::find(Children_.begin(), Children_.end(), Child);
    
    if (it != Children_.end())
    {
        Children_.erase(it);
        return true;
    }
    
    return false;
}

bool Node::removeChild()
{
    if (!Children_.empty())
    {
        Children_.erase(Children_.begin());
        return true;
    }
    return false;
}

u32 Node::removeChildren(const std::list<Node*> &Children)
{
    u32 RemovedChildren = 0;
    
    for (std::list<Node*>::const_iterator it = Children.begin(); it != Children.end(); ++it)
    {
        if (removeChild(*it))
            ++RemovedChildren;
    }
    
    return RemovedChildren;
}

void Node::removeChildren()
{
    Children_.clear();
}


} // /namespace sp



// ================================================================================
