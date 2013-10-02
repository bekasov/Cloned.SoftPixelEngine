/*
 * Quaternion header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_DIMENSION_QUATERNION_H__
#define __SP_DIMENSION_QUATERNION_H__


#include "Base/spStandard.hpp"
#include "Base/spDimensionVector3D.hpp"
#include "Base/spDimensionMatrix4.hpp"
#include "Base/spVectorArithmetic.hpp"

#include <math.h>


namespace sp
{
namespace dim
{


/**
Quaternion 4 class (X, Y, Z, W).
\ingroup group_data_types
*/
template <typename T> class quaternion4
{
    
    public:
        
        static const size_t NUM = 4;
        
        quaternion4() :
            X(0),
            Y(0),
            Z(0),
            W(1)
        {
        }
        quaternion4(const T &x, const T &y, const T &z) :
            X(0),
            Y(0),
            Z(0),
            W(1)
        {
            set(x, y, z);
        }
        quaternion4(const T &x, const T &y, const T &z, const T &w) :
            X(x),
            Y(y),
            Z(z),
            W(w)
        {
        }
        quaternion4(const vector3d<T> &Vec) :
            X(0),
            Y(0),
            Z(0),
            W(1)
        {
            set(Vec);
        }
        quaternion4(const vector4d<T> &Vec) :
            X(Vec.X),
            Y(Vec.Y),
            Z(Vec.Z),
            W(Vec.W)
        {
        }
        quaternion4(const quaternion4<T> &Other) :
            X(Other.X),
            Y(Other.Y),
            Z(Other.Z),
            W(Other.W)
        {
        }
        quaternion4(const matrix3<T> &Matrix) :
            X(0),
            Y(0),
            Z(0),
            W(1)
        {
            setMatrix(Matrix);
        }
        quaternion4(const matrix4<T> &Matrix) :
            X(0),
            Y(0),
            Z(0),
            W(1)
        {
            setMatrix(Matrix);
        }
        ~quaternion4()
        {
        }
        
        /* === Operators - copying === */
        
        inline quaternion4<T>& operator = (const quaternion4<T> &Other)
        {
            X = Other.X;
            Y = Other.Y;
            Z = Other.Z;
            W = Other.W;
            return *this;
        }
        
        /* === Operators - comparisions === */
        
        inline bool operator == (const quaternion4<T> &Other) const
        {
            return compareVecEqual(*this, Other);
        }
        inline bool operator != (const quaternion4<T> &Other) const
        {
            return compareVecNotEqual(*this, Other);
        }
        
        inline bool operator < (const quaternion4<T> &Other) const
        {
            return compareVecLessThan(*this, Other);
        }
        inline bool operator > (const quaternion4<T> &Other) const
        {
            return compareVecGreaterThan(*this, Other);
        }
        
        inline bool operator <= (const quaternion4<T> &Other) const
        {
            return compareVecLessThanOrEqual(*this, Other);
        }
        inline bool operator >= (const quaternion4<T> &Other) const
        {
            return compareVecGreaterThanOrEqual(*this, Other);
        }
        
        /* === Operators - addition, subtraction, division, multiplication === */
        
        inline quaternion4<T> operator + (const quaternion4<T> &Other) const
        {
            return quaternion4<T>(X + Other.X, Y + Other.Y, Z + Other.Z, W + Other.W);
        }
        inline quaternion4<T>& operator += (const quaternion4<T> &Other)
        {
            X += Other.X; Y += Other.Y; Z += Other.Z; W += Other.W; return *this;
        }
        
        inline quaternion4<T> operator - (const quaternion4<T> &Other) const
        {
            return quaternion4<T>(X - Other.X, Y - Other.Y, Z - Other.Z, W - Other.W);
        }
        inline quaternion4<T>& operator -= (const quaternion4<T> &Other)
        {
            X -= Other.X; Y -= Other.Y; Z -= Other.Z; W -= Other.W; return *this;
        }
        
        inline quaternion4<T> operator / (const quaternion4<T> &Other) const
        {
            return quaternion4<T>(X / Other.X, Y / Other.Y, Z / Other.Z, W / Other.W);
        }
        inline quaternion4<T>& operator /= (const quaternion4<T> &Other)
        {
            X /= Other.X; Y /= Other.Y; Z /= Other.Z; W /= Other.W; return *this;
        }
        
        quaternion4<T> operator * (const quaternion4<T> &Other) const
        {
            quaternion4<T> Result;
            
            Result.W = (Other.W * W) - (Other.X * X) - (Other.Y * Y) - (Other.Z * Z);
            Result.X = (Other.W * X) + (Other.X * W) + (Other.Y * Z) - (Other.Z * Y);
            Result.Y = (Other.W * Y) + (Other.Y * W) + (Other.Z * X) - (Other.X * Z);
            Result.Z = (Other.W * Z) + (Other.Z * W) + (Other.X * Y) - (Other.Y * X);
            
            return Result;
        }
        inline quaternion4<T>& operator *= (const quaternion4<T> &Other)
        {
            *this = *this * Other;
            return *this;
        }
        
        vector3d<T> operator * (const vector3d<T> &Vector) const
        {
            vector3d<T> uv, uuv;
            vector3d<T> qvec(X, Y, Z);
            
            uv = qvec.cross(Vector);
            uuv = qvec.cross(uv);
            uv *= (T(2) * W);
            uuv *= T(2);
            
            /* Result := Vector + uv + uuv */
            uv += uuv;
            uv += Vector;
            return uv;
        }
        
        inline quaternion4 operator / (const T &Size) const
        {
            return quaternion4(X / Size, Y / Size, Z / Size, W / Size);
        }
        inline quaternion4& operator /= (const T &Size)
        {
            X /= Size; Y /= Size; Z /= Size; W /= Size; return *this;
        }
        
        inline quaternion4 operator * (const T &Size) const
        {
            return quaternion4(X * Size, Y * Size, Z * Size, W * Size);
        }
        inline quaternion4& operator *= (const T &Size)
        {
            X *= Size; Y *= Size; Z *= Size; W *= Size; return *this;
        }
        
        /* === Additional operators === */
        
        inline const T& operator [] (size_t i) const
        {
            return *(&X + i);
        }
        inline T& operator [] (size_t i)
        {
            return *(&X + i);
        }

        /* === Functions === */
        
        /**
         * \tparam M Specifies the matrix type. This class must implement the following operator:
         * \code
         * // Must return a valid matrix element with 'Row' and 'Col' values 0, 1 and 2.
         * const T& operator () (size_t Row, size_t Col) const;
         * \ endcode
         * You can use matrix3 and matrix4 as matrix type or your own matrix class.
         * \param[in] Mat Specifies the matrix.
         */
        template <typename M> void setMatrix(const M &Mat)
        {
            T trace = Mat(0, 0) + Mat(1, 1) + Mat(2, 2) + T(1);
            
            if (trace > T(0))
            {
                const T s = T(2) * sqrt(trace);
                X = (Mat(2, 1) - Mat(1, 2)) / s;
                Y = (Mat(0, 2) - Mat(2, 0)) / s;
                Z = (Mat(1, 0) - Mat(0, 1)) / s;
                W = T(0.25) * s;
            }
            else
            {
                if (Mat(0, 0) > Mat(1, 1) && Mat(0, 0) > Mat(2, 2))
                {
                    const T s = T(2) * sqrt(T(1) + Mat(0, 0) - Mat(1, 1) - Mat(2, 2));
                    X = T(0.25) * s;
                    Y = (Mat(0, 1) + Mat(1, 0) ) / s;
                    Z = (Mat(2, 0) + Mat(0, 2) ) / s;
                    W = (Mat(2, 1) - Mat(1, 2) ) / s;
                }
                else if (Mat(1, 1) > Mat(2, 2))
                {
                    const T s = T(2) * sqrt(T(1) + Mat(1, 1) - Mat(0, 0) - Mat(2, 2));
                    X = (Mat(0, 1) + Mat(1, 0) ) / s;
                    Y = T(0.25) * s;
                    Z = (Mat(1, 2) + Mat(2, 1) ) / s;
                    W = (Mat(0, 2) - Mat(2, 0) ) / s;
                }
                else
                {
                    const T s = T(2) * sqrt(T(1) + Mat(2, 2) - Mat(0, 0) - Mat(1, 1));
                    X = (Mat(0, 2) + Mat(2, 0) ) / s;
                    Y = (Mat(1, 2) + Mat(2, 1) ) / s;
                    Z = T(0.25) * s;
                    W = (Mat(1, 0) - Mat(0, 1) ) / s;
                }
            }
            
            normalize();
        }
        
        /**
        Sets the angle axis.
        \param[in] Angle Specifies the angle (in radian).
        \param[in] Axis Specifies the axis which is used to rotate this quaternion around.
        \return Reference to this quaternion
        */
        quaternion4<T>& setAngleAxis(const T &Angle, const vector3d<T> &Axis)
        {
            const T HalfAngle   = T(0.5) * Angle;
            const T Sine        = sin(HalfAngle);
            
            X = Sine * Axis.X;
            Y = Sine * Axis.Y;
            Z = Sine * Axis.Z;
            W = cos(HalfAngle);
            
            return *this;
        }
        
        void getAngleAxis(T &Angle, vector3d<T> &Axis) const
        {
            const T Scale = sqrt(X*X + Y*Y + Z*Z);
            
            if ( ( Scale > T(-1.0e-6) && Scale < T(1.0e-6) ) || W > T(1) || W < T(-1) )
            {
                Axis.X  = T(0);
                Axis.Y  = T(1);
                Axis.Z  = T(0);
                Angle   = T(0);
            }
            else
            {
                const T InvScale = T(1) / Scale;
                Axis.X  = X * InvScale;
                Axis.Y  = Y * InvScale;
                Axis.Z  = Z * InvScale;
                Angle   = T(2) * acos(W);
            }
        }
        
        void getEuler(vector3d<T> &Euler) const
        {
            const T sqX = X*X;
            const T sqY = Y*Y;
            const T sqZ = Z*Z;
            const T sqW = W*W;
            
            T tmp = T(-2) * (X*Z - Y*W);
            
            math::clamp(tmp, T(-1), T(1));
            
            Euler.X = atan2(T(2) * (Y*Z + X*W), -sqX - sqY + sqZ + sqW);
            Euler.Y = asin(tmp);
            Euler.Z = atan2(T(2) * (X*Y + Z*W), sqX - sqY - sqZ + sqW);
        }
        
        /**
        Spherically linear interpolates the quaternion.
        \param[in] to Specifies the quaternion to which this quaternion is about to be interpolated.
        \param[in] t Specifies the interpolation. Should be in the range [0.0 .. 1.0].
        */
        void slerp(const quaternion4<T> &to, const T &t)
        {
            /* Temporary variables */
            T to1[4];
            T omega, cosom, sinom;
            T scale0, scale1;
            
            /* Calculate cosine */
            cosom = X*to.X + Y*to.Y + Z*to.Z + W*to.W;
            
            /* Adjust signs (if necessary) */
            if (cosom < T(0))
            {
                cosom = -cosom;
                to1[0] = -to.X;
                to1[1] = -to.Y;
                to1[2] = -to.Z;
                to1[3] = -to.W;
            }
            else
            {
                to1[0] = to.X;
                to1[1] = to.Y;
                to1[2] = to.Z;
                to1[3] = to.W;
            }
            
            /* Calculate coefficients */
            if ( ( T(1) - cosom ) > T(1e-10) )
            {
                /* Standard case (slerp) */
                omega = acos(cosom);
                sinom = sin(omega);
                scale0 = sin((T(1) - t) * omega) / sinom;
                scale1 = sin(t * omega) / sinom;
            }
            else
            {
                /*
                 * "from" and "to" quaternions are very close
                 *  ... so we can do a linear interpolation
                 */
                scale0 = T(1) - t;
                scale1 = t;
            }
            
            /* Calculate final values */
            X = scale0*X + scale1*to1[0];
            Y = scale0*Y + scale1*to1[1];
            Z = scale0*Z + scale1*to1[2];
            W = scale0*W + scale1*to1[3];
        }
        
        /**
        Spherically linear interpolates the quaternion.
        \param[in] from Specifies the quaternion from which this quaternion is to be interpolated.
        \param[in] to Specifies the quaternion to which this quaternion is to be interpolated.
        \param[in] t Specifies the interpolation. Should be in the range [0.0 .. 1.0].
        */
        void slerp(const quaternion4<T> &from, const quaternion4<T> &to, const T &t)
        {
            /* Temporary variables */
            T to1[4];
            T omega, cosom, sinom;
            T scale0, scale1;
            
            /* Calculate cosine */
            cosom = from.X*to.X + from.Y*to.Y + from.Z*to.Z + from.W*to.W;
            
            /* Adjust signs (if necessary) */
            if (cosom < T(0))
            {
                cosom = -cosom;
                to1[0] = -to.X;
                to1[1] = -to.Y;
                to1[2] = -to.Z;
                to1[3] = -to.W;
            }
            else
            {
                to1[0] = to.X;
                to1[1] = to.Y;
                to1[2] = to.Z;
                to1[3] = to.W;
            }
            
            /* Calculate coefficients */
            if ((T(1) - cosom) > T(1e-10)) 
            {
                /* Standard case (slerp) */
                omega = acos(cosom);
                sinom = sin(omega);
                scale0 = sin((T(1) - t) * omega) / sinom;
                scale1 = sin(t * omega) / sinom;
            }
            else
            {        
                /*
                 * "from" and "to" quaternions are very close 
                 *  ... so we can do a linear interpolation
                 */
                scale0 = T(1) - t;
                scale1 = t;
            }

            /* Calculate final values */
            X = scale0*from.X + scale1*to1[0];
            Y = scale0*from.Y + scale1*to1[1];
            Z = scale0*from.Z + scale1*to1[2];
            W = scale0*from.W + scale1*to1[3];
        }
        
        //! Sets the quaternion rotation with the specified euler angles (in radian).
        void set(const T &x, const T &y, const T &z)
        {
            const T cp = cos(x/2);
            const T cr = cos(z/2);
            const T cy = cos(y/2);
            
            const T sp = sin(x/2);
            const T sr = sin(z/2);
            const T sy = sin(y/2);
            
            const T cpcy = cp * cy;
            const T spsy = sp * sy;
            const T cpsy = cp * sy;
            const T spcy = sp * cy;
            
            X = sr * cpcy - cr * spsy;
            Y = cr * spcy + sr * cpsy;
            Z = cr * cpsy - sr * spcy;
            W = cr * cpcy + sr * spsy;
            
            normalize();
        }
        
        //! Returns the quaternion rotation as 3x3 matrix.
        void getMatrix(matrix3<T> &Mat) const
        {
            Mat[0] = T(1) - T(2)*Y*Y - T(2)*Z*Z;
            Mat[1] =        T(2)*X*Y + T(2)*Z*W;
            Mat[2] =        T(2)*X*Z - T(2)*Y*W;
            
            Mat[3] =        T(2)*X*Y - T(2)*Z*W;
            Mat[4] = T(1) - T(2)*X*X - T(2)*Z*Z;
            Mat[5] =        T(2)*Z*Y + T(2)*X*W;
            
            Mat[6] =        T(2)*X*Z + T(2)*Y*W;
            Mat[7] =        T(2)*Z*Y - T(2)*X*W;
            Mat[8] = T(1) - T(2)*X*X - T(2)*Y*Y;
        }
        
        //! Returns the quaternion rotation as 4x4 matrix.
        void getMatrix(matrix4<T> &Mat) const
        {
            Mat[ 0] = T(1) - T(2)*Y*Y - T(2)*Z*Z;
            Mat[ 1] =        T(2)*X*Y + T(2)*Z*W;
            Mat[ 2] =        T(2)*X*Z - T(2)*Y*W;
            Mat[ 3] =        T(0);
            
            Mat[ 4] =        T(2)*X*Y - T(2)*Z*W;
            Mat[ 5] = T(1) - T(2)*X*X - T(2)*Z*Z;
            Mat[ 6] =        T(2)*Z*Y + T(2)*X*W;
            Mat[ 7] =        T(0);
            
            Mat[ 8] =        T(2)*X*Z + T(2)*Y*W;
            Mat[ 9] =        T(2)*Z*Y - T(2)*X*W;
            Mat[10] = T(1) - T(2)*X*X - T(2)*Y*Y;
            Mat[11] =        T(0);
            
            Mat[12] = T(0);
            Mat[13] = T(0);
            Mat[14] = T(0);
            Mat[15] = T(1);
        }
        
        //! Returns the quaternion rotation as transposed 3x3 matrix.
        void getMatrixTransposed(matrix3<T> &Mat) const
        {
            Mat[0] = T(1) - T(2)*Y*Y - T(2)*Z*Z;
            Mat[3] =        T(2)*X*Y + T(2)*Z*W;
            Mat[6] =        T(2)*X*Z - T(2)*Y*W;
            
            Mat[1] =        T(2)*X*Y - T(2)*Z*W;
            Mat[4] = T(1) - T(2)*X*X - T(2)*Z*Z;
            Mat[7] =        T(2)*Z*Y + T(2)*X*W;
            
            Mat[2] =        T(2)*X*Z + T(2)*Y*W;
            Mat[5] =        T(2)*Z*Y - T(2)*X*W;
            Mat[8] = T(1) - T(2)*X*X - T(2)*Y*Y;
        }
        
        //! Returns the quaternion rotation as transposed 4x4 matrix.
        void getMatrixTransposed(matrix4<T> &Mat) const
        {
            Mat[ 0] = T(1) - T(2)*Y*Y - T(2)*Z*Z;
            Mat[ 4] =        T(2)*X*Y + T(2)*Z*W;
            Mat[ 8] =        T(2)*X*Z - T(2)*Y*W;
            Mat[12] =        T(0);
            
            Mat[ 1] =        T(2)*X*Y - T(2)*Z*W;
            Mat[ 5] = T(1) - T(2)*X*X - T(2)*Z*Z;
            Mat[ 9] =        T(2)*Z*Y + T(2)*X*W;
            Mat[13] =        T(0);
            
            Mat[ 2] =        T(2)*X*Z + T(2)*Y*W;
            Mat[ 6] =        T(2)*Z*Y - T(2)*X*W;
            Mat[10] = T(1) - T(2)*X*X - T(2)*Y*Y;
            Mat[14] =        T(0);
            
            Mat[ 3] = T(0);
            Mat[ 7] = T(0);
            Mat[11] = T(0);
            Mat[15] = T(1);
        }
        
        /* === Inline functions === */
        
        inline T dot(const quaternion4<T> &Other) const
        {
            return dim::dot(*this, Other);
        }
        
        inline quaternion4<T>& normalize()
        {
            dim::normalize(*this);
            return *this;
        }
        
        inline quaternion4& setInverse()
        {
            X = -X; Y = -Y; Z = -Z; return *this;
        }
        inline quaternion4 getInverse() const
        {
            return quaternion4(-X, -Y, -Z, W);
        }
        
        //! \see set(const T &x, const T &y, const T &z)
        inline void set(const vector3d<T> &Vector)
        {
            set(Vector.X, Vector.Y, Vector.Z);
        }
        
        inline matrix4<T> getMatrix() const
        {
            matrix4<T> Mat;
            getMatrix(Mat);
            return Mat;
        }
        
        inline matrix4<T> getMatrixTransposed() const
        {
            matrix4<T> Mat;
            getMatrixTransposed(Mat);
            return Mat;
        }
        
        //! Loads the identity (quaternion(0, 0, 0, 1)).
        inline void reset()
        {
            X = Y = Z = T(0);
            W = T(1);
        }
        
        /* === Members === */
        
        T X, Y, Z, W;
        
};


typedef quaternion4<f32> quaternion;    //<! Default quaternion type.
typedef quaternion4<f32> quaternion4f;
typedef quaternion4<f64> quaternion4d;


} // /namespace dim

} // /namespace sp


#endif



// ================================================================================
