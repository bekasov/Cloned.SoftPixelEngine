/*
 * Camera scene node file
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#include "SceneGraph/spSceneCamera.hpp"
#include "Platform/spSoftPixelDeviceOS.hpp"
#include "Base/spSharedObjects.hpp"


namespace sp
{

extern video::RenderSystem* GlbRenderSys;

namespace scene
{


Camera::Camera() :
    SceneNode   (NODE_CAMERA                                                                ),
    Projection_ (dim::rect2di(0, 0, gSharedObjects.ScreenWidth, gSharedObjects.ScreenHeight)),
    isMirror_   (false                                                                      )
{
}
Camera::Camera(
    const dim::rect2di &Viewport, f32 NearPlane, f32 FarPlane, f32 FieldOfView) :
    SceneNode   (NODE_CAMERA                                ),
    Projection_ (Viewport, NearPlane, FarPlane, FieldOfView ),
    isMirror_   (false                                      )
{
}
Camera::~Camera()
{
}

void Camera::updateControl()
{
    // do nothing
}
void Camera::drawMenu()
{
    // do nothing
}

void Camera::setRange(f32 NearRange, f32 FarRange)
{
    Projection_.setNearPlane(NearRange);
    Projection_.setFarPlane(FarRange);
}

void Camera::setRangeNear(f32 NearRange)
{
    Projection_.setNearPlane(NearRange);
}
void Camera::setRangeFar(f32 FarRange)
{
    Projection_.setFarPlane(FarRange);
}

void Camera::setFOV(f32 FieldOfView)
{
    Projection_.setFOV(FieldOfView);
}

void Camera::setZoom(f32 Zoom)
{
    Projection_.setZoom(Zoom);
}
f32 Camera::getZoom() const
{
    return Projection_.getZoom();
}

void Camera::setOrtho(bool isOrtho)
{
    Projection_.setOrtho(isOrtho);
}

void Camera::setViewport(const dim::rect2di &Viewport)
{
    Projection_.setViewport(Viewport);
}
void Camera::setPerspective(const dim::rect2di &Viewport, f32 NearRange, f32 FarRange, f32 FieldOfView)
{
    Projection_.setViewport(Viewport);
    Projection_.setNearPlane(NearRange);
    Projection_.setFarPlane(FarRange);
    Projection_.setFOV(FieldOfView);
}

void Camera::getPerspective(dim::rect2di &Viewport, f32 &NearRange, f32 &FarRange, f32 &FieldOfView)
{
    Viewport    = Projection_.getViewport();
    NearRange   = Projection_.getNearPlane();
    FarRange    = Projection_.getFarPlane();
    FieldOfView = Projection_.getFOV();
}

bool Camera::projectPoint(dim::vector4df &Point, f32 NearClippingPlane, f32 FarClippingPlane) const
{
    /* Get view-projection matrix from camera */
    dim::matrix4f ViewProjection(getProjection().getMatrixLH());
    ViewProjection *= getTransformation(true).getInverseMatrix();
    
    /* Transform position with view-projection matrix */
    Point = ViewProjection * Point;
    
    if (Point.Z < getProjection().getNearPlane())
        return false;
    
    /* Apply RHW (Reciprocal Homogeneous W) coordinate */
    Point.X /= Point.W;
    Point.Y /= Point.W;
    Point.Z /= Point.W;
    
    /* Transform point to viewport */
    const dim::point2df ViewportOrigin(getViewport().getLTPoint().cast<f32>());
    const dim::size2df ViewportSize(getViewport().getSize().cast<f32>());
    
    Point.X =  Point.X * ViewportSize.Width  * 0.5f + (ViewportOrigin.X + ViewportSize.Width  * 0.5f);
    Point.Y = -Point.Y * ViewportSize.Height * 0.5f + (ViewportOrigin.Y + ViewportSize.Height * 0.5f);
    Point.Z =  Point.Z * (FarClippingPlane - NearClippingPlane)*0.5f + (FarClippingPlane + NearClippingPlane)*0.5f;
    
    return true;
}

dim::line3df Camera::getPickingLine(const dim::point2di &Position, f32 Length) const
{
    if (Length < 0.0f)
        Length = getRangeFar();
    
    /* Temporary variables */
    dim::line3df Line;
    dim::point2df Coord(
        static_cast<f32>(Position.X - getViewport().Left),
        static_cast<f32>(Position.Y - getViewport().Top)
    );
    
    const dim::matrix4f Mat(getTransformMatrix(true));
    
    if (getOrtho())
    {
        dim::point2df Origin(
            Coord.X - static_cast<f32>(getViewport().Right/2),
            Coord.Y - static_cast<f32>(getViewport().Bottom/2)
        );
        
        Origin /= getFOV();
        
        Line.Start  = Mat * dim::vector3df(Origin.X, -Origin.Y, 0.0f);
        Line.End    = Mat * dim::vector3df(Origin.X, -Origin.Y, Length);
    }
    else
    {
        Coord.make3DFrustum(static_cast<f32>(getViewport().Right), static_cast<f32>(getViewport().Bottom));
        
        Line.Start  = Mat.getPosition();
        Line.End    = Mat * ( dim::vector3df(Coord.X, Coord.Y, 1.0f).normalize() * Length );
    }
    
    /* Return the result */
    return Line;
}

void Camera::lookAt(dim::vector3df Position, bool isGlobal)
{
    /* Settings */
    dim::vector3df Pos(getPosition(isGlobal));
    dim::vector3df Rot(getRotation(isGlobal));
    
    /* Calculate rotation */
    Rot.X = -math::ASin( (Position.Y - Pos.Y) / math::getDistance(Pos, Position) );
    Rot.Y = -math::ASin( (Position.X - Pos.X) / math::getDistance(dim::point2df(Pos.X, Pos.Z), dim::point2df(Position.X, Position.Z)) ) + 180.0f;
    Rot.Z = 0.0f;
    
    if (Pos.Z < Position.Z)
        Rot.Y = 180.0f - Rot.Y;
    
    /* Final rotation */
    setRotation(Rot, isGlobal);
}

void Camera::setupCameraView()
{
    /* Setup viewport and projection matrix */
    GlbRenderSys->setViewport(
        getViewport().getLTPoint(),
        dim::size2di(getViewport().Right, getViewport().Bottom)//getViewport().getSize() //!!!
    );
    GlbRenderSys->setProjectionMatrix(getProjectionMatrix());
}

void Camera::updateTransformation()
{
    /* Compute view matrix: inverse camera transformation */
    dim::matrix4f ViewMatrix(getTransformMatrix(true));
    ViewMatrix.setInverse();
    
    if (isMirror_)
        ViewMatrix *= MirrorMatrix_;
    
    GlbRenderSys->setViewMatrix(ViewMatrix);
    
    /* Update the view-frustum */
    if (getOrtho())
        ViewFrustum_.setFrustum(ViewMatrix, Projection_.getMatrixLH());
    else
        ViewFrustum_.setFrustum(ViewMatrix, Projection_.getMatrixRH());
}

Camera* Camera::copy() const
{
    /* Allocate a new camera */
    Camera* NewCamera = MemoryManager::createMemory<Camera>("scene::Camera");
    
    /* Copy the root attributes */
    copyRoot(NewCamera);
    
    /* Copy the camera configurations */
    NewCamera->Projection_      = Projection_;
    NewCamera->ViewFrustum_     = ViewFrustum_;
    NewCamera->MirrorMatrix_    = MirrorMatrix_;
    NewCamera->isMirror_        = isMirror_;
    
    /* Return the new sprite */
    return NewCamera;
}

const dim::matrix4f& Camera::getProjectionMatrix() const
{
    if (GlbRenderSys && GlbRenderSys->getProjectionMatrixType() == dim::MATRIX_RIGHTHANDED)
        return Projection_.getMatrixRH();
    return Projection_.getMatrixLH();
}


} // /namespace scene

} // /namespace sp



// ================================================================================
