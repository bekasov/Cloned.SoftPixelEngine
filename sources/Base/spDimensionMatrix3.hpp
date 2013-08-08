/*
 * 3x3 Matrix header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_DIMENSION_MATRIX3_H__
#define __SP_DIMENSION_MATRIX3_H__


#include "Base/spStandard.hpp"
#include "Base/spMath.hpp"

#include <string.h>


namespace sp
{
namespace dim
{


/**
Matrix 3x3 class.
\ingroup group_data_types
*/
template <typename T> class matrix3
{
    
    public:
        
        static const u32 NUM = 3;

        matrix3()
        {
            reset();
        }
        matrix3(const T (&Other)[9])
        {
            *this = Other;
        }
        matrix3(const matrix3<T> &Other)
        {
            *this = Other;
        }
        matrix3(
            T m1n1, T m2n1, T m3n1,
            T m1n2, T m2n2, T m3n2,
            T m1n3, T m2n3, T m3n3)
        {
            M[0] = m1n1; M[3] = m2n1; M[6] = m3n1;
            M[1] = m1n2; M[4] = m2n2; M[7] = m3n2;
            M[2] = m1n3; M[5] = m2n3; M[8] = m3n3;
        }
        ~matrix3()
        {
        }
        
        /* === Operators === */
        
        inline const T& operator () (u32 row, u32 col) const
        {
            return M[ (row * 3) + col ];
        }
        inline T& operator () (u32 row, u32 col)
        {
            return M[ (row * 3) + col ];
        }
        
        inline const T& operator [] (u32 i) const
        {
            return M[i];
        }
        inline T& operator [] (u32 i)
        {
            return M[i];
        }
        
        inline bool operator == (const matrix3<T> &Other)
        {
            return dim::compareMatEqual(*this, Other);
        }
        inline bool operator != (const matrix3<T> &Other)
        {
            return dim::compareMatNotEqual(*this, Other);
        }
        
        inline matrix3<T>& operator = (const matrix3<T> &Other)
        {
            std::copy(Other.M, Other.M + 9, M);
            return *this;
        }
        
        inline matrix3<T> operator + (const matrix3<T> &Other) const
        {
            matrix3<T> Result;
            dim::matrixAdd(Result, M, Other);
            return Result;
        }
        inline matrix3<T>& operator += (const matrix3<T> &Other)
        {
            dim::matrixAdd(M, M, Other);
            return *this;
        }
        
        inline matrix3<T> operator - (const matrix3<T> &Other) const
        {
            matrix3<T> Result;
            dim::matrixSub(Result, M, Other);
            return Result;
        }
        inline matrix3<T>& operator -= (const matrix3<T> &Other)
        {
            dim::matrixSub(M, M, Other);
            return *this;
        }
        
        inline matrix3<T> operator * (const matrix3<T> &Other) const
        {
            matrix4<T> Result;
            dim::matrixMul<NUM, T>(Result.M, M, Other.M);
            return Result;
        }
        inline matrix3<T> operator * (const T &Scalar) const
        {
            matrix3<T> Result;
            dim::matrixMul(Result, *this, Scalar);
            return Result;
        }
        
        inline matrix3<T>& operator *= (const matrix3<T> &Other)
        {
            T Prev[9];
            std::copy(M, M + 9, Prev);
            
            dim::matrixMul<NUM, T>(M, Prev, Other.M);
            
            return *this;
        }
        inline matrix3<T>& operator *= (T Scalar)
        {
            dim::matrixMul(*this, *this, Scalar);
            return *this;
        }
        
        /*
        (  1-1  2-1  3-1  )     ( x )
        |  1-2  2-2  3-2  |  *  | y |
        (  1-3  2-3  3-3  )     ( z )
        */
        inline vector3d<T> operator * (const vector3d<T> &Vector) const
        {
            return vector3d<T>(
                Vector.X*M[0] + Vector.Y*M[3] + Vector.Z*M[6],
                Vector.X*M[1] + Vector.Y*M[4] + Vector.Z*M[7],
                Vector.X*M[2] + Vector.Y*M[5] + Vector.Z*M[8]
            );
        }
        
        inline vector2d<T> operator * (const vector2d<T> &Vector) const
        {
            return vector2d<T>(
                Vector.X*M[0] + Vector.Y*M[3],
                Vector.X*M[1] + Vector.Y*M[4]
            );
        }
        
        inline triangle3d<T> operator * (const triangle3d<T> &Triangle) const
        {
            return triangle3d<T>(
                *this * Triangle.PointA,
                *this * Triangle.PointB,
                *this * Triangle.PointC
            );
        }
        
        inline triangle3d<T> operator * (const triangle3d<T, vector3d<T>*> &Triangle) const
        {
            return triangle3d<T>(
                *this * (*Triangle.PointA),
                *this * (*Triangle.PointB),
                *this * (*Triangle.PointC)
            );
        }
        
        plane3d<T> operator * (const plane3d<T> &Plane) const
        {
            plane3d<T> NewPlane(Plane);
            
            vector3d<T> Member( *this * ( - NewPlane.Normal * NewPlane.Distance ) );
            
            vector3d<T> Origin;
            NewPlane.Normal = *this * NewPlane.Normal;
            Origin = *this * Origin;
            
            NewPlane.Normal -= Origin;
            NewPlane.Distance = - Member.dot(NewPlane.Normal);
            
            return NewPlane;
        }
        
        /* === Transformation functions === */
        
        inline vector3d<T> vecRotate(const vector3d<T> &Vector) const
        {
            return vector3d<T>(
                Vector.X*M[0] + Vector.Y*M[3] + Vector.Z*M[6],
                Vector.X*M[1] + Vector.Y*M[4] + Vector.Z*M[7],
                Vector.X*M[2] + Vector.Y*M[5] + Vector.Z*M[8]
            );
        }
        
        inline vector3d<T> vecRotateInverse(const vector3d<T> &Vector) const
        {
            return vector3d<T>(
                Vector.X*M[0] + Vector.Y*M[1] + Vector.Z*M[2],
                Vector.X*M[3] + Vector.Y*M[4] + Vector.Z*M[5],
                Vector.X*M[6] + Vector.Y*M[7] + Vector.Z*M[8]
            );
        }
        
        inline void clear()
        {
            memset(M, 0, sizeof(M));
        }
        
        /*
        (  1  0  0  )
        |  0  1  0  |
        (  0  0  1  )
        */
        inline matrix3<T>& reset() // Loads identity
        {
            dim::loadIdentity(*this);
            return *this;
        }
        
        void matrixLookAt(const vector3d<T> &Position, const vector3d<T> &LookAt, const vector3d<T> &upVector)
        {
            vector3d<T> ZAxis = LookAt - Position;
            ZAxis.normalize();
            
            vector3d<T> XAxis = upVector.cross(ZAxis);
            XAxis.normalize();
            
            vector3d<T> YAxis = ZAxis.cross(XAxis);
            
            M[0] = XAxis.X; M[3] = XAxis.Y; M[6] = XAxis.Z;
            M[1] = YAxis.X; M[4] = YAxis.Y; M[7] = YAxis.Z;
            M[2] = ZAxis.X; M[5] = ZAxis.Y; M[8] = ZAxis.Z;
        }
        
        //! Returns the determinant of this matrix.
        T determinant() const
        {
            const matrix3<T> &m = *this;
            
            return
                ( m(0, 0) * m(1, 1) * m(2, 2) ) + ( m(0, 1) * m(1, 2) * m(2, 0) ) + ( m(0, 2) * m(1, 0) * m(2, 1) ) -
                ( m(2, 0) * m(1, 1) * m(0, 2) ) - ( m(2, 0) * m(1, 2) * m(0, 0) ) - ( m(2, 2) * m(1, 0) * m(0, 1) );
        }

        bool getInverse(matrix3<T> &InverseMat) const
        {
            const matrix3<T> &m = *this;
            
            T d = determinant();
            
            if (d == T(0))
                return false;
            
            d = T(1) / d;
            
            InverseMat(0, 0) = d * ( m(1, 1) * m(2, 2) - m(1, 2) * m(2, 1) );
            InverseMat(0, 1) = d * ( m(0, 2) * m(2, 1) - m(0, 1) * m(2, 2) );
            InverseMat(0, 2) = d * ( m(0, 1) * m(1, 2) - m(0, 2) * m(1, 1) );
            InverseMat(1, 0) = d * ( m(1, 2) * m(2, 0) - m(1, 0) * m(2, 2) );
            InverseMat(1, 1) = d * ( m(0, 0) * m(2, 2) - m(0, 2) * m(2, 0) );
            InverseMat(1, 2) = d * ( m(0, 0) * m(1, 1) - m(0, 1) * m(1, 0) );
            InverseMat(2, 0) = d * ( m(1, 0) * m(2, 1) - m(1, 1) * m(2, 0) );
            InverseMat(2, 1) = d * ( m(0, 1) * m(2, 0) - m(0, 0) * m(2, 1) );
            InverseMat(2, 2) = d * ( m(0, 0) * m(1, 1) - m(0, 1) * m(1, 0) );
            
            return true;
        }
        
        inline bool setInverse()
        {
            matrix3<T> Matrix;
            
            if (getInverse(Matrix))
            {
                *this = Matrix;
                return true;
            }
            
            return false;
        }
        
        inline matrix3<T> getInverse() const
        {
            matrix3<T> Mat;
            getInverse(Mat);
            return Mat;
        }
        
        /*
        (  x  0  0  )
        |  0  y  0  |
        (  0  0  z  )
        */
        inline matrix3<T>& scale(const vector3d<T> &Vector)
        {
            matrix3<T> other;
            
            other[0] = Vector.X;
            other[4] = Vector.Y;
            other[8] = Vector.Z;
            
            return *this *= other;
        }
        
        /*
        ( xx(1-c)+c   xy(1-c)-zs  xz(1-c)+ys )
        | yx(1-c)+zs  yy(1-c)+c   yz(1-c)-xs |
        ( xz(1-c)-ys  yz(1-c)+xs  zz(1-c)+c  )
        */
        matrix3<T>& rotate(const T &Angle, vector3d<T> Rotation)
        {
            matrix3<T> other;
            
            /* Normalize the vector */
            Rotation.normalize();
            
            /* Parameters normalization */
            T x   = Rotation.X;
            T y   = Rotation.Y;
            T z   = Rotation.Z;
            T c   = math::Cos(Angle);
            T s   = math::Cos(Angle);
            T cc  = T(1) - c;
            
            /* Rotation */
            other[0] = x*x*cc+c;   other[3] = x*y*cc-z*s; other[6] = x*z*cc+y*s;
            other[1] = y*x*cc+z*s; other[4] = y*y*cc+c;   other[7] = y*z*cc-x*s;
            other[2] = x*z*cc-y*s; other[5] = y*z*cc+x*s; other[8] = z*z*cc+c;
            
            return *this *= other;
        }
        
        matrix3<T>& rotateX(const T &Angle)
        {
            matrix3<T> other;
            T c = math::Cos(Angle);
            T s = math::Sin(Angle);
            
            /* Rotation */
            other[0] = 1; other[4] = 0; other[ 8] =  0;
            other[1] = 0; other[5] = c; other[ 9] = -s;
            other[2] = 0; other[6] = s; other[10] =  c;
            
            return *this *= other;
        }
        
        matrix3<T>& rotateY(const T &Angle)
        {
            matrix3<T> other;
            T c = math::Cos(Angle);
            T s = math::Sin(Angle);
            
            /* Rotation */
            other[0] =  c; other[3] = 0; other[6] = s;
            other[1] =  0; other[4] = 1; other[7] = 0;
            other[2] = -s; other[5] = 0; other[8] = c;
            
            return *this *= other;
        }
        
        matrix3<T>& rotateZ(const T &Angle)
        {
            matrix3<T> other;
            T c = math::Cos(Angle);
            T s = math::Sin(Angle);
            
            /* Rotation */
            other[0] = c; other[3] = -s; other[6] = 0;
            other[1] = s; other[4] =  c; other[7] = 0;
            other[2] = 0; other[5] =  0; other[8] = 1;
            
            return *this *= other;
        }
        
        inline void rotateYXZ(const vector3df &Rotation)
        {
            rotateY(Rotation.Y);
            rotateX(Rotation.X);
            rotateZ(Rotation.Z);
        }
        
        inline void rotateZXY(const vector3df &Rotation)
        {
            rotateZ(Rotation.Z);
            rotateX(Rotation.X);
            rotateY(Rotation.Y);
        }
        
        void setRotation(vector3df Rotation, bool UseDegrees = true)
        {
            if (UseDegrees)
                Rotation = Rotation * M_PI / 180.0;
            
            f64 cx = cos(Rotation.X);
            f64 sx = sin(Rotation.X);
            f64 cy = cos(Rotation.Y);
            f64 sy = sin(Rotation.Y);
            f64 cz = cos(Rotation.Z);
            f64 sz = sin(Rotation.Z);
            
            f64 sxy = sx*sy;
            f64 cxy = cx*sy;
            
            M[0] = (T)(cy*cz);
            M[1] = (T)(cy*sz);
            M[2] = (T)(-sy);
            
            M[3] = (T)(sxy*cz - cx*sz);
            M[4] = (T)(sxy*sz + cx*cz);
            M[5] = (T)(sx*cy);
            
            M[6] = (T)(cxy*cz + sx*sz);
            M[7] = (T)(cxy*sz - sx*cz);
            M[8] = (T)(cx*cy);
        }
        
        void setInverseRotation(vector3df Rotation, bool UseDegrees = true)
        {
            if (UseDegrees)
                Rotation = Rotation * M_PI / 180.0;
            
            f64 cx = cos(Rotation.X);
            f64 sx = sin(Rotation.X);
            f64 cy = cos(Rotation.Y);
            f64 sy = sin(Rotation.Y);
            f64 cz = cos(Rotation.Z);
            f64 sz = sin(Rotation.Z);
            
            f64 sxy = sx*sy;
            f64 cxy = cx*sy;
            
            M[0] = (T)(cy*cz);
            M[3] = (T)(cy*sz);
            M[6] = (T)(-sy);
            
            M[1] = (T)(sxy*cz - cx*sz);
            M[4] = (T)(sxy*sz + cx*cz);
            M[7] = (T)(sx*cy);
            
            M[2] = (T)(cxy*cz + sx*sz);
            M[5] = (T)(cxy*sz - sx*cz);
            M[8] = (T)(cx*cy);
        }
        
        void setTextureRotation(const T &Degree)
        {
            T c = math::Cos(Degree);
            T s = math::Sin(Degree);
            
            M[0] = c;
            M[1] = s;
            M[2] = T( -0.5 * ( c + s) + 0.5 );
            
            M[3] = -s;
            M[4] = c;
            M[5] = T( -0.5 * (-s + c) + 0.5 );
        }
        
        /* === Row & columns === */
        
        vector3d<T> getRow(u32 Position) const
        {
            switch (Position)
            {
                case 0: return vector3d<T>(M[0], M[3], M[6]);
                case 1: return vector3d<T>(M[1], M[4], M[7]);
                case 2: return vector3d<T>(M[2], M[5], M[8]);
            }
            return vector3d<T>();
        }
        
        void setRow(u32 Position, const vector3d<T> &Vec)
        {
            switch (Position)
            {
                case 0: M[0] = Vec.X, M[3] = Vec.Y, M[6] = Vec.Z; break;
                case 1: M[1] = Vec.X, M[4] = Vec.Y, M[7] = Vec.Z; break;
                case 2: M[2] = Vec.X, M[5] = Vec.Y, M[8] = Vec.Z; break;
            }
        }
        
        inline const vector3d<T>& getColumn(u32 Position) const
        {
            return *reinterpret_cast<const vector3d<T>*>(&M[Position * 3]);
        }
        inline vector3d<T>& getColumn(u32 Position)
        {
            return *reinterpret_cast<vector3d<T>*>(&M[Position * 3]);
        }
        inline void setColumn(u32 Position, const vector3d<T> &Vec)
        {
            getColumn(Position) = Vec;
        }
        
        inline void setScale(const vector3d<T> &Scale)
        {
            M[0] = Scale.X; M[4] = Scale.Y; M[8] = Scale.Z;
        }
        inline vector3d<T> getScale() const
        {
            return vector3d<T>(M[0], M[5], M[10]);
        }
        
        vector3d<T> getRotation() const
        {
            const matrix3<T> &Mat = *this;
            
            T Y = -(T)asin(Mat(0, 2));
            T C = (T)cos(Y);
            T rotx, roty, X, Z;
            
            Y *= (T)math::RAD64;
            
            if (fabs(C) > math::ROUNDING_ERROR)
            {
                C       = (T)(1.0 / C);
                rotx    = Mat(2, 2) * C;
                roty    = Mat(1, 2) * C;
                X       = (T)(atan2(roty, rotx) * math::RAD64);
                rotx    = Mat(0, 0) * C;
                roty    = Mat(0, 1) * C;
                Z       = (T)(atan2(roty, rotx) * math::RAD64);
            }
            else
            {
                X       = 0;
                rotx    =  Mat(1, 1);
                roty    = -Mat(1, 0);
                Z       = atan2(roty, rotx) * math::RAD64;
            }
            
            if (X < 0) X += 360;
            if (Y < 0) Y += 360;
            if (Z < 0) Z += 360;
            
            return vector3d<T>(X, Y, Z);
        }
        
        inline matrix3<T> getTransposed() const
        {
            matrix3<T> Mat;
            getTransposed(Mat);
            return Mat;
        }
        
        inline void getTransposed(matrix3<T> &other) const
        {
            dim::transpose(Other, *this);
        }
        
        inline matrix3<T>& setTransposed()
        {
            dim::transpose(*this);
            return *this;
        }
        
        inline T trace() const
        {
            return dim::trace(*this);
        }
        
        matrix3<T> interpolate(const matrix3<T> &Other, const T &t) const
        {
            matrix3<T> Mat;
            
            for (u32 i = 0; i < 9; ++i)
                math::lerp(Mat.M[i], M[i], Other.M[i], t);
            
            return Mat;
        }
        
        //! Normalizes all 3 column vectors.
        matrix3<T>& normalize()
        {
            ((vector3d<T>*)&M[0])->normalize();
            ((vector3d<T>*)&M[3])->normalize();
            ((vector3d<T>*)&M[6])->normalize();
            return *this;
        }
        
        inline bool isIdentity() const
        {
            return dim::hasIdentity(*this);
        }
        
        inline const T* getArray() const
        {
            return M;
        }
        
        inline T* getArray()
        {
            return M;
        }
        
        template <typename B> inline matrix3<B> cast() const
        {
            matrix3<B> Result;
            
            for (u32 i = 0; i < 9; ++i)
                Result.M[i] = static_cast<B>(M[i]);
            
            return Result;
        }
        
    private:
        
        /* === Members === */
        
        //! The matrix memory buffer.
        T M[9];
        
};

typedef matrix3<f32> matrix3f;
typedef matrix3<f64> matrix3d;


} // /namespace dim

} // /namespace sp


#endif



// ================================================================================
