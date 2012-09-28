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

extern video::RenderSystem* __spVideoDriver;

namespace scene
{


Camera::Camera() :
    SceneNode   (NODE_CAMERA            ),
    Viewport_(
        0,
        0,
        gSharedObjects.ScreenWidth,
        gSharedObjects.ScreenHeight
    ),
    NearRange_  (0.25f                  ),
    FarRange_   (1000.0f                ),
    FieldOfView_(DEF_PERSPECTIVE_FOV    ),
    isOrtho_    (false                  ),
    isMirror_   (false                  )
{
    updatePerspective();
}
Camera::Camera(
    const dim::rect2di &Viewport, f32 NearPlane, f32 FarPlane, f32 FieldOfView) :
    SceneNode   (NODE_CAMERA),
    Viewport_   (Viewport   ),
    NearRange_  (NearPlane  ),
    FarRange_   (FarPlane   ),
    FieldOfView_(FieldOfView),
    isOrtho_    (false      ),
    isMirror_   (false      )
{
    updatePerspective();
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

void Camera::updatePerspective()
{
    /* Check which projection matrix is used */
    if (isOrtho_)
    {
        /* Check which projection matrix the renderer is using */
        if (__spVideoDriver && __spVideoDriver->getProjectionMatrixType() == dim::MATRIX_LEFTHANDED)
        {
            ProjectionMatrix_.setOrthoLH(
                static_cast<f32>(Viewport_.Left) / FieldOfView_,
                static_cast<f32>(Viewport_.Left + Viewport_.Right) / FieldOfView_,
                static_cast<f32>(Viewport_.Top) / FieldOfView_,
                static_cast<f32>(Viewport_.Top + Viewport_.Bottom) / FieldOfView_,
                NearRange_, FarRange_
            );
        }
        else
        {
            ProjectionMatrix_.setOrthoRH(
                static_cast<f32>(Viewport_.Left) / FieldOfView_,
                static_cast<f32>(Viewport_.Left + Viewport_.Right) / FieldOfView_,
                static_cast<f32>(Viewport_.Top) / FieldOfView_,
                static_cast<f32>(Viewport_.Top + Viewport_.Bottom) / FieldOfView_,
                NearRange_, FarRange_
            );
        }
    }
    else
    {
        /* Temporary constants */
        const f32 AspectRatio = static_cast<f32>(Viewport_.Right) / Viewport_.Bottom;
        
        /* Check which projection matrix the renderer is using */
        if (__spVideoDriver && __spVideoDriver->getProjectionMatrixType() == dim::MATRIX_LEFTHANDED)
            ProjectionMatrix_.setPerspectiveLH(FieldOfView_, AspectRatio, NearRange_, FarRange_);
        else
            ProjectionMatrix_.setPerspectiveRH(FieldOfView_, AspectRatio, NearRange_, FarRange_);
    }
    
    /* Update the render matrices */
    spProjectionMatrix = ProjectionMatrix_;
    spViewMatrix.reset();
    spWorldMatrix.reset();
}

void Camera::setRange(f32 NearRange, f32 FarRange)
{
    NearRange_  = NearRange;
    FarRange_   = FarRange;
    updatePerspective();
}

void Camera::setRangeNear(f32 NearRange)
{
    NearRange_ = NearRange;
    updatePerspective();
}
void Camera::setRangeFar(f32 FarRange)
{
    FarRange_ = FarRange;
    updatePerspective();
}

void Camera::setFOV(f32 FieldOfView)
{
    FieldOfView_ = FieldOfView;
    updatePerspective();
}

void Camera::setZoom(f32 Zoom)
{
    setFOV(math::ATan(2.0f / Zoom));
}
f32 Camera::getZoom() const
{
    return 1.0f / math::Tan(FieldOfView_ / 2.0f);
}

void Camera::setProjectionMatrix(const dim::matrix4f &Matrix)
{
    ProjectionMatrix_ = Matrix;
    updatePerspective();
}

void Camera::setOrtho(bool isOrtho)
{
    isOrtho_ = isOrtho;
    updatePerspective();
}

void Camera::setViewport(const dim::rect2di &Viewport)
{
    Viewport_ = Viewport;
    updatePerspective();
}

void Camera::setPerspective(const dim::rect2di &Viewport, f32 NearRange, f32 FarRange, f32 FieldOfView)
{
    /* Copy values and update perspective */
    Viewport_       = Viewport;
    NearRange_      = NearRange;
    FarRange_       = FarRange;
    FieldOfView_    = FieldOfView;
    
    updatePerspective();
}

void Camera::getPerspective(dim::rect2di &Viewport, f32 &NearRange, f32 &FarRange, f32 &FieldOfView)
{
    /* Copy values */
    Viewport    = Viewport_;
    NearRange   = NearRange_;
    FarRange    = FarRange_;
    FieldOfView = FieldOfView_;
}

dim::point2di Camera::getProjection(dim::vector3df Position) const
{
    /* Generate the line coordinates in dependent to the frustum culling */
    Position = getTransformMatrix(true).getInverse() * Position;
    
    if (Position.Z <= 0)
        return dim::point2di(-10000, -10000);
    
    dim::point2df ScreenCoord;
    
    /* Compute the 2d coordinates */
    if (isOrtho_)
    {
        ScreenCoord.X =   Position.X * FieldOfView_ + Viewport_.Right /2 + Viewport_.Left;
        ScreenCoord.Y = - Position.Y * FieldOfView_ + Viewport_.Bottom/2 + Viewport_.Top;
    }
    else
    {
        const f32 Aspect = static_cast<f32>(math::STDASPECT) / ( static_cast<f32>(Viewport_.Right) / static_cast<f32>(Viewport_.Bottom) );
        
        ScreenCoord.X =   Position.X / Position.Z * static_cast<f32>(Viewport_.Right/2) * Aspect + Viewport_.Right /2 + Viewport_.Left;
        ScreenCoord.Y = - Position.Y / Position.Z * static_cast<f32>(Viewport_.Right/2) * Aspect + Viewport_.Bottom/2 + Viewport_.Top;
    }
    
    return dim::point2df(ScreenCoord.X, ScreenCoord.Y).cast<s32>();
}

dim::line3df Camera::getPickingLine(const dim::point2di &Position, f32 Length) const
{
    if (Length < 0.0f)
        Length = FarRange_;
    
    /* Temporary variables */
    dim::line3df Line;
    dim::point2df Coord(
        static_cast<f32>(Position.X - Viewport_.Left),
        static_cast<f32>(Position.Y - Viewport_.Top)
    );
    
    const dim::matrix4f Mat(getTransformMatrix(true));
    
    if (isOrtho_)
    {
        dim::point2df Origin(
            Coord.X - f32(Viewport_.Right/2), Coord.Y - f32(Viewport_.Bottom/2)
        );
        
        Origin /= FieldOfView_;
        
        Line.Start  = Mat * dim::vector3df(Origin.X, -Origin.Y, 0.0f);
        Line.End    = Mat * dim::vector3df(Origin.X, -Origin.Y, Length);
    }
    else
    {
        Coord.make3DFrustum(static_cast<f32>(Viewport_.Right), static_cast<f32>(Viewport_.Bottom));
        
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
    __spVideoDriver->setViewport(
        dim::point2di(Viewport_.Left, Viewport_.Top), dim::size2di(Viewport_.Right, Viewport_.Bottom)
    );
    
    __spVideoDriver->setProjectionMatrix(ProjectionMatrix_);
}

void Camera::updateTransformation()
{
    /* Compute view matrix: inverse camera transformation */
    dim::matrix4f ViewMatrix(getTransformMatrix(true).getInverse());
    
    if (isMirror_)
        ViewMatrix *= MirrorMatrix_;
    
    __spVideoDriver->setViewMatrix(ViewMatrix);
    
    /* Update the view-frustum */
    const f32 AspectRatio = static_cast<f32>(Viewport_.Right) / Viewport_.Bottom;
    
    dim::matrix4f ViewFrustumProjection;
    
    if (isOrtho_)
    {
        ViewFrustumProjection.setOrthoLH(
            static_cast<f32>(Viewport_.Left) / FieldOfView_,
            static_cast<f32>(Viewport_.Left + Viewport_.Right) / FieldOfView_,
            static_cast<f32>(Viewport_.Top) / FieldOfView_,
            static_cast<f32>(Viewport_.Top + Viewport_.Bottom) / FieldOfView_,
            NearRange_, FarRange_
        );
    }
    else
        ViewFrustumProjection.setPerspectiveRH(FieldOfView_, AspectRatio, NearRange_, FarRange_);
    
    ViewFrustum_.setFrustum(ViewMatrix, ViewFrustumProjection);
}

Camera* Camera::copy() const
{
    /* Allocate a new camera */
    Camera* NewCamera = MemoryManager::createMemory<Camera>("scene::Camera");
    
    /* Copy the root attributes */
    copyRoot(NewCamera);
    
    /* Copy the camera configurations */
    NewCamera->ProjectionMatrix_    = ProjectionMatrix_;
    NewCamera->MirrorMatrix_        = MirrorMatrix_;
    NewCamera->ViewFrustum_         = ViewFrustum_;
    NewCamera->Viewport_            = Viewport_;
    
    NewCamera->NearRange_           = NearRange_;
    NewCamera->FarRange_            = FarRange_;
    NewCamera->FieldOfView_         = FieldOfView_;
    
    NewCamera->isOrtho_             = isOrtho_;
    NewCamera->isMirror_            = isMirror_;
    
    /* Return the new sprite */
    return NewCamera;
}


} // /namespace scene

} // /namespace sp



// ================================================================================
