/*
 * Path finder file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "Framework/Tools/spToolPathFinder.hpp"

#ifdef SP_COMPILE_WITH_PATHFINDER


#include "Base/spMemoryManagement.hpp"

#include <boost/foreach.hpp>


namespace sp
{
namespace tool
{


/*
 * PathNode class
 */

PathNode::PathNode() :
    BaseObject      (   ),
    WayCosts_       (0  ),
    DirectDistance_ (0  ),
    Predecessor_    (0  )
{
}
PathNode::PathNode(const dim::vector3df &Position, void* Data) :
    BaseObject      (           ),
    Position_       (Position   ),
    WayCosts_       (0          ),
    DirectDistance_ (0          ),
    Predecessor_    (0          )
{
    setUserData(Data);
}
PathNode::~PathNode()
{
}

void PathNode::setPosition(const dim::vector3df &Position)
{
    Position_ = Position;
    
    foreach (PathEdge* Edge, Edges_)
        Edge->updateNodePosition(this);
}

std::list<PathNode*> PathNode::getNeighbors() const
{
    std::list<PathNode*> Neighbors;
    
    for (std::list<SNeighbor>::const_iterator it = Neighbors_.begin(); it != Neighbors_.end(); ++it)
        Neighbors.push_back(it->Node);
    
    return Neighbors;
}

void PathNode::addEdge(PathEdge* Edge)
{
    Edges_.push_back(Edge);
    updateNeighbors();
}
void PathNode::removeEdge(PathEdge* Edge)
{
    MemoryManager::removeElement(Edges_, Edge);
    updateNeighbors();
}

void PathNode::updateNeighbors()
{
    Neighbors_.clear();
    
    for (std::list<PathEdge*>::iterator it = Edges_.begin(); it != Edges_.end(); ++it)
    {
        if ((*it)->getAdjusted())
        {
            if ((*it)->getFrom() == this)
                Neighbors_.push_back(SNeighbor(this, (*it)->getTo()));
            else
                Neighbors_.push_back(SNeighbor(this, (*it)->getFrom()));
        }
        else
            Neighbors_.push_back(SNeighbor(this, (*it)->getTo()));
    }
}


/*
 * PathEdge class
 */

PathEdge::PathEdge() :
    BaseObject  (       ),
    From_       (0      ),
    To_         (0      ),
    Distance_   (0      ),
    Adjusted_   (false  )
{
}
PathEdge::PathEdge(PathNode* From, PathNode* To, bool Adjusted) :
    BaseObject  (           ),
    From_       (From       ),
    To_         (To         ),
    Adjusted_   (Adjusted   )
{
    if (From_ && To_)
    {
        Distance_ = math::getDistance(From_->getPosition(), To_->getPosition());
        From_->addEdge(this);
        if (!Adjusted_)
            To_->addEdge(this);
    }
    else
    {
        From_ = To_ = 0;
        Adjusted = false;
    }
}
PathEdge::~PathEdge()
{
}

void PathEdge::updateNodePosition(PathNode* Node)
{
    if (From_ == Node)
        Distance_ = math::getDistance(Node->getPosition(), To_->getPosition());
    else
        Distance_ = math::getDistance(From_->getPosition(), Node->getPosition());
}


/*
 * PathFinder class
 */

PathGraph::PathGraph() : StartNode_(0), TargetNode_(0)
{
    isSolved_ = false;
}
PathGraph::~PathGraph()
{
    MemoryManager::deleteList(NodeList_);
    MemoryManager::deleteList(EdgeList_);
}

PathNode* PathGraph::addNode(const dim::vector3df &Position, void* Data)
{
    PathNode* NewNode = new PathNode(Position, Data);
    NodeList_.push_back(NewNode);
    return NewNode;
}
void PathGraph::removeNode(PathNode* Node)
{
    /* Remove each edge which is incident to this node */
    for (std::list<PathEdge*>::iterator it = EdgeList_.begin(); it != EdgeList_.end();)
    {
        if ((*it)->getFrom() == Node || (*it)->getTo() == Node)
        {
            (*it)->getFrom()->removeEdge(*it);
            (*it)->getTo()->removeEdge(*it);
            
            MemoryManager::deleteMemory(*it);
            it = EdgeList_.erase(it);
        }
        else
            ++it;
    }
    
    /* Remove the node */
    MemoryManager::removeElement(NodeList_, Node, true);
}

void PathGraph::clearNodeList()
{
    MemoryManager::deleteList(NodeList_);
    MemoryManager::deleteList(EdgeList_);
}

PathEdge* PathGraph::addEdge(PathNode* From, PathNode* To, bool Adjusted)
{
    if (From && To && From != To)
    {
        PathEdge* NewEdge = new PathEdge(From, To, Adjusted);
        EdgeList_.push_back(NewEdge);
        return NewEdge;
    }
    return 0;
}
void PathGraph::removeEdge(PathEdge* Edge)
{
    MemoryManager::removeElement(EdgeList_, Edge);
}

void PathGraph::clearEdgeList()
{
    MemoryManager::deleteList(EdgeList_);
}

void PathGraph::createGrid(
    const dim::vector3df &From, const dim::vector3df &To, const dim::vector3di &Steps,
    const std::vector<bool> &Bitmap, bool DiagonalEdges)
{
    if (Steps.X < 1 || Steps.Y < 1 || Steps.Z < 1)
        return;
    
    s32 ArrayIndex;
    dim::vector3di i, j, k;
    dim::vector3df p;
    PathNode**** NodeVec = 0;
    
    const dim::vector3di Stretch(
        math::Max(1, Steps.X - 1), math::Max(1, Steps.Y - 1), math::Max(1, Steps.Z - 1)
    );
    
    /* Create nodes */
    NodeVec = new PathNode***[Steps.Z];
    for (i.Z = 0; i.Z < Steps.Z; ++i.Z)
    {
        NodeVec[i.Z] = new PathNode**[Steps.Y];
        for (i.Y = 0; i.Y < Steps.Y; ++i.Y)
        {
            NodeVec[i.Z][i.Y] = new PathNode*[Steps.X];
            for (i.X = 0; i.X < Steps.X; ++i.X)
            {
                ArrayIndex = i.Z*Steps.Y*Steps.X + i.Y*Steps.X + i.X;
                
                if (static_cast<u32>(ArrayIndex) >= Bitmap.size() || Bitmap[ArrayIndex])
                {
                    p = From + (To - From) * i.cast<f32>() / Stretch.cast<f32>();
                    NodeVec[i.Z][i.Y][i.X] = addNode(p);
                }
                else
                    NodeVec[i.Z][i.Y][i.X] = 0;
            }
        }
    }
    
    /* Create edges */
    for (i.Z = 0; i.Z < Steps.Z; ++i.Z)
    {
        for (i.Y = 0; i.Y < Steps.Y; ++i.Y)
        {
            for (i.X = 0; i.X < Steps.X; ++i.X)
            {
                /* Check all neighbors */
                for (j.Z = -1; j.Z <= 1; ++j.Z)
                for (j.Y = -1; j.Y <= 1; ++j.Y)
                for (j.X = -1; j.X <= 1; ++j.X)
                {
                    k = i + j;
                    if ( j != dim::vector3di(0) &&
                         k.X >= 0 && k.X < Steps.X &&
                         k.Y >= 0 && k.Y < Steps.Y &&
                         k.Z >= 0 && k.Z < Steps.Z )
                    {
                        if ( DiagonalEdges || ( j.X == 0 && j.Y == 0 ) || ( j.X == 0 && j.Z == 0 ) || ( j.Y == 0 && j.Z == 0 ) )
                            addEdge(NodeVec[i.Z][i.Y][i.X], NodeVec[k.Z][k.Y][k.X], true);
                    }
                }
            }
            //delete [] NodeVec[i.Z][i.Y];
        }
        //delete [] NodeVec[i.Z];
    }
    for (i.Z = 0; i.Z < Steps.Z; ++i.Z)
    {
        for (i.Y = 0; i.Y < Steps.Y; ++i.Y)
            delete [] NodeVec[i.Z][i.Y];
        delete [] NodeVec[i.Z];
    }
    delete [] NodeVec;
}

std::list<PathNode*> PathGraph::findPath(PathNode* From, PathNode* To)
{
    /* Check if start equals end node */
    if (From == To)
        return std::list<PathNode*>(1, From);
    
    /* Initialization */
    StartNode_  = From;
    TargetNode_ = To;
    
    isSolved_ = false;
    
    /* Add the first node to the queue list */
    addNodeToQueue(StartNode_, 0, 0);
    
    /* Process next step while searching the optimal path in the graph */
    while (nextStep());
    
    /* Reset the storage */
    NodeQueue_.clear();
    ClosedMap_.clear();
    
    std::list<PathNode*> Path;
    
    if (!isSolved_)
        return Path;
    
    /* Build the path */
    constructPath(Path, TargetNode_);
    
    return Path;
}

std::list<PathNode*> PathGraph::findPath(const dim::vector3df &From, const dim::vector3df &To)
{
    PathNode* FromNode = 0, * ToNode = 0;
    
    f32 DistanceSq;
    f32 DistFrom = 999999.f, DistTo = 999999.f;
    
    for (std::list<PathNode*>::iterator it = NodeList_.begin(); it != NodeList_.end(); ++it)
    {
        /* Check distance for start node */
        DistanceSq = math::getDistanceSq(From, (*it)->getPosition());
        
        if (DistanceSq < DistFrom)
        {
            DistFrom = DistanceSq;
            FromNode = *it;
        }
        
        /* Check distance for end node */
        DistanceSq = math::getDistanceSq(To, (*it)->getPosition());
        
        if (DistanceSq < DistTo)
        {
            DistTo = DistanceSq;
            ToNode = *it;
        }
    }
    
    return findPath(FromNode, ToNode);
}


/*
 * ======= Protected: =======
 */

bool PathGraph::nextStep()
{
    /* Get the next accessable node */
    PathNode* CurNode = getNextNode();
    
    /* Check if no path has been found */
    if (!CurNode)
        return false;
    
    /* Check if path has been found */
    if (CurNode == TargetNode_)
    {
        isSolved_ = true;
        return false;
    }
    
    /* Add the neighbors to the queue */
    for (std::list<PathNode::SNeighbor>::iterator it = CurNode->Neighbors_.begin(); it != CurNode->Neighbors_.end(); ++it)
        addNodeToQueue(it->Node, CurNode, CurNode->WayCosts_ + it->Distance);
    
    return true;
}

PathNode* PathGraph::getNextNode()
{
    if (NodeQueue_.empty())
        return 0;
    
    /* Search the optimal next node */
    f32 Distance = 999999.f, CurDist;
    std::list<PathNode*>::iterator itLast;
    
    for (std::list<PathNode*>::iterator it = NodeQueue_.begin(); it != NodeQueue_.end(); ++it)
    {
        CurDist = (*it)->getMinWayCosts();
        
        if (CurDist < Distance)
        {
            Distance    = CurDist;
            itLast      = it;
        }
    }
    
    /* Return the next node and remove it from the queue */
    PathNode* NextNode = *itLast;
    NodeQueue_.erase(itLast);
    
    return NextNode;
}

void PathGraph::addNodeToQueue(PathNode* Node, PathNode* Predecessor, const f32 WayCosts)
{
    /* Check if the node is already in the closed list */
    std::map<PathNode*, bool>::iterator itMap = ClosedMap_.find(Node);
    
    if (itMap == ClosedMap_.end())
    {
        /* Store the direct distance between the current node and the target node */
        Node->DirectDistance_ = math::getDistance(Node->getPosition(), TargetNode_->getPosition());
        
        /* Add the node to the queue and to the closed list */
        NodeQueue_.push_back(Node);
        ClosedMap_[Node] = true;
    }
    if (itMap == ClosedMap_.end() || WayCosts < Node->WayCosts_)
    {
        /* Setup information about this node */
        Node->Predecessor_  = Predecessor;
        Node->WayCosts_     = WayCosts;
    }
}

void PathGraph::constructPath(std::list<PathNode*> &Path, PathNode* NextNode)
{
    if (!NextNode)
        return;
    
    /* Add the first node to the path */
    Path.push_back(NextNode);
    
    /* Repeat the following process until the start node has reached */
    do
    {
        /* Go to the next predecessor */
        if (NextNode->Predecessor_)
            NextNode = NextNode->Predecessor_;
        else
        {
            io::Log::error("Incomplete path while searching in graph");
            break;
        }
        
        /* Add the next node to the path */
        Path.push_back(NextNode);
    }
    while (NextNode != StartNode_);
}


} // /namespace tool

} // /namespace sp


#endif



// ================================================================================
