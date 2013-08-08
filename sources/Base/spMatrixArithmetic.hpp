/*
 * Matirx arithmetic header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_MATRIX_ARITHMETIC_H__
#define __SP_MATRIX_ARITHMETIC_H__


#include "Base/spMathCore.hpp"

#include <math.h>


namespace sp
{
namespace dim
{


/* === Functions === */

/**
Matrix multiplicatiohn function.
\tparam Num Specifies the number of matrix rows and columns (i.e. the matrix has Num x Num elements, e.g. Num = 4 -> 4x4 Matrix).
\tparam T Specifies the base data type.
\param[out] Out Pointer to the output matrix. Must not be the same pointer as for parameters "A" and "B"!
\param[in] A Pointer to the first matrix.
\param[in] B Pointer to the second matrix.
\return True if the matrices could be multiplied. Otherwise at least on of the two parameters ("A" or "B")
is the same pointer as for the output matrix.
\note These pointers must never be null!
\since Version 3.3
\ingroup group_arithmetic
*/
template <u32 Num, typename T> inline bool matrixMul(T* Out, const T* A, const T* B)
{
    if (Out != A && Out != B)
    {
        for (u32 i = 0, n = Num*Num; i < n; ++i)
        {
            Out[i] = T(0);
            
            for (u32 j = 0; j < Num; ++j)
                Out[i] += A[ i % Num + j * Num ] * B[ i - (i % Num) + j ];
        }
        return true;
    }
    return false;
}

/**
Matrix multiplicatiohn function.
\tparam M Specifies the matrix type. This type must have a static constant field
named "NUM" specifying the number of rows and columns of the matrix type.
This could be matrix2, matrix3 or matrix4.
\tparam T Specifies the data type. This should be a floating point type (e.g. float or double).
\param[out] Out Specifies the output matrix. Must not be the same object as for parameters "A" and "B"!
\param[in] A Specifies the first matrix.
\param[in] B Specifies the second matrix.
\return True if the matrices could be multiplied. Otherwise at least on of the two parameters ("A" or "B")
is the same object as for the output matrix.
\since Version 3.3
\ingroup group_arithmetic
*/
template < template <typename> class M, typename T > inline bool matrixMul(M<T> &Out, const M<T> &A, const M<T> &B)
{
    matrixMul<M<T>::NUM, T>(&Out[0], &A[0], &B[0]);
}

/**
Loads the matrix identity. Thereafter, the matrix includes all zeros except
for the diagonal, where it contains only ones.
\param[out] Out Pointer to the output matrix.
\since Version 3.3
\ingroup group_arithmetic
*/
template <u32 Num, typename T> void matrixIdentity(T* Out)
{
    for (u32 col = 0; col < Num; ++col)
    {
        for (u32 row = 0; row < Num; ++row)
            Out[i] = (row == col ? T(1) : T(0));
    }
}

template < template <typename> class M, typename T > inline bool matrixIdentity(M<T> &Out)
{
    matrixIdentity<M<T>::NUM, T>(&Out[0]);
}

template <u32 Num, typename T> void matrixAdd(T* Out, const T* A, const T* B)
{
    for (u32 i = 0, n = Num*Num; i < n; ++i)
        Out[i] = A[i] + B[i];
}

template <u32 Num, typename T> void matrixSub(T* Out, const T* A, const T* B)
{
    for (u32 i = 0, n = Num*Num; i < n; ++i)
        Out[i] = A[i] - B[i];
}

#if 0

/**
Compares the two specified vectors A and B for equalilty.
\tparam V Specifies the vector type. This class needs a static constant member called "NUM"
holding the number of vector components (e.g. vector2d, vector3d or vector4d).
\tparam T Specifies the data type. This can be floating point type (e.g. float or double) or
an integer type (e.g. int, unsigned int etc.).
\param[in] A Specifies the first vector.
\param[in] B Specifies the second vector.
\return True if the two vectors A and B are equal.
\note This function is only used as base function for the vector comparision operators (for vector2d, vector3d and vector4d).
\see math::equal
\since Version 3.3
*/
template < template <typename> class V, typename T > inline bool compareVecEqual(const V<T> &A, const V<T> &B)
{
    for (u32 i = 0; i < V<T>::NUM; ++i)
    {
        if (!math::equal<T>(A[i], B[i]))
            return false;
    }
    return true;
}

//! \see compareVecEqual
template < template <typename> class V, typename T > inline bool compareVecNotEqual(const V<T> &A, const V<T> &B)
{
    return !compareVecEqual(A, B);
}

//! \see compareVecEqual
template < template <typename> class V, typename T > inline bool compareVecLessThan(const V<T> &A, const V<T> &B)
{
    for (u32 i = 0; i < V<T>::NUM; ++i)
    {
        if (A[i] < B[i])
            return true;
        if (A[i] > B[i])
            return false;
    }
    return false;
}

//! \see compareVecEqual
template < template <typename> class V, typename T > inline bool compareVecGreaterThan(const V<T> &A, const V<T> &B)
{
    for (u32 i = 0; i < V<T>::NUM; ++i)
    {
        if (A[i] > B[i])
            return true;
        if (A[i] < B[i])
            return false;
    }
    return false;
}

//! \see compareVecEqual
template < template <typename> class V, typename T > inline bool compareVecLessThanOrEqual(const V<T> &A, const V<T> &B)
{
    return compareVecLessThan(A, B) || compareVecEqual(A, B);
}

//! \see compareVecEqual
template < template <typename> class V, typename T > inline bool compareVecGreaterThanOrEqual(const V<T> &A, const V<T> &B)
{
    return compareVecGreaterThan(A, B) || compareVecEqual(A, B);
}

#endif


} // /namespace dim

} // /namespace sp


#endif



// ================================================================================
