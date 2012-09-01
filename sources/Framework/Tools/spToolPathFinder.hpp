/*
 * Path finder header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_TOOL_PATHFINDER_H__
#define __SP_TOOL_PATHFINDER_H__


#include "Base/spStandard.hpp"

#ifdef SP_COMPILE_WITH_PATHFINDER


#include "Base/spBaseObject.hpp"
#include "Base/spDimension.hpp"

#include <list>
#include <map>


namespace sp
{
namespace tool
{


class PathEdge;

/**
Node class for a graph.
\ingroup group_pathfinding
*/
class SP_EXPORT PathNode : public BaseObject
{
    
    public:
        
        PathNode();
        PathNode(const dim::vector3df &Position, void* Data = 0);
        ~PathNode();
        
        /* Functions */
        
        //! Sets the position and updates all distances between this node and its neighbors.
        void setPosition(const dim::vector3df &Position);
        
        //! Returns a list with all neighbors of this node.
        std::list<PathNode*> getNeighbors() const;
        
        /* Inline functions */
        
        //! Returns the node's position.
        inline dim::vector3df getPosition() const
        {
            return Position_;
        }
        
        //! Returns the list of incident edges.
        inline const std::list<PathEdge*>& getIncidentEdges() const
        {
            return Edges_;
        }
        
    private:
        
        friend class PathEdge;
        friend class PathGraph;
        
        /* Structures */
        
        struct SNeighbor
        {
            SNeighbor(PathNode* Predecessor, PathNode* Neighbor) : Node(Neighbor)
            {
                Distance = math::getDistance(Predecessor->getPosition(), Neighbor->getPosition());
            }
            ~SNeighbor()
            {
            }
            
            PathNode* Node;
            f32 Distance;
        };
        
        /* Functions */
        
        void addEdge(PathEdge* Edge);
        void removeEdge(PathEdge* Edge);
        void updateNeighbors();
        
        /* Inline functions */
        
        inline f32 getMinWayCosts() const
        {
            return WayCosts_ + DirectDistance_;
        }
        
        /* Members */
        
        dim::vector3df Position_;
        
        f32 WayCosts_;
        f32 DirectDistance_;
        
        std::list<PathEdge*> Edges_;
        std::list<SNeighbor> Neighbors_;
        
        PathNode* Predecessor_;
        
};


/**
Edge class for a graph (connects two path nodes).
\ingroup group_pathfinding
*/
class SP_EXPORT PathEdge : public BaseObject
{
    
    public:
        
        PathEdge();
        PathEdge(PathNode* From, PathNode* To, bool Adjusted = false);
        ~PathEdge();
        
        //! Returns pointer to the source PathNode object.
        inline PathNode* getFrom() const
        {
            return From_;
        }
        //! Returns pointer to the target PathNode object.
        inline PathNode* getTo() const
        {
            return To_;
        }
        
        //! Returns true if this edge is adjusted (i.e. points in a direction like a vector).
        inline bool getAdjusted() const
        {
            return Adjusted_;
        }
        
        //! Returns the distance between the two nodes.
        inline f32 getDistance() const
        {
            return Distance_;
        }
        
    private:
        
        friend class PathNode;
        friend class PathGraph;
        
        /* Functions */
        
        void updateNodePosition(PathNode* Node);
        
        /* Members */
        
        PathNode* From_;
        PathNode* To_;
        
        f32 Distance_;
        bool Adjusted_;
        
};


/**
PathGraph objects represent a graph for path finding. The "A* Algorithm" is used for fast path finding.
\ingroup group_pathfinding
*/
class SP_EXPORT PathGraph
{
    
    public:
        
        PathGraph();
        virtual ~PathGraph();
        
        /**
        Adds a new node to the graph. Each node represents a point in the scene.
        \param Position: Specifies the global position in the scene.
        \param Data: Specifies any kind of data linked to the new node object.
        \return Pointer to the new PathNode object.
        */
        PathNode* addNode(const dim::vector3df &Position, void* Data = 0);
        
        //! Removes the spcified PathNode object.
        void removeNode(PathNode* Node);
        
        //! Clears the whole path node list.
        void clearNodeList();
        
        /**
        Adds a new edge to the graph. Each edge represents a connection between two nodes.
        \param From: Specifies the start PathNode object.
        \param To: Specifies the target PathNode object.
        \param Adjusted: Specifies whether the edge is to be adjusted or not.
        By default false which means the connection goes in both forwards and backwards directions.
        \return Pointer to the new PathEdge object. If the "From" and "To" parameters as invalid (null pointer)
        the return value is 0 (null).
        */
        PathEdge* addEdge(PathNode* From, PathNode* To, bool Adjusted = false);
        
        //! Removes the specified PathEdge object.
        void removeEdge(PathEdge* Edge);
        
        //! Clears the whole path edge list.
        void clearEdgeList();
        
        /**
        Creats a grid of nodes connected with edges.
        \param From: Specifies the global start position of the grid.
        \param To: Specifies the global target position of the grid.
        \param Steps: Specifies the count of nodes between the axels.
        e.g. if you want a grid of 10x10x1 nodes (a plane) set this parameter to (10, 10, 1).
        \param Bitmap: Specifies the array of enabled nodes. The array needs to be in the
        following order: [ Z*Steps.Z*Steps.Y + Y*Steps.Y + X ] where X, Y and Z is in
        the range [0, Steps.X/ Y/ Z]. By default empty array which means that the grid
        will be created completely.
        \param DiagonalEdges: Specifies whether diagonal edge connections are to be enabled or not.
        \note Edges will also automatically created for each direct neighbor of each node.
        i.e. if you have a 2D grid (a plane) nearly each node (not these at the border) will
        have 8 neighbors.
        */
        void createGrid(
            const dim::vector3df &From, const dim::vector3df &To, const dim::vector3di &Steps,
            const std::vector<bool> &Bitmap = std::vector<bool>(), bool DiagonalEdges = true
        );
        
        /**
        Trys to find a path from the specified start node to the target node through this path graph.
        \param From: Specifies the start PathNode object.
        \param To: Specifies the target PathNode object.
        \return List with all nodes which form the path. If no path has been found the list is empty.
        */
        virtual std::list<PathNode*> findPath(PathNode* From, PathNode* To);
        
        //! Uses the other "findPath" function but uses the nearest PathNode objects from the specified global positions .
        virtual std::list<PathNode*> findPath(const dim::vector3df &From, const dim::vector3df &To);
        
        /* Inline functions */
        
        //! Returns true if the last searched path has been found. Otherwise false and no path has been found.
        inline bool foundPath() const
        {
            return isSolved_;
        }
        
        //! Returns the node list.
        inline std::list<PathNode*> getNodeList() const
        {
            return NodeList_;
        }
        //! Returns the edge list.
        inline std::list<PathEdge*> getEdgeList() const
        {
            return EdgeList_;
        }
        
    protected:
        
        /* Functions */
        
        bool nextStep();
        
        PathNode* getNextNode();
        void addNodeToQueue(PathNode* Node, PathNode* Predecessor, const f32 WayCosts);
        
        void constructPath(std::list<PathNode*> &Path, PathNode* NextNode);
        
        /* Members */
        
        std::list<PathNode*> NodeList_;
        std::list<PathEdge*> EdgeList_;
        
        std::list<PathNode*> NodeQueue_;
        std::map<PathNode*, bool> ClosedMap_;
        
        PathNode* StartNode_, * TargetNode_;
        
        bool isSolved_;
        
};


} // /namespace tool

} // /namespace sp


#endif

#endif



// ================================================================================
