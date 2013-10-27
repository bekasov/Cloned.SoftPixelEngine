/*
 * Matrix arithmetic header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_MATRIX_ARITHMETIC_H__
#define __SP_MATRIX_ARITHMETIC_H__


#include "Base/spMathCore.hpp"

#include <math.h>
#include <cstdlib>


namespace sp
{
namespace dim
{


/* === Functions === */

/**
Matrix multiplication "core" function.
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
template <size_t Num, typename T> inline bool matrixMul(T* const Out, T const * const A, T const * const B)
{
    if (Out != A && Out != B)
    {
        for (size_t i = 0, n = Num*Num; i < n; ++i)
        {
            Out[i] = T(0);
            
            for (size_t j = 0; j < Num; ++j)
                Out[i] += A[ i % Num + j * Num ] * B[ i - (i % Num) + j ];
        }
        return true;
    }
    return false;
}

/**
Matrix multiplication function.
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

template <size_t Num, typename T> inline bool matrixMul(T* const Out, T const * const In, const T &Scalar)
{
    for (size_t i = 0, n = Num*Num; i < n; ++i)
        Out[i] = In[i] * Scalar;
}

//! \see dim::matrixMul
template < template <typename> class M, typename T > inline bool matrixMul(M<T> &Out, const M<T> &In, const T &Scalar)
{
    matrixMul<M<T>::NUM, T>(&Out[0], &In[0], Scalar);
}

//! \see dim::matrixMul
template <size_t Num, typename T> void matrixAdd(T* Out, T const * const A, T const * const B)
{
    for (size_t i = 0, n = Num*Num; i < n; ++i)
        Out[i] = A[i] + B[i];
}

//! \see dim::matrixMul
template < template <typename> class M, typename T > inline bool matrixAdd(M<T> &Out, const M<T> &A, const M<T> &B)
{
    matrixAdd<M<T>::NUM, T>(&Out[0], &A[0], &B[0]);
}

//! \see dim::matrixMul
template <size_t Num, typename T> void matrixSub(T* Out, T const * const A, T const * const B)
{
    for (size_t i = 0, n = Num*Num; i < n; ++i)
        Out[i] = A[i] - B[i];
}

//! \see dim::matrixMul
template < template <typename> class M, typename T > inline bool matrixSub(M<T> &Out, const M<T> &A, const M<T> &B)
{
    matrixSub<M<T>::NUM, T>(&Out[0], &A[0], &B[0]);
}

/**
Loads the matrix identity. Thereafter, the matrix includes all zeros except
for the diagonal, where it contains only ones.
\param[out] Out Pointer to the output matrix.
\note This pointer must never be null!
\since Version 3.3
\ingroup group_arithmetic
*/
template <size_t Num, typename T> void loadIdentity(T* const Out)
{
    for (size_t i = 0, n = Num*Num; i < n; ++i)
        Out[i] = T(i % (Num + 1) == 0 ? 1 : 0);
}

/**
Loads the matrix identity. For more information see the first "loadIdentity" function.
\since Version 3.3
\ingroup group_arithmetic
*/
template < template <typename> class M, typename T > inline void loadIdentity(M<T> &Out)
{
    loadIdentity<M<T>::NUM, T>(&Out[0]);
}

/**
Determines whether the specified matrix has the identity or not.
\param[in] Matrix Constant pointer to the matrix which is to be checked for identity.
\return True if the specified matrix has the identity. This happens when all
elements are zero, except the elements on the main diagonal.
\note This pointer must never be null!
\since Version 3.3
\ingroup group_arithmetic
*/
template <size_t Num, typename T> bool hasIdentity(T const * const Matrix)
{
    for (size_t col = 0; col < Num; ++col)
    {
        for (size_t row = 0; row < Num; ++row)
        {
            if (!math::equal(Matrix[col * Num + row], T(col == row ? 1 : 0)))
                return false;
        }
    }
    return true;
}

template < template <typename> class M, typename T > inline bool hasIdentity(const M<T> &Matrix)
{
    return hasIdentity<M<T>::NUM, T>(&Matrix[0]);
}

/**
Returns the trace of the specified matrix. This is the sum of the elements on the main diagonal.
\param[in] Matrix Pointer to the matrix whose trace is to be computed.
\return Trace of the matrix (M(0, 0) + M(1, 1) + ... + M(n, n)).
\note This pointer must never be null!
\since Version 3.3
\ingroup group_arithmetic
*/
template <size_t Num, typename T> T trace(T const * const Matrix)
{
    T Tr = T(0);
    
    for (size_t i = 0; i < Num; ++i)
        Tr += Matrix[i * Num + i];
    
    return Tr;
}

template < template <typename> class M, typename T > inline T trace(const M<T> &Matrix)
{
    return trace<M<T>::NUM, T>(&Matrix[0]);
}

/*
Transposes the specified matrix. Only the relevant matrix elements will be swaped.
\tparam Num Specifies the number of matrix rows and columns (i.e. the matrix has Num x Num elements, e.g. Num = 4 -> 4x4 Matrix).
\tparam T Specifies the base data type.
\param[in,out] Matrix Pointer to the matrix which is to be transposed.
\note This pointer must never be null!
\since Version 3.3
\ingroup group_arithmetic
*/
template <size_t Num, typename T> void transpose(T* const Matrix)
{
    for (size_t i = 0; i + 1 < Num; ++i)
    {
        for (size_t j = 1; j + i < Num; ++j)
        {
            std::swap(
                Matrix[i*(Num + 1) + j],
                Matrix[(j + i)*Num + i]
            );
        }
    }
}

/**
Transposes the specified matrix. For more information see the other "transpose" function.
\since Version 3.3
\ingroup group_arithmetic
*/
template < template <typename> class M, typename T > inline void transpose(M<T> &Matrix)
{
    transpose<M<T>::NUM, T>(&Matrix[0]);
}

/**
Makes a transposed copy of the specified matrix.
\param[out] Out Pointer to the output matrix.
\param[in] In Pointer to the input matrix which is to be transposed.
\note These pointers must never be null!
\since Version 3.3
\ingroup group_arithmetic
*/
template <size_t Num, typename T> void transpose(T* const Out, T const * const In)
{
    for (size_t col = 0; col < Num; ++col)
    {
        for (size_t row = 0; row < Num; ++row)
            Out[col*Num + row] = In[row*Num + col];
    }
}

/**
Transposes the specified matrix. For more information see the other "transpose" function.
\since Version 3.3
\ingroup group_arithmetic
*/
template < template <typename> class M, typename T > inline void transpose(M<T> &Out, const M<T> &In)
{
    transpose<M<T>::NUM, T>(&Out[0], &In[0]);
}

/**
Compares the two specified matrices A and B for equalilty.
\tparam M Specifies the matrix type. This class needs a static constant member called "NUM"
holding the number of matrix rows and columns (e.g. matrix2, matrix3 or matrix4).
\tparam T Specifies the data type. This can be floating point type (e.g. float or double) or
an integer type (e.g. int, unsigned int etc.).
\param[in] A Specifies the first matrix.
\param[in] B Specifies the second matrix.
\return True if the two matrices A and B are equal.
\note This function is only used as base function for the matrix comparision operators (for matrix2, matrix3 and matrix4).
\see math::equal
\since Version 3.3
*/
template < template <typename> class M, typename T > inline bool compareMatEqual(const M<T> &A, const M<T> &B)
{
    for (size_t i = 0, n = M<T>::NUM * M<T>::NUM; i < n; ++i)
    {
        if (!math::equal<T>(A[i], B[i]))
            return false;
    }
    return true;
}

//! \see dim::compareMatEqual
template < template <typename> class M, typename T > inline bool compareMatNotEqual(const M<T> &A, const M<T> &B)
{
    return !compareMatEqual(A, B);
}


} // /namespace dim

} // /namespace sp


#endif



// ================================================================================
