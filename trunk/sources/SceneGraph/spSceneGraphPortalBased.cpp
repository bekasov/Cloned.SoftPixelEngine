/*
 * Portal-based scene graph file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "SceneGraph/spSceneGraphPortalBased.hpp"

#ifdef SP_COMPILE_WITH_SCENEGRAPH_PORTAL_BASED


#include "Base/spMemoryManagement.hpp"
#include "SceneGraph/spSceneSector.hpp"
#include "SceneGraph/spScenePortal.hpp"
#include "SceneGraph/spSceneLight.hpp"
#include "SceneGraph/spRenderNode.hpp"

#include <boost/foreach.hpp>


namespace sp
{
namespace scene
{


SceneGraphPortalBased::SceneGraphPortalBased() :
    SceneGraph(SCENEGRAPH_PORTAL_BASED)
{
}
SceneGraphPortalBased::~SceneGraphPortalBased()
{
    clearSectors();
    clearPortals();
}

Sector* SceneGraphPortalBased::createSector(const scene::Transformation &Transform)
{
    Sector* NewSector = MemoryManager::createMemory<Sector>("scene::Sector");
    Sectors_.push_back(NewSector);
    
    NewSector->setTransformation(Transform.getMatrix());
    
    return NewSector;
}

void SceneGraphPortalBased::deleteSector(Sector* SectorObj)
{
    MemoryManager::removeElement(Sectors_, SectorObj, true);
}

void SceneGraphPortalBased::clearSectors()
{
    MemoryManager::deleteList(Sectors_);
}

Portal* SceneGraphPortalBased::createPortal(const scene::Transformation &Transform)
{
    Portal* NewPortal = MemoryManager::createMemory<Portal>("scene::Portal");
    Portals_.push_back(NewPortal);
    
    NewPortal->setTransformation(Transform.getMatrix());
    
    return NewPortal;
}

Portal* SceneGraphPortalBased::createPortal(const scene::Transformation &Transform, Sector* FrontSector, Sector* BackSector)
{
    Portal* NewPortal = createPortal(Transform);
    
    NewPortal->setFrontSector(FrontSector);
    NewPortal->setBackSector(BackSector);
    
    return NewPortal;
}

void SceneGraphPortalBased::deletePortal(Portal* PortalObj)
{
    MemoryManager::removeElement(Portals_, PortalObj, true);
}

void SceneGraphPortalBased::clearPortals()
{
    MemoryManager::deleteList(Portals_);
}

void SceneGraphPortalBased::render()
{
    /* Update scene graph transformation */
    const dim::matrix4f BaseMatrix(getTransformMatrix(true));
    
    /* Render lights */
    arrangeLightList(LightList_);
    
    s32 LightIndex = 0;
    
    foreach (Light* Node, LightList_)
    {
        if (!Node->getVisible())
            continue;
        if (++LightIndex > MAX_COUNT_OF_LIGHTS)
            break;
        
        spWorldMatrix = BaseMatrix;
        Node->render();
    }
    
    /* Draw portal-based render nodes */
    Camera* ViewCamera = getActiveCamera();
    ViewFrustum& Frustum = ViewCamera->ViewFrustum_;
    
    const dim::vector3df GlobalViewOrigin(ViewCamera->getPosition(true));
    
    Sector* SectorObj = findSector(GlobalViewOrigin);
    
    if (SectorObj)
        SectorObj->render(0, GlobalViewOrigin, Frustum, BaseMatrix);
    #if 0
    else
    {
        const ViewFrustum OrigFrustum(Frustum);
        
        /* If no sector has found, draw all sectors */
        foreach (Sector* Sctr, Sectors_)
        {
            Sctr->render(GlobalViewOrigin, Frustum, BaseMatrix);
            Frustum = OrigFrustum;
        }
    }
    #endif
    
    /* Draw global render nodes */
    foreach (RenderNode* Node, GlobalRenderNodes_)
    {
        if (Node->getVisible())
        {
            Node->updateTransformationBase(BaseMatrix);
            Node->render();
        }
    }
}

Sector* SceneGraphPortalBased::findSector(const dim::vector3df &Point) const
{
    /* Search sector where the given point is inside */
    foreach (Sector* SectorObj, Sectors_)
    {
        if (SectorObj->isPointInside(Point))
            return SectorObj;
    }
    return 0;
}

void SceneGraphPortalBased::connectSectors(f32 DistanceTolerance)
{
    foreach (Portal* PortalObj, Portals_)
    {
        /* Find nearest sectors */
        foreach (Sector* SectorObj, Sectors_)
        {
            if (SectorObj->isPortalNearby(PortalObj, DistanceTolerance))
            {
                if (!SectorObj->addPortal(PortalObj))
                    break;
            }
        }
    }
}

void SceneGraphPortalBased::insertRenderNodes()
{
    /* Check if there are no sectors. If so, only use global render nodes */
    if (Sectors_.empty())
    {
        GlobalRenderNodes_ = RenderList_;
        return;
    }
    
    GlobalRenderNodes_.clear();
    
    /* Insert all render nodes into  */
    foreach (RenderNode* Node, RenderList_)
    {
        //if (Node->isDynamic()) continue; ...
        
        /* Get bounding volume */
        const BoundingVolume& BoundVolume = Node->getBoundingVolume();
        
        if (BoundVolume.getType() == BOUNDING_NONE)
            continue;
        
        const dim::matrix4f InvMatrix(Node->getTransformMatrix(true).getInverse());
        
        /* Find sectors where the current rener node is inside */
        Sector* PrevSectorObj = 0;
        
        foreach (Sector* SectorObj, Sectors_)
        {
            if (SectorObj->isBoundingVolumeInsideInv(BoundVolume, InvMatrix))
            {
                /* Check if the render node is inside several sectors */
                if (PrevSectorObj)
                {
                    /* Can not be inserted in only one sector -> let it as global render node */
                    PrevSectorObj = 0;
                    break;
                }
                
                /* Store current sector as potential render node insertion */
                PrevSectorObj = SectorObj;
            }
        }
        
        /* Insert render node into selected sector or global node list */
        if (PrevSectorObj)
            PrevSectorObj->addRenderNode(Node);
        else
            GlobalRenderNodes_.push_back(Node);
    }
}

void SceneGraphPortalBased::releaseRenderNodes()
{
    //...
}


} // /namespace scene

} // /namespace sp


#endif



// ================================================================================
