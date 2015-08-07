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
#include "Base/spMatrixArithmetic.hpp"
#include "Base/spDimensionMatrix3.hpp"

#include <string.h>
#include <cstdlib>


namespace sp
{
namespace dim
{


/**
Matrix 2x2 class.
\ingroup group_data_types
*/
template <typename T> class matrix2
{
    
    public:
        
        static const size_t NUM = 2;

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
        
        inline const T& operator () (size_t row, size_t col) const
        {
            return M[(row << 1) + col];
        }
        inline T& operator () (size_t row, size_t col)
        {
            return M[(row << 1) + col];
        }
        
        inline const T& operator [] (size_t i) const
        {
            return M[i];
        }
        inline T& operator [] (size_t i)
        {
            return M[i];
        }
        
        inline bool operator == (const matrix2<T> &Other)
        {
            return sp::dim::compareMatEqual(*this, Other);
        }
        inline bool operator != (const matrix2<T> &Other)
        {
            return sp::dim::compareMatNotEqual(*this, Other);
        }
        
        inline matrix2<T>& operator = (const matrix2<T> &Other)
        {
            std::copy(Other.M, Other.M + 4, M);
            return *this;
        }
        
        inline matrix2<T> operator + (const matrix2<T> &Other) const
        {
            matrix2<T> Result;
            dim::matrixAdd(Result, M, Other);
            return Result;
        }
        inline matrix2<T>& operator += (const matrix2<T> &Other)
        {
            dim::matrixAdd(M, M, Other);
            return *this;
        }
        
        inline matrix2<T> operator - (const matrix2<T> &Other) const
        {
            matrix2<T> Result;
            dim::matrixSub(Result, M, Other);
            return Result;
        }
        inline matrix2<T>& operator -= (const matrix2<T> &Other)
        {
            dim::matrixSub(M, M, Other);
            return *this;
        }
        
        inline matrix2<T> operator * (const matrix2<T> &Other) const
        {
            matrix4<T> Result;
            dim::matrixMul<NUM, T>(Result.M, M, Other.M);
            return Result;
        }
        inline matrix2<T> operator * (const T &Scalar) const
        {
            sp::dim::matrix3<T> Result;
            dim::matrixMul(Result, *this, Scalar);
            return Result;
        }
        
        inline matrix2<T>& operator *= (const matrix2<T> &Other)
        {
            T Prev[4];
            std::copy(M, M + 4, Prev);
            
            dim::matrixMul<NUM, T>(M, Prev, Other.M);
            
            return *this;
        }
        inline matrix2<T>& operator *= (const T &Scalar)
        {
            dim::matrixMul(*this, *this, Scalar);
            return *this;
        }
        
        inline vector2d<T> operator * (const vector2d<T> &Vector) const
        {
            return vector2d<T>(
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
            dim::loadIdentity(*this);
            return *this;
        }
        
        //! Returns the determinant of this matrix.
        T determinant() const
        {
            return M[0]*M[3] - M[2]*M[1];
        }
        
        bool getInverse(matrix2<T> &InverseMat) const
        {
            T d = determinant();
            
            if (d == T(0))
                return false;
            
            d = T(1) / d;
            
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
        inline matrix2<T>& scale(const vector2d<T> &Vector)
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
        
        void setRotation(T Rotation, bool UseDegrees = true)
        {
            if (UseDegrees)
                Rotation = Rotation * static_cast<T>(M_PI) / T(180);
            
            const T s = sin(Rotation);
            const T c = cos(Rotation);
            
            M[0] =  c; M[2] = s;
            M[1] = -s; M[3] = c;
        }
        
        /* === Row & columns === */
        
        inline vector2d<T> getRow(s32 Position) const
        {
            switch (Position)
            {
                case 0: return vector2d<T>(M[0], M[2]);
                case 1: return vector2d<T>(M[1], M[3]);
            }
            return vector2d<T>();
        }
        
        inline const vector2d<T>& getColumn(u32 Position) const
        {
            return *reinterpret_cast<const vector2d<T>*>(&M[Position << 1]);
        }
        inline vector2d<T>& getColumn(u32 Position)
        {
            return *reinterpret_cast<vector2d<T>*>(&M[Position << 1]);
        }
        inline void setColumn(u32 Position, const vector2d<T> &Vec)
        {
            getColumn(Position) = Vec;
        }
        
        inline void setScale(const vector2d<T> &Scale)
        {
            getColumn(0).setLength(Scale.X);
            getColumn(1).setLength(Scale.Y);
        }
        inline vector2d<T> getScale() const
        {
            return vector2d<T>(
                getColumn(0).getLength(),
                getColumn(1).getLength()
            );
        }
        
        //! \todo Not implemented yet
        inline T getRotation() const
        {
            return T(0); //todo ...
        }
        
        inline matrix2<T> getTransposed() const
        {
            matrix2<T> Mat;
            getTransposed(Mat);
            return Mat;
        }
        
        inline void getTransposed(matrix2<T> &Other) const
        {
            dim::transpose(Other, *this);
        }
        
        inline matrix2<T>& setTransposed()
        {
            dim::transpose(*this);
            return *this;
        }
        
        inline T trace() const
        {
            return dim::trace(*this);
        }
        
        vector2d<T> interpolate(const vector2d<T> &Other, const T &t) const
        {
            vector2d<T> Mat;
            
            for (u32 i = 0; i < 4; ++i)
                math::lerp(Mat.M[i], M[i], Other.M[i], t);
            
            return Mat;
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
        
        template <typename B> inline matrix2<B> cast() const
        {
            matrix2<B> Result;
            
            for (u32 i = 0; i < 4; ++i)
                Result.M[i] = static_cast<B>(M[i]);
            
            return Result;
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
