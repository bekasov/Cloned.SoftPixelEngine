/*
 * Portal file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "SceneGraph/spScenePortal.hpp"

#ifdef SP_COMPILE_WITH_SCENEGRAPH_PORTAL_BASED


#include "SceneGraph/spSceneSector.hpp"

#define _DEB_FRUSTUM_
#ifdef _DEB_FRUSTUM_
#   include "RenderSystem/spRenderSystem.hpp"
#   include "Base/spInputOutputControl.hpp"
#endif


namespace sp
{
#ifdef _DEB_FRUSTUM_
extern video::RenderSystem* __spVideoDriver;
extern io::InputControl* __spInputControl;
#endif
namespace scene
{


#ifdef _DEB_FRUSTUM_

static ViewFrustum _deb_F;
static dim::vector3df _deb_Origin;
static bool _deb_F_Enabled = false;
static dim::vector3df _deb_v[4];

void _deb_DrawFrustum(
    const ViewFrustum &f_, const dim::vector3df &o_,
    const dim::vector3df &v0, const dim::vector3df &v1,
    const dim::vector3df &v2, const dim::vector3df &v3)
{
    if (__spInputControl->keyHit(io::KEY_SPACE))
    {
        __spInputControl->keyHit(io::KEY_SPACE) = false;
        _deb_F_Enabled = !_deb_F_Enabled;
        
        if (_deb_F_Enabled)
        {
            _deb_F = f_;
            _deb_Origin = o_;
            
            _deb_v[0] = v0;
            _deb_v[1] = v1;
            _deb_v[2] = v2;
            _deb_v[3] = v3;
        }
    }
    
    if (_deb_F_Enabled)
    {
        const ViewFrustum& f = _deb_F;
        
        const dim::vector3df p1(f.getLeftDown());
        const dim::vector3df p2(f.getLeftUp());
        const dim::vector3df p3(f.getRightDown());
        const dim::vector3df p4(f.getRightUp());
        
        static const video::color c(0, 255, 0);
        
        __spVideoDriver->setRenderState(video::RENDER_LIGHTING, false);
        
        __spVideoDriver->draw3DLine(_deb_Origin, p1, c);
        __spVideoDriver->draw3DLine(_deb_Origin, p2, c);
        __spVideoDriver->draw3DLine(_deb_Origin, p3, c);
        __spVideoDriver->draw3DLine(_deb_Origin, p4, c);
        
        __spVideoDriver->setPointSize(15);
        for (u32 i = 0; i < 4; ++i)
            __spVideoDriver->draw3DPoint(_deb_v[i], video::color(0, 0, 255));
        __spVideoDriver->setPointSize(1);
        
        __spVideoDriver->setRenderState(video::RENDER_LIGHTING, true);
    }
}

#endif

Portal::Portal() :
    Enabled_    (true   ),
    FrontSector_(0      ),
    BackSector_ (0      )
{
}
Portal::~Portal()
{
}

Sector* Portal::getNeighbor(Sector* SectorObj) const
{
    if (FrontSector_ == SectorObj)
        return BackSector_;
    if (BackSector_ == SectorObj)
        return FrontSector_;
    return 0;
}

bool Portal::connected(Sector* SectorObj) const
{
    return FrontSector_ == SectorObj || BackSector_ == SectorObj;
}

bool Portal::connected() const
{
    return FrontSector_ != 0 && BackSector_ != 0;
}

bool Portal::insideViewFrustum(const ViewFrustum &Frustum) const
{
    //...
    return false;
}

bool Portal::transformViewFrustum(const dim::vector3df &ViewOrigin, ViewFrustum &Frustum) const
{
    /* Check if the portal is inside the frustum */
    static const dim::aabbox3df BoundBox(
        dim::vector3df(-0.5f, -0.5f, 0.0f), dim::vector3df(0.5f, 0.5f, 0.0f)
    );
    
    if (!Frustum.isBoundBoxInsideInv(BoundBox, InvTransform_))
        return false;
    
    /* Compute new frustum planes (but not the near- and far clipping plane) */
    s32 TransformCounter = 0;
    
    transformViewFrustumPlane(
        Frustum.getPlane(VIEWFRUSTUM_LEFT), ViewOrigin,
        RECTPOINT_LEFTDOWN, RECTPOINT_LEFTUP,
        RECTPOINT_RIGHTUP, RECTPOINT_RIGHTDOWN,
        TransformCounter
    );
    transformViewFrustumPlane(
        Frustum.getPlane(VIEWFRUSTUM_RIGHT), ViewOrigin,
        RECTPOINT_RIGHTUP, RECTPOINT_RIGHTDOWN,
        RECTPOINT_LEFTDOWN, RECTPOINT_LEFTUP,
        TransformCounter
    );
    transformViewFrustumPlane(
        Frustum.getPlane(VIEWFRUSTUM_TOP), ViewOrigin,
        RECTPOINT_LEFTUP, RECTPOINT_RIGHTUP,
        RECTPOINT_RIGHTDOWN, RECTPOINT_LEFTDOWN,
        TransformCounter
    );
    transformViewFrustumPlane(
        Frustum.getPlane(VIEWFRUSTUM_BOTTOM), ViewOrigin,
        RECTPOINT_RIGHTDOWN, RECTPOINT_LEFTDOWN,
        RECTPOINT_LEFTUP, RECTPOINT_RIGHTUP,
        TransformCounter
    );
    
    /* Finalize new frustum */
    if (TransformCounter > 0)
        Frustum.normalize();
    
    #ifdef _DEB_FRUSTUM_
    _deb_DrawFrustum(
        Frustum,
        ViewOrigin,
        Points_[0],
        Points_[1],
        Points_[2],
        Points_[3]
    );
    #endif
    
    return true;
}

void Portal::setTransformation(const dim::matrix4f &Transform)
{
    /* Setup transformation */
    InvTransform_ = Transform.getInverse();
    
    /* Update point transformations */
    Points_[0] = Transform * dim::vector3df(-0.5f,  0.5f, 0.0f);
    Points_[1] = Transform * dim::vector3df(-0.5f, -0.5f, 0.0f);
    Points_[2] = Transform * dim::vector3df( 0.5f,  0.5f, 0.0f);
    Points_[3] = Transform * dim::vector3df( 0.5f, -0.5f, 0.0f);
}

dim::matrix4f Portal::getTransformation() const
{
    return InvTransform_.getInverse();
}


/*
 * ======= Protected: =======
 */

bool Portal::connect(Sector* SectorObj)
{
    if (!FrontSector_)
    {
        FrontSector_ = SectorObj;
        return true;
    }
    if (!BackSector_)
    {
        BackSector_ = SectorObj;
        return true;
    }
    return false;
}

bool Portal::disconnect(Sector* SectorObj)
{
    if (FrontSector_ == SectorObj)
    {
        FrontSector_ = 0;
        return true;
    }
    if (BackSector_ == SectorObj)
    {
        BackSector_ = 0;
        return true;
    }
    return false;
}


/*
 * ======= Private: =======
 */

void Portal::transformViewFrustumPlane(
    dim::plane3df &Plane, const dim::vector3df &ViewOrigin,
    const ERectPoints CornerA, const ERectPoints CornerB,
    const ERectPoints OpCornerA, const ERectPoints OpCornerB,
    s32 &TransformCounter) const
{
    if (!Plane.isPointFrontSide(Points_[CornerA]) || !Plane.isPointFrontSide(Points_[CornerB]))
    {
        Plane = dim::plane3df(ViewOrigin, Points_[CornerA], Points_[CornerB]);
        
        if ( Plane.isPointFrontSide(Points_[OpCornerA]) ||
             Plane.isPointFrontSide(Points_[OpCornerB]) )
        {
            Plane.swap();
        }
        
        ++TransformCounter;
    }
}


} // /namespace scene

} // /namespace sp


#endif



// ================================================================================
 
