/*
 * Sector header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_SCENE_SECTOR_H__
#define __SP_SCENE_SECTOR_H__


#include "Base/spStandard.hpp"

#ifdef SP_COMPILE_WITH_SCENEGRAPH_PORTAL_BASED


#include "Base/spViewFrustum.hpp"
#include "Base/spConvexPolyhedron.hpp"
#include "SceneGraph/spBoundingVolume.hpp"

#include <map>


namespace sp
{
namespace scene
{


class Portal;
class RenderNode;
class Camera;
class SceneGraphPortalBased;

/**
This is the sector class for portal-based scene graphs.
\ingroup group_scenegraph
\since Version 3.2
*/
class SP_EXPORT Sector
{
    
    public:
        
        Sector();
        ~Sector();
        
        /* === Functions === */
        
        bool addPortal(Portal* PortalObj);
        bool removePortal(Portal* PortalObj);
        void clearPortals();
        
        void addRenderNode(RenderNode* NodeObj);
        void removeRenderNode(RenderNode* NodeObj);
        void clearRenderNodes();
        
        //! Returns the distance between the given point and this sector's bounding box.
        f32 getPointDistance(const dim::vector3df &Point) const;
        
        //! Returns true if the given point is inside this sector's convex hull.
        bool isPointInside(const dim::vector3df &Point) const;
        
        bool isBoundingVolumeInsideInv(const BoundingVolume &BoundVolume, const dim::matrix4f &InvMatrix) const;
        
        /**
        Check is if the portal is nearby the sector.
        \param[in] PortalObj Pointer to the portal object which is to be tested against this sector.
        \param[in] Tolerance Specifies the distance tolerance value. By default 0.1.
        \return Ture if the given portal is nearby this sector or inside the sector.
        */
        bool isPortalNearby(const Portal* PortalObj, f32 Tolerance = 0.1f) const;
        
        void setTransformation(const dim::matrix4f &Transform);
        dim::matrix4f getTransformation() const;
        
    private:
        
        friend class SceneGraphPortalBased;
        
        /* === Functions === */
        
        void render(
            Sector* Predecessor, const dim::vector3df &GlobalViewOrigin,
            ViewFrustum &Frustum, const dim::matrix4f &BaseMatrix
        );
        
        /* === Members === */
        
        dim::matrix4f InvTransform_;
        dim::obbox3df BoundBox_;
        ConvexPolyhedron<f32, 6> ConvexHull_;
        
        std::vector<Portal*> Portals_;
        std::vector<RenderNode*> RenderNodes_;
        
};


} // /namespace scene

} // /namespace sp


#endif

#endif



// ================================================================================
