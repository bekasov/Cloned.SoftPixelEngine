/*
 * Projection header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_SCENE_PROJECTION_H__
#define __SP_SCENE_PROJECTION_H__


#include "Base/spStandard.hpp"
#include "Base/spMathCore.hpp"
#include "Base/spDimensionRect2D.hpp"
#include "Base/spDimensionMatrix4.hpp"
//#include "Base/spViewport.hpp"


namespace sp
{
namespace scene
{


/**
Base class for projection matrices.
\see matrix4
\since Version 3.2
*/
template <typename T> class Projection3D
{
    
    public:
        
        /* === Macros === */
        
        static const dim::rect2di   DEFAULT_VIEWPORT;
        static const T              DEFAULT_NEAR;
        static const T              DEFAULT_FAR;
        static const T              DEFAULT_FOV;
        
        /* === Constructors & destructor === */
        
        Projection3D() :
            Viewport_   (Projection3D<T>::DEFAULT_VIEWPORT  ),
            NearPlane_  (Projection3D<T>::DEFAULT_NEAR      ),
            FarPlane_   (Projection3D<T>::DEFAULT_FAR       ),
            FieldOfView_(Projection3D<T>::DEFAULT_FOV       ),
            IsOrtho_    (false                              ),
            HasChanged_ (true                               )
        {
        }
        Projection3D(
            const dim::rect2di &Viewport,
            const T &NearPlane = Projection3D<T>::DEFAULT_NEAR,
            const T &FarPlane = Projection3D<T>::DEFAULT_FAR,
            const T &FieldOfView = Projection3D<T>::DEFAULT_FOV,
            bool IsOrtho = false) :
            Viewport_   (Viewport   ),
            NearPlane_  (NearPlane  ),
            FarPlane_   (FarPlane   ),
            FieldOfView_(FieldOfView),
            IsOrtho_    (IsOrtho    ),
            HasChanged_ (true       )
        {
        }
        Projection3D(const Projection3D<T> &Other) :
            Viewport_       (Other.Viewport_    ),
            NearPlane_      (Other.NearPlane_   ),
            FarPlane_       (Other.FarPlane_    ),
            FieldOfView_    (Other.FieldOfView_ ),
            IsOrtho_        (Other.IsOrtho_     ),
            ProjMatrixLH_   (Other.ProjMatrixLH_),
            ProjMatrixRH_   (Other.ProjMatrixRH_),
            HasChanged_     (Other.HasChanged_  )
        {
        }
        ~Projection3D()
        {
        }
        
        /* === Operators === */
        
        Projection3D<T>& operator = (const Projection3D<T> &Other)
        {
            Viewport_       = Other.Viewport_;
            NearPlane_      = Other.NearPlane_;
            FarPlane_       = Other.FarPlane_;
            FieldOfView_    = Other.FieldOfView_;
            IsOrtho_        = Other.IsOrtho_;
            ProjMatrixLH_   = Other.ProjMatrixLH_;
            ProjMatrixRH_   = Other.ProjMatrixRH_;
            HasChanged_     = Other.HasChanged_;
            return *this;
        }
        
        /* === Functions === */
        
        /*
        Updates the projection matrices. This will be called automatically when
        something in the configuration has changed, and "getMatrixLH" or "getMatrixRH" is called.
        \see getMatrixLH
        \see getMatrixRH
        */
        void update() const
        {
            /* Check which projection matrix is used */
            if (IsOrtho_)
            {
                ProjMatrixLH_.setOrthoLH(
                    static_cast<f32>(Viewport_.Left) / FieldOfView_,
                    static_cast<f32>(Viewport_.Left + Viewport_.Right) / FieldOfView_,
                    static_cast<f32>(Viewport_.Top) / FieldOfView_,
                    static_cast<f32>(Viewport_.Top + Viewport_.Bottom) / FieldOfView_,
                    NearPlane_, FarPlane_
                );
                ProjMatrixRH_.setOrthoRH(
                    static_cast<f32>(Viewport_.Left) / FieldOfView_,
                    static_cast<f32>(Viewport_.Left + Viewport_.Right) / FieldOfView_,
                    static_cast<f32>(Viewport_.Top) / FieldOfView_,
                    static_cast<f32>(Viewport_.Top + Viewport_.Bottom) / FieldOfView_,
                    NearPlane_, FarPlane_
                );
            }
            else
            {
                /* Temporary constants */
                const f32 AspectRatio = static_cast<f32>(Viewport_.Right) / Viewport_.Bottom;
                
                /* Check which projection matrix the renderer is using */
                ProjMatrixLH_.setPerspectiveLH(FieldOfView_, AspectRatio, NearPlane_, FarPlane_);
                ProjMatrixRH_.setPerspectiveRH(FieldOfView_, AspectRatio, NearPlane_, FarPlane_);
            }
            
            HasChanged_ = false;
        }
        
        /**
        Returns the left-handed 4x4 projection matrix. This is the default projection matrix.
        Only use the right-handed projection matrix for the OpenGL render sytsem.
        \see getMatrixRH
        */
        const dim::matrix4<T>& getMatrixLH() const
        {
            if (HasChanged_)
                update();
            return ProjMatrixLH_;
        }
        
        /**
        Returns the right-handed 4x4 projection matrix. This is only used for the OpenGL render system.
        \see getMatrixLH
        */
        const dim::matrix4<T>& getMatrixRH() const
        {
            if (HasChanged_)
                update();
            return ProjMatrixRH_;
        }
        
        /* === Inline functions === */
        
        //! Sets the viewport rectangle. By default (0, 0, 100, 100).
        inline void setViewport(const dim::rect2di &Viewport)
        {
            Viewport_ = Viewport;
            HasChanged_ = true;
        }
        //! Returns the viewport rectangle.
        inline const dim::rect2di& getViewport() const
        {
            return Viewport_;
        }
        
        //! Sets the near clipping plane. By default 0.25.
        inline void setNearPlane(const T &NearPlane)
        {
            NearPlane_ = NearPlane;
            HasChanged_ = true;
        }
        //! Returns the near clipping plane.
        inline const T& getNearPlane() const
        {
            return NearPlane_;
        }
        
        //! Sets the far clipping plane. By default 1000.0.
        inline void setFarPlane(const T &FarPlane)
        {
            FarPlane_ = FarPlane;
            HasChanged_ = true;
        }
        //! Returns the far clipping plane.
        inline const T& getFarPlane() const
        {
            return FarPlane_;
        }
        
        //! Sets the field of view (FOV). By default 74.0.
        inline void setFOV(const T &FieldOfView)
        {
            FieldOfView_ = FieldOfView;
            HasChanged_ = true;
        }
        //! Returns the field of view (FOV).
        inline const T& getFOV() const
        {
            return FieldOfView_;
        }
        
        //! Sets the zoom factor. By default 1.0. This uses the setFOV function.
        inline void setZoom(const T &Zoom)
        {
            setFOV(math::ATan(T(2) / Zoom));
        }
        inline T getZoom() const
        {
            return T(1) / math::Tan(getFOV() / T(2));
        }
        
        //! Enables or disables orthogonal projection. By default false.
        inline void setOrtho(bool Enable)
        {
            IsOrtho_ = Enable;
            HasChanged_ = true;
        }
        //! Returns true if orthogonal projection is enabled.
        inline bool getOrtho() const
        {
            return IsOrtho_;
        }
        
    private:
        
        /* === Members === */
        
        //Viewport Viewport_;
        dim::rect2di Viewport_;
        T NearPlane_;
        T FarPlane_;
        T FieldOfView_;
        bool IsOrtho_;
        
        mutable dim::matrix4<T> ProjMatrixLH_;  //!< Left-handed projection matrix (Base projection matrix and used for Direct3D).
        mutable dim::matrix4<T> ProjMatrixRH_;  //!< Right-handed projection matrix (Only used for OpenGL).
        mutable bool HasChanged_;
        
};

template <typename T> const dim::rect2di    Projection3D<T>::DEFAULT_VIEWPORT   = dim::rect2di(0, 0, 100, 100);
template <typename T> const T               Projection3D<T>::DEFAULT_NEAR       = T(0.25);
template <typename T> const T               Projection3D<T>::DEFAULT_FAR        = T(1000);
template <typename T> const T               Projection3D<T>::DEFAULT_FOV        = T(74);


typedef Projection3D<f32> Projection;


} // /namespace scene

} // /namespace sp


#endif



// ================================================================================
