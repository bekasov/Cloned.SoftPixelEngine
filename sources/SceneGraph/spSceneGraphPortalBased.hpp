/*
 * Portal-based scene graph header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_SCENEGRAPH_PORTAL_BASED_H__
#define __SP_SCENEGRAPH_PORTAL_BASED_H__


#include "Base/spStandard.hpp"

#ifdef SP_COMPILE_WITH_SCENEGRAPH_PORTAL_BASED


#include "SceneGraph/spSceneGraph.hpp"


namespace sp
{
namespace scene
{


class Sector;
class Portal;

/**
This is the class for portal-based scene graphs.
\ingroup group_scenegraph
\since Version 3.2
*/
class SP_EXPORT SceneGraphPortalBased : public SceneGraph
{
    
    public:
        
        SceneGraphPortalBased();
        ~SceneGraphPortalBased();
        
        /* === Functions === */
        
        Sector* createSector(const scene::Transformation &Transform);
        void deleteSector(Sector* SectorObj);
        void clearSectors();
        
        Portal* createPortal(const scene::Transformation &Transform);
        Portal* createPortal(const scene::Transformation &Transform, Sector* FrontSector, Sector* BackSector);
        void deletePortal(Portal* PortalObj);
        void clearPortals();
        
        void render();
        
        //! Returns the sector where the specified point is inside.
        Sector* findSector(const dim::vector3df &Point) const;
        
        /**
        Inserts all 'global' render nodes into the sectors.
        This is the opposite functionality of "releaseRenderNodes".
        \see releaseRenderNodes
        */
        void insertRenderNodes();
        /**
        Releases all render nodes from the sectors and makes them 'global' again.
        This is the opposite functionality of "insertRenderNodes".
        \see insertRenderNodes
        */
        void releaseRenderNodes();
        
        /* === Inline functions === */
        
        inline const std::list<Sector*>& getSectorList() const
        {
            return Sectors_;
        }
        inline const std::list<Portal*>& getPortalList() const
        {
            return Portals_;
        }
        
    private:
        
        /* === Members === */
        
        std::list<Sector*> Sectors_;
        std::list<Portal*> Portals_;
        
        std::vector<RenderNode*> GlobalRenderNodes_;
        
};


} // /namespace scene

} // /namespace sp


#endif

#endif



// ================================================================================
