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
        
        void addPortal(Portal* PortalObj);
        void removePortal(Portal* PortalObj);
        void clearPortals();
        
        void addRenderNode(RenderNode* NodeObj);
        void removeRenderNode(RenderNode* NodeObj);
        void clearRenderNodes();
        
        bool isPointInside(const dim::vector3df &Point) const;
        bool isBoundingVolumeInsideInv(const BoundingVolume &BoundVolume, const dim::matrix4f &InvMatrix) const;
        
        void setTransformation(const dim::matrix4f &Transform);
        dim::matrix4f getTransformation() const;
        
    private:
        
        friend class SceneGraphPortalBased;
        
        /* === Functions === */
        
        void render(
            const dim::vector3df &GlobalViewOrigin, ViewFrustum &Frustum,
            const dim::matrix4f &BaseMatrix, std::map<Sector*, bool> &TraversedSectors
        );
        
        /* === Members === */
        
        dim::matrix4f InvTransform_;
        ConvexPolyhedron<f32, 6> ConvexHull_;
        
        std::vector<Portal*> Portals_;
        std::vector<RenderNode*> RenderNodes_;
        
};


} // /namespace scene

} // /namespace sp


#endif

#endif



// ================================================================================
