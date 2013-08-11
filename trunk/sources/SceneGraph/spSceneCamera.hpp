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
#include "Base/spViewFrustum.hpp"
#include "Base/spProjection.hpp"
#include "SceneGraph/spSceneNode.hpp"
#include "SceneGraph/spSceneMesh.hpp"

#include <vector>


namespace sp
{
namespace scene
{


//! Default picking length (A value < 0 means camera's far range).
static const f32 DEF_PICKING_LENGTH = -1.0f;

#ifdef SP_COMPILE_WITH_SCENEGRAPH_PORTAL_BASED
class SceneGraphPortalBased;
#endif


/**
Base class for camera objects.
\see FirstPersonCamera
\see TrackingCamera
\see BlenderCamera
*/
class SP_EXPORT Camera : public SceneNode
{
    
    public:
        
        Camera();
        Camera(const dim::rect2di &Viewport, f32 NearPlane, f32 FarPlane, f32 FieldOfView = DEF_PERSPECTIVE_FOV);
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
        
        /**
        Sets the camera view range or the near- and far clipping planes.
        By defualt the near-clipping-plane is 1.0 and the far-clipping-plane is 1000.0.
        \param NearRange: Range for the near clipping plane. Must be greater then 0. By default 0.25.
        \param FarRange: Range for the far clipping plane. Must be greater then NearRange. By default 1000.
        */
        void setRange(f32 NearRange, f32 FarRange);
        
        //! Sets only the near clipping plane of the camera view range. By default 0.25.
        void setRangeNear(f32 NearRange);
        //! Sets only the far clipping plane of the camera view range. By default 1000.
        void setRangeFar(f32 FarRange);
        
        /**
        Sets the field-of-view angle. By default 74.0 which causes a typical view frustum.
        \param FieldOfView: Angle for the field of view. The range shall be: 0 < ZoomAngle < 180.
        */
        void setFOV(f32 FieldOfView);
        
        //! Sets the zoom factor. By default 1.0. This uses the setFOV function.
        void setZoom(f32 Zoom);
        //! Returns the zoom factor. By default 1.0. This uses the getFOV function.
        f32 getZoom() const;
        
        //! Enables or disables the orthographic view. With this you can switch between an orthographic or a perspective view.
        void setOrtho(bool isOrtho);
        
        /**
        Sets the camera's viewport in screen space.
        \param[in] Viewport Specifies the new viewport for the camera. In this case the two parameters of
        rect2di ("Right" and "Bottom") specify the size (width and height).
        */
        void setViewport(const dim::rect2di &Viewport);
        
        /**
        Sets multiple camera configurations concurrently.
        \param Viewport: Camera's viewport in screen coordinates.
        \param NearRange: Range for the camera's near clipping plane. By default 1.0.
        \param FarRange: Range for the camera's far clipping plane. By default 1000.0.
        \param FieldOfView: Angle of FOV (Field-of-view). By default 74.0.
        \deprecated
        */
        void setPerspective(const dim::rect2di &Viewport, f32 NearRange, f32 FarRange, f32 FieldOfView = DEF_PERSPECTIVE_FOV);
        //! \deprecated
        void getPerspective(dim::rect2di &Viewport, f32 &NearRange, f32 &FarRange, f32 &FieldOfView);
        
        /**
        Performs a projection from 3D (plus RHW coordinate) to 2D.
        \param[in,out] Point Specifies the point which is to be projected. This is a 4D vector
        to also provide the RHW (Reciprocal Homogeneous W) coordinate as fourth component.
        During projection all three coordinates (X, Y and Z) will be devided by this value,
        after the vector has been multiplied by the view-projection matrix from this camera.
        The camera's viewport is also taken into account.
        \param[in] NearClippingPlane Specifies the near clipping plane.
        This is used to project the point's Z coordinate into clipping space. By default 0.0.
        \param[in] FarClippingPlane Specifies the far clipping plane.
        This is used to project the point's Z coordinate into clipping space. By default 1.0.
        \return True if the point could be projected. Otherwise the point is behind the camera.
        \since Version 3.3
        */
        bool projectPoint(dim::vector4df &Point, f32 NearClippingPlane = 0.0f, f32 FarClippingPlane = 1.0f) const;
        
        /**
        Performs a projection from 3D (plus RHW coordinate) to 2D. For more information see "projectPoint" function.
        \see projectPoint
        \since Version 3.3
        */
        dim::point2di getProjectedPoint(dim::vector4df Point, f32 NearClippingPlane = 0.0f, f32 FarClippingPlane = 1.0f) const;
        
        /**
        Computes the picking line (or ray) using the global transformation and projection of this camera.
        \param Position: 2D point in normal screen space. If you want to pick with your cursor just use the cursor position.
        \param Length: Length (or depth) of the line. If you only want to pick 2 or 3 units forwards you can save
        calculation time.
        \return 3D line which can be used for further picking calculations. Use this line when calling "findIntersection"
        from the CollisionGraph class.
        */
        dim::line3df getPickingLine(const dim::point2di &Position, f32 Length = DEF_PICKING_LENGTH) const;
        
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
        
        /**
        Returns the camera's projection matrix.
        \note This depends on the used render system. For OpenGL a right-handed projection matrix is used
        and for all the other render systems a left-handed projection matrix is used.
        If you want to choose between them use the "getProjection" function
        \see Projection3D
        */
        const dim::matrix4f& getProjectionMatrix() const;
        
        /* === Inline functions === */
        
        //! Sets the camera's projection object.
        inline void setProjection(const Projection &Proj)
        {
            Projection_ = Proj;
        }
        //! Returns a constant reference to the camera's projection object.
        inline const Projection& getProjection() const
        {
            return Projection_;
        }
        //! Returns a reference to the camera's projection object.
        inline Projection& getProjection()
        {
            return Projection_;
        }
        
        /**
        Returns the camera's view frustum, consisting of 6 planes which describe the view's volume.
        \note The camera's view frustum will be updated every time "updateTransformation" is
        called because it depends on the view-matrix.
        */
        inline const ViewFrustum& getViewFrustum() const
        {
            return ViewFrustum_;
        }
        
        //! Returns the near clipping plane range. By default 0.25.
        inline f32 getRangeNear() const
        {
            return Projection_.getNearPlane();
        }
        //! Returns the far clipping plane range. By default 1000.
        inline f32 getRangeFar() const
        {
            return Projection_.getFarPlane();
        }
        
        //! Returns the field-of-view angle. By default 74.0.
        inline f32 getFOV() const
        {
            return Projection_.getFOV();
        }
        
        //! Returns true if the camera projection is orthographic.
        inline bool getOrtho() const
        {
            return Projection_.getOrtho();
        }
        
        //! Returns the camera's viewport. By default (0, 0, ScreeWidth, ScreenHeight).
        inline const dim::rect2di& getViewport() const
        {
            return Projection_.getViewport();
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
        /**
        Returns the mirror matrix.
        \see setMirrorMatrix
        */
        inline const dim::matrix4f& getMirrorMatrix() const
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
        //! Returns true if the mirror matrix is used. By default false.
        inline bool getMirror() const
        {
            return isMirror_;
        }
        
    protected:
        
        #ifdef SP_COMPILE_WITH_SCENEGRAPH_PORTAL_BASED
        friend class SceneGraphPortalBased;
        #endif
        
        /* === Members === */
        
        Projection Projection_;
        ViewFrustum ViewFrustum_;
        
        dim::matrix4f MirrorMatrix_;
        bool isMirror_;
        
};


} // /namespace scene

} // /namespace sp


#endif



// ================================================================================
