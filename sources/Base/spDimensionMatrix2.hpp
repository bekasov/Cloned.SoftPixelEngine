/*
 * 2x2 Matrix header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_DIMENSION_MATRIX2_H__
#define __SP_DIMENSION_MATRIX2_H__


#include "Base/spStandard.hpp"
#include "Base/spMath.hpp"

#include <string.h>


namespace sp
{
namespace dim
{


template <typename T> class matrix2
{
    
    public:
        
        /* === Constructors === */
        
        matrix2()
        {
            reset();
        }
        matrix2(const T (&other)[4])
        {
            *this = other;
        }
        matrix2(const matrix2<T> &other)
        {
            *this = other;
        }
        matrix2(
            T m1n1, T m2n1,
            T m1n2, T m2n2)
        {
            M[0] = m1n1; M[2] = m2n1;
            M[1] = m1n2; M[3] = m2n2;
        }
        ~matrix2()
        {
        }
        
        /* === Operators === */
        
        inline const T operator () (u32 row, u32 col) const
        {
            return row < 2 && col < 2 ? M[(row << 1) + col] : (T)0;
        }
        inline T& operator () (u32 row, u32 col)
        {
            return M[(row << 1) + col];
        }
        
        inline const T operator [] (u32 i) const
        {
            return i < 4 ? M[i] : (T)0;
        }
        inline T& operator [] (u32 i)
        {
            return M[i];
        }
        
        inline bool operator == (const matrix2<T> &other)
        {
            for (s32 i = 0; i < 4; ++i)
            {
                if (M[i] != other.M[i])
                    return false;
            }
            return true;
        }
        inline bool operator != (const matrix2<T> &other)
        {
            for (s32 i = 0; i < 4; ++i)
            {
                if (M[i] != other.M[i])
                    return true;
            }
            return false;
        }
        
        inline matrix2<T>& operator = (const T (&other)[4])
        {
            M[0] = other[0]; M[2] = other[2];
            M[1] = other[1]; M[3] = other[3];
            return *this;
        }
        inline matrix2<T>& operator = (const matrix2<T> &other)
        {
            M[0] = other[0]; M[2] = other[2];
            M[1] = other[1]; M[3] = other[3];
            return *this;
        }
        inline matrix2<T>& operator = (T Scalar)
        {
            memset(M, Scalar, sizeof(M));
            return *this;
        }
        
        inline matrix2<T> operator + (const matrix2<T> &mltMatrix) const
        {
            matrix2<T> other;
            
            other[0] = M[0] + mltMatrix[0]; other[2] = M[2] + mltMatrix[2];
            other[1] = M[1] + mltMatrix[1]; other[3] = M[3] + mltMatrix[3];
            
            return other;
        }
        
        inline matrix2<T>& operator += (const matrix2<T> &mltMatrix)
        {
            M[0] += mltMatrix[0]; M[2] += mltMatrix[2];
            M[1] += mltMatrix[1]; M[3] += mltMatrix[3];
            return *this;
        }
        
        inline matrix2<T> operator - (const matrix2<T> &mltMatrix) const
        {
            matrix2<T> other;
            other[0] = M[0] - mltMatrix[0]; other[2] = M[2] - mltMatrix[2];
            other[1] = M[1] - mltMatrix[1]; other[3] = M[3] - mltMatrix[3];
            return other;
        }
        inline matrix2<T>& operator -= (const matrix2<T> &mltMatrix)
        {
            M[0] -= mltMatrix[0]; M[2] -= mltMatrix[2];
            M[1] -= mltMatrix[1]; M[3] -= mltMatrix[3];
            return *this;
        }
        
        inline matrix2<T> operator * (const matrix2<T> &mltMatrix) const
        {
            matrix2<T> m3;
            const T* m1 = M;
            const T* m2 = mltMatrix.M;
            
            m3[0] = m1[0]*m2[0] + m1[2]*m2[1];
            m3[1] = m1[1]*m2[0] + m1[3]*m2[1];
            
            m3[2] = m1[0]*m2[2] + m1[2]*m2[3];
            m3[3] = m1[1]*m2[2] + m1[3]*m2[3];
            
            return m3;
        }
        
        inline matrix2<T> operator * (T Scalar) const
        {
            matrix2<T> other;
            
            other[0] = M[0]*Scalar; other[2] = M[2]*Scalar;
            other[1] = M[1]*Scalar; other[3] = M[3]*Scalar;
            
            return other;
        }
        
        inline matrix2<T>& operator *= (const matrix2<T> &mltMatrix)
        {
            T m1[4];
            memcpy(m1, M, sizeof(M));
            const T* m2 = mltMatrix.M;
            
            M[0] = m1[0]*m2[0] + m1[2]*m2[1];
            M[1] = m1[1]*m2[0] + m1[3]*m2[1];
            
            M[2] = m1[0]*m2[2] + m1[2]*m2[3];
            M[3] = m1[1]*m2[2] + m1[3]*m2[3];
            
            return *this;
        }
        
        inline matrix2<T>& operator *= (T Scalar)
        {
            M[0] *= Scalar; M[2] *= Scalar;
            M[1] *= Scalar; M[3] *= Scalar;
            return *this;
        }
        
        inline point2d<T> operator * (const point2d<T> &Vector) const
        {
            return point2d<T>(
                Vector.X*M[0] + Vector.Y*M[2],
                Vector.X*M[1] + Vector.Y*M[3]
            );
        }
        
        inline void clear()
        {
            memset(M, 0, sizeof(M));
        }
        
        /*
        ( 1  0 )
        ( 0  1 )
        */
        inline matrix2<T>& reset() // Loads identity
        {
            M[0] = 1; M[2] = 0;
            M[1] = 0; M[3] = 1;
            return *this;
        }
        
        inline void multiplySingleMatrix(const T (&other)[2]) // Multiplies this matrix with a ( 1 x 2 ) matrix
        {
            T Result[2];
            
            Result[0] = M[0]*other[0] + M[2]*other[1];
            Result[1] = M[1]*other[0] + M[3]*other[1];
            
            other[0] = Result[0]; other[1] = Result[1];
        }
        
        inline bool getInverse(matrix2<T> &InverseMat) const
        {
            f32 d = M[0]*M[3] - M[2]*M[1];
            
            if (fabs(d) < math::ROUNDING_ERROR)
                return false;
            
            d = 1.0f / d;
            
            InverseMat[0] = d * (  M[3] );
            InverseMat[1] = d * ( -M[1] );
            InverseMat[2] = d * ( -M[2] );
            InverseMat[3] = d * (  M[0] );
            
            return true;
        }
        
        inline bool setInverse()
        {
            matrix2<T> Matrix;
            
            if (getInverse(Matrix))
            {
                *this = Matrix;
                return true;
            }
            
            return false;
        }
        
        inline matrix2<T> getInverse() const
        {
            matrix2<T> Mat;
            getInverse(Mat);
            return Mat;
        }
        
        /*
        ( x  0 )
        ( 0  y )
        */
        inline matrix2<T>& scale(const point2d<T> &Vector)
        {
            matrix2<T> other;
            
            other[0] = Vector.X;
            other[3] = Vector.Y;
            
            return *this *= other;
        }
        
        /*
        (  cos  sin )
        ( -sin  cos )
        */
        inline matrix2<T>& rotate(const T Angle)
        {
            matrix2<T> other;
            other.setRotation(Angle);
            return *this *= other;
        }
        
        inline void setRotation(T Rotation, bool UseDegrees = true)
        {
            if (UseDegrees)
                Rotation = Rotation * static_cast<T>(M_PI) / T(180);
            
            const T s = sin(Rotation);
            const T c = cos(Rotation);
            
            M[0] =  c; M[2] = s;
            M[1] = -s; M[3] = c;
        }
        
        /* === Row & columns === */
        
        inline point2d<T> getRow(s32 Position) const
        {
            switch (Position) {
                case 0:
                    return point2d<T>(M[0], M[2]);
                case 1:
                    return point2d<T>(M[1], M[3]);
            }
            return point2d<T>();
        }
        
        inline point2d<T> getColumn(s32 Position) const
        {
            switch (Position) {
                case 0:
                    return point2d<T>(M[0], M[1]);
                case 1:
                    return point2d<T>(M[2], M[3]);
            }
            return point2d<T>();
        }
        
        inline void setScale(const point2d<T> &Scale)
        {
            M[0] = Scale.X; M[2] = Scale.Y;
        }
        inline point2d<T> getScale() const
        {
            if (math::Equal(M[1], 0.0f) && math::Equal(M[3], 0.0f))
                return point2d<T>(M[0], M[3]);
            
            return point2d<T>(
                sqrtf(M[0]*M[0] + M[1]*M[1]),
                sqrtf(M[2]*M[2] + M[3]*M[3])
            );
        }
        
        inline T getRotation() const
        {
            return 0.0f; // !TODO!
        }
        
        inline matrix4<T> getTransposed() const
        {
            matrix4<T> Mat;
            getTransposed(Mat);
            return Mat;
        }
        
        inline void getTransposed(matrix4<T> &other) const
        {
            other[0] = M[0]; other[2] = M[1];
            other[1] = M[2]; other[3] = M[3];
        }
        
        inline point2d<T> interpolate(const point2d<T> &other, f32 seek) const
        {
            point2d<T> Mat;
            
            for (s32 i = 0; i < 4; ++i)
                Mat.M[i] = M[i] + (other.M[i] - M[i]) * seek;
            
            return Mat;
        }
        
        inline bool isIdentity() const
        {
            return
                math::Equal(M[0], 1.0f) && math::Equal(M[1], 0.0f) &&
                math::Equal(M[2], 1.0f) && math::Equal(M[3], 0.0f);
        }
        
        inline const T* getArray() const
        {
            return M;
        }
        inline T* getArray()
        {
            return M;
        }
        
        template <typename B> inline matrix2<B> cast() const
        {
            B other[4];
            
            for (s32 i = 0; i < 4; ++i)
                other[i] = static_cast<B>(M[i]);
            
            return matrix2<B>(other);
        }
        
    private:
        
        /* === Member === */
        
        /*
         * The matrix memory
         * (all 2x2 matrices allocate 4 elements)
         */
        T M[4];
        
};

typedef matrix2<s32> matrix2i;
typedef matrix2<f32> matrix2f;
typedef matrix2<f64> matrix2d;


} // /namespace dim

} // /namespace sp


#endif



// ================================================================================
