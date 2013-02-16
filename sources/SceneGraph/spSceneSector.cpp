/*
 * Sector file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "SceneGraph/spSceneSector.hpp"

#ifdef SP_COMPILE_WITH_SCENEGRAPH_PORTAL_BASED


#include "Base/spMemoryManagement.hpp"
#include "Base/spMathCollisionLibrary.hpp"
#include "SceneGraph/spScenePortal.hpp"
#include "SceneGraph/spSceneCamera.hpp"
#include "SceneGraph/spRenderNode.hpp"

#include <boost/foreach.hpp>


namespace sp
{
namespace scene
{

Sector::Sector()
{
}
Sector::~Sector()
{
}

bool Sector::addPortal(Portal* PortalObj)
{
    if (PortalObj && PortalObj->connect(this))
    {
        Portals_.push_back(PortalObj);
        return true;
    }
    return false;
}

bool Sector::removePortal(Portal* PortalObj)
{
    if (PortalObj && PortalObj->disconnect(this))
    {
        MemoryManager::removeElement(Portals_, PortalObj);
        return true;
    }
    return false;
}

void Sector::clearPortals()
{
    Portals_.clear();
}

void Sector::addRenderNode(RenderNode* NodeObj)
{
    if (NodeObj && !MemoryManager::hasElement(RenderNodes_, NodeObj))
        RenderNodes_.push_back(NodeObj);
}

void Sector::removeRenderNode(RenderNode* NodeObj)
{
    MemoryManager::removeElement(RenderNodes_, NodeObj);
}

void Sector::clearRenderNodes()
{
    RenderNodes_.clear();
}

f32 Sector::getPointDistance(const dim::vector3df &Point) const
{
    return math::CollisionLibrary::getPointBoxDistance(BoundBox_, Point);
}

bool Sector::isPointInside(const dim::vector3df &Point) const
{
    return ConvexHull_.isPointInside(InvTransform_ * Point);
}

bool Sector::isBoundingVolumeInsideInv(const BoundingVolume &BoundVolume, const dim::matrix4f &InvMatrix) const
{
    switch (BoundVolume.getType())
    {
        case BOUNDING_SPHERE:
            return ConvexHull_.isPointInside(InvTransform_ * (InvMatrix.getPosition()), BoundVolume.getRadius());
        case BOUNDING_BOX:
            return ConvexHull_.isBoundBoxInsideInv(BoundVolume.getBox(), InvMatrix * getTransformation());
        default:
            break;
    }
    return false;
}

bool Sector::isPortalNearby(const Portal* PortalObj, f32 Tolerance) const
{
    if (PortalObj)
    {
        /* Check if one of the portal's corners are nearby this sector */
        for (u32 i = 0; i < 4; ++i)
        {
            if (getPointDistance(PortalObj->getPoint(i)) < Tolerance || isPointInside(PortalObj->getPoint(i)))
                return true;
        }
    }
    return false;
}

void Sector::setTransformation(const dim::matrix4f &Transform)
{
    /* Store inverse transformation */
    InvTransform_ = Transform.getInverse();
    
    /* Store oriented-bounding box */
    BoundBox_ = dim::obbox3df(
        Transform.getPosition(),
        Transform.vecRotate(dim::vector3df(0.5f, 0, 0)),
        Transform.vecRotate(dim::vector3df(0, 0.5f, 0)),
        Transform.vecRotate(dim::vector3df(0, 0, 0.5f))
    );
    
    /* Setup convex polyhedron */
    for (u32 i = 0; i < 6; ++i)
        ConvexHull_.getPlane(i) = dim::aabbox3df::CUBE.getPlane(i);
}

dim::matrix4f Sector::getTransformation() const
{
    return InvTransform_.getInverse();
}


/*
 * ======= Private: =======
 */

void Sector::render(
    Sector* Predecessor, const dim::vector3df &GlobalViewOrigin,
    ViewFrustum &Frustum, const dim::matrix4f &BaseMatrix)
{
    /* Find portals */
    const ViewFrustum OrigFrustum(Frustum);
    
    foreach (Portal* PortalObj, Portals_)
    {
        if (!PortalObj->getEnable())
            continue;
        
        /*Check if this sector has a neighbor within this portal */
        Sector* Neighbor = PortalObj->getNeighbor(this);
        
        if (!Neighbor || Neighbor == Predecessor)
            continue;
        
        /* Transform current view-frustum through the portal */
        if (!PortalObj->transformViewFrustum(GlobalViewOrigin, Frustum))
            continue;
        
        /* Render next sector */
        Neighbor->render(this, GlobalViewOrigin, Frustum, BaseMatrix);
        
        Frustum = OrigFrustum;
    }
    
    /* Draw render nodes of this sector */
    foreach (RenderNode* Node, RenderNodes_)
    {
        if (Node->getVisible())
        {
            Node->updateTransformationBase(BaseMatrix);
            Node->render();
        }
    }
}


} // /namespace scene

} // /namespace sp


#endif



// ================================================================================
 
