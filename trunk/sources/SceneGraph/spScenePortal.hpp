/*
 * Portal header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_SCENE_PORTAL_H__
#define __SP_SCENE_PORTAL_H__


#include "Base/spStandard.hpp"

#ifdef SP_COMPILE_WITH_SCENEGRAPH_PORTAL_BASED


#include "Base/spViewFrustum.hpp"


namespace sp
{
namespace scene
{


class Sector;

/**
This is the portal class for portal-based scene graphs.
\ingroup group_scenegraph
\since Version 3.2
*/
class SP_EXPORT Portal
{
    
    public:
        
        Portal();
        ~Portal();
        
        /* === Functions === */
        
        /**
        Returns the neighbor sector for the given one. If the given sector is
        not connected to this portal the return value is null.
        */
        Sector* getNeighbor(Sector* SectorObj) const;
        
        //! Returns true if the given sector is connected with this poral.
        bool connected(Sector* SectorObj) const;
        
        //! Returns true if the portal is connected with two sectors.
        bool connected() const;
        
        //! Returns true if this portal is inside the given view frustum.
        bool insideViewFrustum(const ViewFrustum &Frustum) const;
        
        //! Transforms the given view frustum with this portal and returns true if the portal is inside the given frustum.
        bool transformViewFrustum(const dim::vector3df &ViewOrigin, ViewFrustum &Frustum) const;
        
        //! Sets the portal's transformation matrix.
        void setTransformation(const dim::matrix4f &Transform);
        //! Returns the portal's transformation matrix.
        dim::matrix4f getTransformation() const;
        
        /* === Inline functions === */
        
        //! Enables or disables the portal. This is very useful when doors are closed.
        inline void setEnable(bool Enable)
        {
            Enabled_ = Enable;
        }
        //! Returns true if the portal is enabled. By default true.
        inline bool getEnable() const
        {
            return Enabled_;
        }
        
        //! Sets the portal's front sector connection.
        inline void setFrontSector(Sector* SectorObj)
        {
            FrontSector_ = SectorObj;
        }
        //! Returns a pointer to the portal's front sector.
        inline Sector* getFrontSector() const
        {
            return FrontSector_;
        }
        
        //! Sets the portal's back sector connection.
        inline void setBackSector(Sector* SectorObj)
        {
            BackSector_ = SectorObj;
        }
        //! Returns a pointer to the portal's back sector.
        inline Sector* getBackSector() const
        {
            return BackSector_;
        }
        
    protected:
        
        friend class Sector;
        
        /* === Functions === */
        
        /**
        Connects the given sector with this portal and returns true on success.
        Otherwise this portal is already connected with two sectors.
        */
        bool connect(Sector* SectorObj);
        
        /**
        Disconnects the given sector with this portal and returns true on success.
        Otherwise this portal was not connected with the given sector.
        */
        bool disconnect(Sector* SectorObj);
        
    private:
        
        /* === Enumerations === */
        
        enum ERectPoints
        {
            RECTPOINT_LEFTUP = 0,
            RECTPOINT_LEFTDOWN,
            RECTPOINT_RIGHTUP,
            RECTPOINT_RIGHTDOWN,
        };
        
        /* === Functions === */
        
        /**
        Transforms the plane with the given origin and corners and
        swaps if the oposite corners are on the wrong side.
        */
        void transformViewFrustumPlane(
            dim::plane3df &Plane, const dim::vector3df &ViewOrigin,
            const ERectPoints CornerA, const ERectPoints CornerB,
            const ERectPoints OpCornerA, const ERectPoints OpCornerB,
            s32 &TransformCounter
        ) const;
        
        /* === Members === */
        
        dim::matrix4f InvTransform_;
        dim::vector3df Points_[4];
        
        bool Enabled_;
        
        Sector* FrontSector_;
        Sector* BackSector_;
        
};


} // /namespace scene

} // /namespace sp


#endif

#endif



// ================================================================================
