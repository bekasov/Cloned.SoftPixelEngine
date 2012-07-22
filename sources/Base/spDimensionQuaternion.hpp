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

#include <math.h>


namespace sp
{
namespace dim
{


class quaternion
{
    
    public:
        
        quaternion() : X(0.0f), Y(0.0f), Z(0.0f), W(1.0f)
        {
        }
        quaternion(f32 NewX, f32 NewY, f32 NewZ)
        {
            set(NewX, NewY, NewZ);
        }
        quaternion(f32 NewX, f32 NewY, f32 NewZ, f32 NewW)
        {
            set(NewX, NewY, NewZ, NewW);
        }
        quaternion(const vector3df &Vector)
        {
            set(Vector);
        }
        quaternion(const vector4df &Vector)
        {
            set(Vector);
        }
        quaternion(const quaternion &other) : X(other.X), Y(other.Y), Z(other.Z), W(other.W)
        {
        }
        quaternion(const matrix4f &Matrix)
        {
            setMatrix(Matrix);
        }
        ~quaternion()
        {
        }
        
        /* === Operators - copying === */
        
        inline quaternion& operator = (const quaternion &other)
        {
            set(other.X, other.Y, other.Z, other.W); return *this;
        }
        
        /* === Operators - comparisions === */
        
        inline bool operator == (const quaternion &other) const
        {
            return X == other.X && Y == other.Y && Z == other.Z && W == other.W;
        }
        inline bool operator != (const quaternion &other) const
        {
            return X != other.X || Y != other.Y || Z != other.Z || W != other.W;
        }
        
        inline bool operator < (const quaternion &other) const
        {
            return (X == other.X) ? ( (Y == other.Y) ? ( (Z == other.Z) ? W < other.W : Z < other.Z ) : Y < other.Y ) : X < other.X;
        }
        inline bool operator > (const quaternion &other) const
        {
            return (X == other.X) ? ( (Y == other.Y) ? ( (Z == other.Z) ? W > other.W : Z > other.Z ) : Y > other.Y ) : X > other.X;
        }
        
        inline bool operator <= (const quaternion &other) const
        {
            return (X == other.X) ? ( (Y == other.Y) ? ( (Z == other.Z) ? W <= other.W : Z <= other.Z ) : Y <= other.Y ) : X <= other.X;
        }
        inline bool operator >= (const quaternion &other) const
        {
            return (X == other.X) ? ( (Y == other.Y) ? ( (Z == other.Z) ? W >= other.W : Z >= other.Z ) : Y >= other.Y ) : X >= other.X;
        }
        
        /* === Operators - addition, subtraction, division, multiplication === */
        
        inline quaternion operator + (const quaternion &other) const
        {
            return quaternion(X + other.X, Y + other.Y, Z + other.Z, W + other.W);
        }
        inline quaternion& operator += (const quaternion &other)
        {
            X += other.X; Y += other.Y; Z += other.Z; W += other.W; return *this;
        }
        
        inline quaternion operator - (const quaternion &other) const
        {
            return quaternion(X - other.X, Y - other.Y, Z - other.Z, W - other.W);
        }
        inline quaternion& operator -= (const quaternion &other)
        {
            X -= other.X; Y -= other.Y; Z -= other.Z; W -= other.W; return *this;
        }
        
        inline quaternion operator / (const quaternion &other) const
        {
            return quaternion(X / other.X, Y / other.Y, Z / other.Z, W / other.W);
        }
        inline quaternion& operator /= (const quaternion &other)
        {
            X /= other.X; Y /= other.Y; Z /= other.Z; W /= other.W; return *this;
        }
        
        inline quaternion operator * (const quaternion &other) const
        {
            quaternion tmp;
            
            tmp.W = (other.W * W) - (other.X * X) - (other.Y * Y) - (other.Z * Z);
            tmp.X = (other.W * X) + (other.X * W) + (other.Y * Z) - (other.Z * Y);
            tmp.Y = (other.W * Y) + (other.Y * W) + (other.Z * X) - (other.X * Z);
            tmp.Z = (other.W * Z) + (other.Z * W) + (other.X * Y) - (other.Y * X);
            
            return tmp;
        }
        inline quaternion& operator *= (const quaternion &other)
        {
            *this = *this * other; return *this;
        }
        
        inline vector3df operator * (const vector3df &Vector) const
        {
            vector3df uv, uuv;
            vector3df qvec(X, Y, Z);
            
            uv = qvec.cross(Vector);
            uuv = qvec.cross(uv);
            uv *= (2.0 * W);
            uuv *= 2.0;
            
            return Vector + uv + uuv;
        }
        
        inline quaternion operator / (f32 Size) const
        {
            return quaternion(X / Size, Y / Size, Z / Size, W / Size);
        }
        inline quaternion& operator /= (f32 Size)
        {
            X /= Size; Y /= Size; Z /= Size; W /= Size; return *this;
        }
        
        inline quaternion operator * (f32 Size) const
        {
            return quaternion(X * Size, Y * Size, Z * Size, W * Size);
        }
        inline quaternion& operator *= (f32 Size)
        {
            X *= Size; Y *= Size; Z *= Size; W *= Size; return *this;
        }
        
        /* === Additional operators === */
        
        inline const f32 operator [] (u32 i) const
        {
            return i < 4 ? *(&X + i) : 0.0f;
        }
        
        inline f32& operator [] (u32 i)
        {
            return *(&X + i);
        }
        
        /* === Extra functions === */
        
        inline f32 dot(const quaternion &other) const // Scalar/ Dot product
        {
            return X*other.X + Y*other.Y + Z*other.Z + W*other.W;
        }
        
        inline quaternion& normalize()
        {
            f32 n = X*X + Y*Y + Z*Z + W*W;
            
            if (n == 1.0f || n == 0.0f)
                return *this;
            
            n = 1.0f / sqrtf(n);
            
            X *= n;
            Y *= n;
            Z *= n;
            W *= n;
            
            return *this;
        } 
        
        inline quaternion& setInverse()
        {
            X = -X; Y = -Y; Z = -Z; return *this;
        }
        inline quaternion getInverse() const
        {
            return quaternion(-X, -Y, -Z, W);
        }
        
        inline void set(f32 NewX, f32 NewY, f32 NewZ, f32 NewW)
        {
            X = NewX;
            Y = NewY;
            Z = NewZ;
            W = NewW;
        }
        
        inline void set(f32 NewX, f32 NewY, f32 NewZ)
        {
            f32 cp = cos(NewX/2);
            f32 cr = cos(NewZ/2);
            f32 cy = cos(NewY/2);
            
            f32 sp = sin(NewX/2);
            f32 sr = sin(NewZ/2);
            f32 sy = sin(NewY/2);
            
            f32 cpcy = cp * cy;
            f32 spsy = sp * sy;
            f32 cpsy = cp * sy;
            f32 spcy = sp * cy;
            
            X = sr * cpcy - cr * spsy;
            Y = cr * spcy + sr * cpsy;
            Z = cr * cpsy - sr * spcy;
            W = cr * cpcy + sr * spsy;
            
            normalize();
        }
        
        inline void set(const vector3df &Vector)
        {
            set(Vector.X, Vector.Y, Vector.Z);
        }
        inline void set(const vector4df &Vector)
        {
            set(Vector.X, Vector.Y, Vector.Z, Vector.W);
        }
        
        inline void getMatrix(matrix4f &Mat) const
        {
            Mat[ 0] = 1.0 - 2.0*Y*Y - 2.0*Z*Z;
            Mat[ 1] =       2.0*X*Y + 2.0*Z*W;
            Mat[ 2] =       2.0*X*Z - 2.0*Y*W;
            Mat[ 3] =       0.0;
            
            Mat[ 4] =       2.0*X*Y - 2.0*Z*W;
            Mat[ 5] = 1.0 - 2.0*X*X - 2.0*Z*Z;
            Mat[ 6] =       2.0*Z*Y + 2.0*X*W;
            Mat[ 7] =       0.0;
            
            Mat[ 8] =       2.0*X*Z + 2.0*Y*W;
            Mat[ 9] =       2.0*Z*Y - 2.0*X*W;
            Mat[10] = 1.0 - 2.0*X*X - 2.0*Y*Y;
            Mat[11] =       0.0;
            
            Mat[12] = 0.0;
            Mat[13] = 0.0;
            Mat[14] = 0.0;
            Mat[15] = 1.0;
        }
        
        inline matrix4f getMatrix() const
        {
            matrix4f Mat;
            getMatrix(Mat);
            return Mat;
        }
        
        inline void getMatrixTransposed(matrix4f &Mat) const
        {
            Mat[ 0] = 1.0 - 2.0*Y*Y - 2.0*Z*Z;
            Mat[ 4] =       2.0*X*Y + 2.0*Z*W;
            Mat[ 8] =       2.0*X*Z - 2.0*Y*W;
            Mat[12] =       0.0;
            
            Mat[ 1] =       2.0*X*Y - 2.0*Z*W;
            Mat[ 5] = 1.0 - 2.0*X*X - 2.0*Z*Z;
            Mat[ 9] =       2.0*Z*Y + 2.0*X*W;
            Mat[13] =       0.0;
            
            Mat[ 2] =       2.0*X*Z + 2.0*Y*W;
            Mat[ 6] =       2.0*Z*Y - 2.0*X*W;
            Mat[10] = 1.0 - 2.0*X*X - 2.0*Y*Y;
            Mat[14] =       0.0;
            
            Mat[ 3] = 0.0;
            Mat[ 7] = 0.0;
            Mat[11] = 0.0;
            Mat[15] = 1.0;
        }
        
        inline matrix4f getMatrixTransposed() const
        {
            matrix4f Mat;
            getMatrixTransposed(Mat);
            return Mat;
        }
        
        inline void setMatrix(const matrix4f &Mat)
        {
            f32 trace = Mat(0, 0) + Mat(1, 1) + Mat(2, 2) + 1.0f;
            
            if (trace > 0.0f)
            {
                f32 s = 2.0f * sqrtf(trace);
                X = (Mat(2, 1) - Mat(1, 2)) / s;
                Y = (Mat(0, 2) - Mat(2, 0)) / s;
                Z = (Mat(1, 0) - Mat(0, 1)) / s;
                W = 0.25f * s;
            }
            else
            {
                if (Mat(0, 0) > Mat(1, 1) && Mat(0, 0) > Mat(2, 2))
                {
                    f32 s = 2.0f * sqrtf(1.0f + Mat(0, 0) - Mat(1, 1) - Mat(2, 2));
                    X = 0.25f * s;
                    Y = (Mat(0, 1) + Mat(1, 0) ) / s;
                    Z = (Mat(2, 0) + Mat(0, 2) ) / s;
                    W = (Mat(2, 1) - Mat(1, 2) ) / s;
                }
                else if (Mat(1, 1) > Mat(2, 2))
                {
                    f32 s = 2.0f * sqrtf(1.0f + Mat(1, 1) - Mat(0, 0) - Mat(2, 2));
                    X = (Mat(0, 1) + Mat(1, 0) ) / s;
                    Y = 0.25f * s;
                    Z = (Mat(1, 2) + Mat(2, 1) ) / s;
                    W = (Mat(0, 2) - Mat(2, 0) ) / s;
                }
                else
                {
                    f32 s = 2.0f * sqrtf(1.0f + Mat(2, 2) - Mat(0, 0) - Mat(1, 1));
                    X = (Mat(0, 2) + Mat(2, 0) ) / s;
                    Y = (Mat(1, 2) + Mat(2, 1) ) / s;
                    Z = 0.25f * s;
                    W = (Mat(1, 0) - Mat(0, 1) ) / s;
                }
            }
            
            normalize();
        }
        
        inline quaternion& setAngleAxis(f32 Angle, const vector3df &Axis)
        {
            const f32 HalfAngle = Angle * 0.5f;
            const f32 Sine      = sinf(HalfAngle);
            
            X = Sine * Axis.X;
            Y = Sine * Axis.Y;
            Z = Sine * Axis.Z;
            W = cosf(HalfAngle);
            
            return *this;
        }
        
        inline void getAngleAxis(f32 &Angle, vector3df &Axis)
        {
            const f32 Scale = sqrtf(X*X + Y*Y + Z*Z);
            
            if ( ( Scale > -1.0e-6 && Scale < 1.0e-6 ) || W > 1.0f || W < -1.0f )
            {
                Axis.X  = 0.0f;
                Axis.Y  = 1.0f;
                Axis.Z  = 0.0f;
                Angle   = 0.0f;
            }
            else
            {
                const f32 InvScale = 1.0f / Scale;
                Axis.X  = X * InvScale;
                Axis.Y  = Y * InvScale;
                Axis.Z  = Z * InvScale;
                Angle   = 2.0f * acosf(W);
            }
        }
        
        inline void getEuler(vector3df &Euler)
        {
            const f64 sqX = X*X;
            const f64 sqY = Y*Y;
            const f64 sqZ = Z*Z;
            const f64 sqW = W*W;
            
            f32 tmp = -2.0f * (X*Z - Y*W);
            
            if (tmp < -1.0f)
                tmp = -1.0f;
            else if (tmp > 1.0f)
                tmp = 1.0f;
            
            Euler.X = (f32)(atan2(2.0 * (Y*Z + X*W), -sqX - sqY + sqZ + sqW));
            Euler.Y = asinf(tmp);
            Euler.Z = (f32)(atan2(2.0 * (X*Y + Z*W), sqX - sqY - sqZ + sqW));
        }
        
        /*
         * Slerp: "spherical linear interpolation"
         * Smoothly (spherically, shortest path on a quaternion sphere) 
         * interpolates between two UNIT quaternion positions
         * slerp(p, q, t) = ( p*sin((1 - t)*omega) + q*sin(t*omega) ) / sin(omega)
         */
        inline void slerp(const quaternion &to, f32 t)
        {
            /* Temporary variables */
            f32 to1[4];
            f32 omega, cosom, sinom;
            f32 scale0, scale1;
            
            /* Calculate cosine */
            cosom = X*to.X + Y*to.Y + Z*to.Z + W*to.W;
            
            /* Adjust signs (if necessary) */
            if (cosom < 0.0)
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
            if ( ( 1.0 - cosom ) > 1e-10 )
            {
                /* Standard case (slerp) */
                omega = acos(cosom);
                sinom = sin(omega);
                scale0 = (f32)sin((1.0 - t) * omega) / sinom;
                scale1 = (f32)sin(t * omega) / sinom;
            }
            else
            {
                /*
                 * "from" and "to" quaternions are very close
                 *  ... so we can do a linear interpolation
                 */
                scale0 = 1.0f - t;
                scale1 = t;
            }
            
            /* Calculate final values */
            X = scale0*X + scale1*to1[0];
            Y = scale0*Y + scale1*to1[1];
            Z = scale0*Z + scale1*to1[2];
            W = scale0*W + scale1*to1[3];
        }
        
        inline void slerp(const quaternion &from, const quaternion &to, f32 t)
        {
            /* Temporary variables */
            f32 to1[4];
            f32 omega, cosom, sinom;
            f32 scale0, scale1;
            
            /* Calculate cosine */
            cosom = from.X*to.X + from.Y*to.Y + from.Z*to.Z + from.W*to.W;
            
            /* Adjust signs (if necessary) */
            if (cosom < 0.0)
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
            if ((1.0 - cosom) > 1e-10) 
            {
                /* Standard case (slerp) */
                omega = acos(cosom);
                sinom = sin(omega);
                scale0 = (f32)sin((1.0 - t) * omega) / sinom;
                scale1 = (f32)sin(t * omega) / sinom;
            }
            else
            {        
                /*
                 * "from" and "to" quaternions are very close 
                 *  ... so we can do a linear interpolation
                 */
                scale0 = 1.0f - t;
                scale1 = t;
            }

            /* Calculate final values */
            X = scale0*from.X + scale1*to1[0];
            Y = scale0*from.Y + scale1*to1[1];
            Z = scale0*from.Z + scale1*to1[2];
            W = scale0*from.W + scale1*to1[3];
        }
        
        inline void reset() // Load identity
        {
            X = Y = Z = 0;
            W = 1;
        }
        
        /* Members */
        
        f32 X, Y, Z, W;
        
};


} // /namespace dim

} // /namespace sp


#endif



// ================================================================================
