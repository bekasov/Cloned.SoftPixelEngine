/*
 * Camera scene node header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_SCENE_CAMERA_H__
#define __SP_SCENE_CAMERA_H__


#include "Base/spStandard.hpp"
#include "Base/spDimension.hpp"
#include "Base/spMathViewFrustum.hpp"
#include "SceneGraph/spSceneNode.hpp"
#include "SceneGraph/spSceneMesh.hpp"

#include <vector>


namespace sp
{
namespace scene
{


/*
 * Macros
 */

static const f32 CAMERADEF_PICKING_LENGTH   = -1.0f;
static const f32 CAMERADEF_RANGE_NEAR       = 1.0f;
static const f32 CAMERADEF_RANGE_FAR        = 1000.0f;


/*
 * Camera class
 */

class SP_EXPORT Camera : public SceneNode
{
    
    public:
        
        Camera();
        virtual ~Camera();
        
        /* === Functions === */
        
        /**
        Updates the camera interaction. This can be used derived cameras like the FirstPersonCamera.
        If you want to use the interaction you have to call this function in your main loop for each frame.
        */
        virtual void updateControl();
        
        /**
        Draws the menu for an interaction camera like the FirstPersonCamera on Android or iOS.
        The base Camera class does not draw any menu. If you have an interaction camera you have to
        draw the menu in the forground and in 2d.
        \see RenderSystem::beginDrawing2D, RenderSystem::endDrawing2D.
        */
        virtual void drawMenu();
        
        //! Updates the current render states to the perspective configuration of this camera.
        void updatePerspective();
        
        /**
        Sets multiple camera configurations concurrently.
        \param Viewport: Camera's viewport in screen coordinates.
        \param NearRange: Range for the camera's near clipping plane. By default 1.0.
        \param FarRange: Range for the camera's far clipping plane. By default 1000.0.
        \param FieldOfView: Angle of FOV (Field-of-view). By default 74.0.
        */
        void setPerspective(const dim::rect2di &Viewport, f32 NearRange, f32 FarRange, f32 FieldOfView = DEF_PERSPECTIVE_FOV);
        void getPerspective(dim::rect2di &Viewport, f32 &NearRange, f32 &FarRange, f32 &FieldOfView);
        
        /**
        Makes a projection from 3D to 2D.
        \param Position: 3D point which is to be projected to 2D.
        \return 2D point which is projected by the global transformation of this camera.
        */
        dim::point2di getProjection(dim::vector3df Position) const;
        
        /**
        Computes the picking line (or ray) using the global transformation and projection of this camera.
        \param Position: 2D point in normal screen space. If you want to pick with your cursor just use the cursor position.
        \param Length: Length (or depth) of the line. If you only want to pick 2 or 3 units forwards you can save
        calculation time.
        \return 3D line which can be used for further picking calculations. Use this line when calling "pickIntersection"
        from the CollisionDetector class.
        */
        dim::line3df getPickingLine(const dim::point2di &Position, f32 Length = CAMERADEF_PICKING_LENGTH) const;
        
        void lookAt(dim::vector3df Position, bool isGlobal = false);
        
        /**
        Setups the render view. This includes the viewport, projection matrix, view matrix and view frustum.
        Call this function before render the scene for this camera.
        */
        inline void setupRenderView()
        {
            setupCameraView();
            updateTransformation();
        }
        
        //! Setups viewport and projection matrix.
        virtual void setupCameraView();
        virtual void updateTransformation();
        
        Camera* copy() const;
        
        /* === Inline functions === */
        
        /**
        Sets the camera view range or the near- and far clipping planes.
        By defualt the near-clipping-plane is 1.0 and the far-clipping-plane is 1000.0.
        \param NearRange: Range for the near clipping plane. Needs to be greater then 0.
        \param FarRange: Range for the far clipping plane. Needs to be greater then NearRange.
        */
        inline void setRange(f32 NearRange, f32 FarRange)
        {
            setPerspective(Viewport_, NearRange, FarRange, FieldOfView_);
        }
        inline void getRange(f32 &NearRange, f32 &FarRange) const
        {
            NearRange   = NearRange_;
            FarRange    = FarRange_;
        }
        
        //! Sets only the near clipping plane of the camera view range.
        inline void setRangeNear(f32 NearRange)
        {
            setPerspective(Viewport_, NearRange, FarRange_, FieldOfView_);
        }
        inline f32 getRangeNear() const
        {
            return NearRange_;
        }
        
        //! Sets only the far clipping plane of the camera view range.
        inline void setRangeFar(f32 FarRange)
        {
            setPerspective(Viewport_, NearRange_, FarRange, FieldOfView_);
        }
        inline f32 getRangeFar() const
        {
            return FarRange_;
        }
        
        /**
        Sets the FOV (Field-of-view) angle. By default 74.0 which causes a typical view frustum.
        \param FieldOfView: Angle for the field of view. The range shall be: 0 < ZoomAngle < 180.
        */
        inline void setFOV(f32 FieldOfView)
        {
            setPerspective(Viewport_, NearRange_, FarRange_, FieldOfView);
        }
        inline f32 getFOV() const
        {
            return FieldOfView_;
        }
        
        /**
        Sets the camera zoom (By default 1.0).
        \param Zoom: Zoom factor for the camera's view frustum.
        */
        inline void setZoom(f32 Zoom)
        {
            setFOV(static_cast<f32>(ATAN(2.0 / Zoom)));
        }
        inline f32 getZoom() const
        {
            return static_cast<f32>(1.0 / TAN(FieldOfView_ / 2.0));
        }
        
        inline void setPerspectiveMatrix(const dim::matrix4f &Matrix)
        {
            PerspectiveMatrix_ = Matrix;
        }
        inline dim::matrix4f getPerspectiveMatrix() const
        {
            return PerspectiveMatrix_;
        }
        
        //! Enables or disables the orthographic view. With this you can switch between an orthographic or a perspective view.
        inline void setOrtho(bool isOrtho)
        {
            isOrtho_ = isOrtho;
        }
        inline bool getOrtho() const
        {
            return isOrtho_;
        }
        
        /**
        Sets the camera's viewport in screen space.
        \param Viewport: Viewport or view area for the camera. In this case the two parameters of
        rect2di ("Right" and "Bottom") specifie the size (width and height).
        */
        inline void setViewport(const dim::rect2di &Viewport)
        {
            setPerspective(Viewport, NearRange_, FarRange_, FieldOfView_);
        }
        inline dim::rect2di getViewport() const
        {
            return Viewport_;
        }
        
        /**
        Sets the mirror transformation matrix. See more about the mirror matrix at the "setMirror" function.
        \param Matrix: Transformation matrix which is process in the "updateModelviewMatrix" when
        the mirror transformation is enabled by "setMirror".
        */
        inline void setMirrorMatrix(const dim::matrix4f &Matrix)
        {
            MirrorMatrix_ = Matrix;
        }
        inline dim::matrix4f getMirrorMatrix() const
        {
            return MirrorMatrix_;
        }
        
        /**
        Enables or disables the mirror transformation. The transformation is
        performed by the MirrorMatrix which can be set by "setMirrorMatrix".
        \param isMirror: Specifies if the mirror transformation it to be enabled or disabled.
        If true the MirrorMatrix is multiplied by the ViewMatrix (see also: VideoDriver::setViewMatrix)
        after the basic camera transformations which are process in the "updateModelviewMatrix" function.
        This additional transformation can be used e.g. for water reflection effects when the scene
        is rendered a second time.
        */
        inline void setMirror(bool isMirror)
        {
            isMirror_ = isMirror;
        }
        inline bool getMirror() const
        {
            return isMirror_;
        }
        
        //! Returns camera's view frustum consisting of 6 planes which describe the view's volume.
        inline math::ViewFrustum getViewFrustum() const
        {
            return ViewFrustum_;
        }
        
    protected:
        
        /* Members */
        
        dim::matrix4f PerspectiveMatrix_;
        dim::matrix4f MirrorMatrix_;
        dim::rect2di Viewport_;
        math::ViewFrustum ViewFrustum_;
        
        f32 NearRange_, FarRange_;
        f32 FieldOfView_;
        
        bool isOrtho_;
        bool isMirror_;
        
};


} // /namespace scene

} // /namespace sp


#endif



// ================================================================================
