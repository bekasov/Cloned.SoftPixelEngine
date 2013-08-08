/*
 * Vector arithmetic header
 * 
 * This file is part of the "SoftPixel Engine" (Copyright (c) 2008 by Lukas Hermanns)
 * See "SoftPixelEngine.hpp" for license information.
 */

#ifndef __SP_VECTOR_ARITHMETIC_H__
#define __SP_VECTOR_ARITHMETIC_H__


#include "Base/spMathCore.hpp"

#include <math.h>


namespace sp
{
namespace dim
{


/* === Enumerations === */

//! Axis direction types.
enum EAxisTypes
{
    AXIS_X_POSITIVE = 0,
    AXIS_X_NEGATIVE,
    AXIS_Y_POSITIVE,
    AXIS_Y_NEGATIVE,
    AXIS_Z_POSITIVE,
    AXIS_Z_NEGATIVE,
};


/* === Functions === */

/**
Returns the cross product of the specified 3D vectors A and B.
\tparam T Specifies the vector type. This type must have the member varaibles X, Y and Z
and a constructor which takes three parameters for these coordinates.
This could be vector3d, vector4d or your own class for a vector with three components.
\param[in] A Specifies the first vector.
\param[in] B Specifies the second vector.
\return Cross product which is of the same type as the input parameters.
\since Version 3.3
\ingroup group_arithmetic
*/
template <typename T> inline T cross(const T &A, const T &B)
{
    return T(
        A.Y*B.Z - B.Y*A.Z,
        B.X*A.Z - A.X*B.Z,
        A.X*B.Y - B.X*A.Y
    );
}

/**
Dot product "core" function. See more about that on the second "dot" function.
\tparam Num Specifies the number of vector components.
\tparam T Specifies the base data type.
\param[in] A Pointer to the first vector.
\param[in] B Pointer to the second vector.
\return Dot product of the two vectors A and B.
\note These pointers must never be null!
\since Version 3.3
\ingroup group_arithmetic
*/
template <u32 Num, typename T> inline T dot(const T* A, const T* B)
{
    T p = T(0);
    
    /* Accumulate the product of all respective vector components */
    for (u32 i = 0; i < Num; ++i)
        p += A[i]*B[i];
    
    return p;
}

/**
Returns the dot- (or rather scalar-) product of the specified 3D vectors A and B.
\tparam Va Specifies the vector type. This type must have a static constant field
named "NUM" specifying the number of elements of the vector type.
This could be vector3d, vector4d or your own class for a vector with three components.
\tparam Vb Same rules as for Va.
\tparam T Specifies the data type. This should be a floating point type (e.g. float or double).
\param[in] A Specifies the first vector.
\param[in] B Specifies the second vector.
\note A and B maybe from different vector types, e.g. A maybe from type of "vector3d" and B maybe
from type of "vector4d". In this case the smallest number of elements will be used,
i.e. vector3d::NUM (which is 3 of course).
\return Dot product which is of the same type as the input parameters.
\since Version 3.3
\ingroup group_arithmetic
*/
template <
    template <typename> class Va,
    template <typename> class Vb,
    typename T >
inline T dot(const Va<T> &A, const Vb<T> &B)
{
    return dot<
        /* Determine lowest common number of elements */
        (Va<T>::NUM <= Vb<T>::NUM ?
            Va<T>::NUM :
            Vb<T>::NUM),
        T>(
            /* Pass vectors A and B to core "dot" function */
            &A[0], &B[0]
        );
}

/**
Vector length "core" function. See more about that on the second "length" function.
\tparam Num Specifies the number of vector components.
\tparam T Specifies the base data type.
\param[in] Vec Pointer to the vector.
\return Length of the specified vector.
\note This pointer must never be null!
\since Version 3.3
\ingroup group_arithmetic
*/
template <u32 Num, typename T> inline T length(const T* Vec)
{
    return sqrt(dot<Num, T>(Vec, Vec));
}

/**
Returns the length of the specified 3D vector.
\tparam V Specifies the vector type. This must be suitable for the 'dot' function.
\tparam T Specifies the data type. This should be a floating point type (e.g. float or double).
\param[in] Vec Specifies the vector whose length is to be computed.
\return Length (or rather the euclidean norm) of the specified vector.
\see dot
\since Version 3.3
\ingroup group_arithmetic
*/
template < template <typename> class V, typename T > inline T length(const V<T> &Vec)
{
    return length<V<T>::NUM, T>(&Vec[0]);
}

/**
Returns the distance bewteen the two specified 3D vectors A and B.
\tparam V Specifies the vector type. This must be suitable for the 'length'
function and must implement the member operator '-='.
\tparam T Specifies the data type. This should be a floating point type (e.g. float or double).
\param[in] A Specifies the first vector.
\param[in] B Specifies the second vector.
\return Distance (with the euclidean norm) between the two vectors. This is equivalent to the following code:
\code
length(B - A);
\endcode
\see length
\since Version 3.3
\ingroup group_arithmetic
*/
template < template <typename> class V, typename T > inline T distance(const V<T> &A, const V<T> &B)
{
    /* Get vector (B - A) */
    V<T> Vec = B;
    Vec -= A;
    
    /* Compute length of the vector */
    return length(Vec);
}

/**
Vector normalization "core" function. See more about that on the second "normalize" function.
\tparam Num Specifies the number of vector components.
\tparam T Specifies the base data type.
\param[in,out] Vec Pointer to the vector which is to be normalized.
\return True if the vector could be normalized. Otherwise it must be a zero vector which is not normalizable.
\note This pointer must never be null!
\since Version 3.3
\ingroup group_arithmetic
*/
template <u32 Num, typename T> inline bool normalize(T* Vec)
{
    /* Get the squared vector length first */
    T n = dot<Num, T>(Vec, Vec);
    
    /* Check for zero vector */
    if (n == T(0))
        return false;
    
    /* Check if the vector is already normalized */
    if (n != T(1))
    {
        /* Compute reciprocal square root of the squared length */
        n = T(1) / sqrt(n);
        
        /* Normalize the vector with the inverse length for optimization */
        for (u32 i = 0; i < Num; ++i)
            Vec[i] *= n;
    }
    
    return true;
}

/**
Normalizes the specified 3D vector. After that the vector has the length of 1.
\tparam V Specifies the vector type. This must be suitable for the 'dot' function.
\tparam T Specifies the data type. This should be a floating point type (e.g. float or double).
\param[in,out] Vec Specifies the vector which is to be normalized.
\note If the specified vector is a zero vector (i.e. X, Y and Z are all zero) this function has no effect.
\see dot
\since Version 3.3
\ingroup group_arithmetic
*/
template < template <typename> class V, typename T > inline void normalize(V<T> &Vec)
{
    normalize<V<T>::NUM, T>(&Vec[0]);
}

/**
Vector angle "core" function. See more about that on the second "angle" function.
\tparam Num Specifies the number of vector components.
\tparam T Specifies the base data type.
\param[in] A Pointer to the first vector.
\param[in] B Pointer to the second vector.
\return Angle (in radian) between the two vectors.
\note These pointers must never be null!
\since Version 3.3
\ingroup group_arithmetic
*/
template <u32 Num, typename T> inline T angle(const T* A, const T* B)
{
    return acos(dot<Num, T>(A, B) / (length<Num, T>(A)*length<Num, T>(B)));
}

/**
Returns the angle (in radian) bewteen the two specified 3D vectors A and B.
\tparam Va Specifies the vector type. This must be suitable for the 'length'.
\tparam Vb Same rules as for Va.
\tparam T Specifies the data type. This should be a floating point type (e.g. float or double).
\param[in] A Specifies the first vector.
\param[in] B Specifies the second vector.
\return Angle (in radian) between the two vectors. Use the following code to get the angle in degree:
\code
angle(A, B) * math::RAD;
\endcode
\see length
\since Version 3.3
\ingroup group_arithmetic
*/
template <
    template <typename> class Va,
    template <typename> class Vb,
    typename T >
inline T angle(const Va<T> &A, const Vb<T> &B)
{
    return angle<
        /* Determine lowest common number of elements */
        (Va<T>::NUM <= Vb<T>::NUM ?
            Va<T>::NUM :
            Vb<T>::NUM),
        T>(
            /* Pass vectors A and B to core "angle" function */
            &A[0], &B[0]
        );
}

/**
Returns the dominant axis of the specified vector.
\tparam Specifies the data type (this should be float or double).
\param[in] Vec Constant pointer to the vector array. The array must have at least 3 elements!
\return The dominant axis type. This is a value of the "EAxisTypes" enumeration.
\see EAxisTypes
\note This pointer must never be null!
\since Version 3.3
\ingroup group_arithmetic
*/
template <typename T> inline EAxisTypes getDominantAxis(const T* Vec)
{
    /* Get absolute vector */
    const T AbsX = std::abs(Vec[0]);
    const T AbsY = std::abs(Vec[1]);
    const T AbsZ = std::abs(Vec[2]);
    
    /* Find dominant axis */
    if (AbsX >= AbsY && AbsX >= AbsZ)
        return (Vec[0] > 0 ? AXIS_X_POSITIVE : AXIS_X_NEGATIVE);
    if (AbsY >= AbsX && AbsY >= AbsZ)
        return (Vec[1] > 0 ? AXIS_Y_POSITIVE : AXIS_Y_NEGATIVE);
    return (Vec[2] > 0 ? AXIS_Z_POSITIVE : AXIS_Z_NEGATIVE);
}

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
\ingroup group_arithmetic
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

/**
\see compareVecEqual
\ingroup group_arithmetic
*/
template < template <typename> class V, typename T > inline bool compareVecNotEqual(const V<T> &A, const V<T> &B)
{
    return !compareVecEqual(A, B);
}

/**
\see compareVecEqual
\ingroup group_arithmetic
*/
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

/**
\see compareVecEqual
\ingroup group_arithmetic
*/
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

/**
\see compareVecEqual
\ingroup group_arithmetic
*/
template < template <typename> class V, typename T > inline bool compareVecLessThanOrEqual(const V<T> &A, const V<T> &B)
{
    return compareVecLessThan(A, B) || compareVecEqual(A, B);
}

/**
\see compareVecEqual
\ingroup group_arithmetic
*/
template < template <typename> class V, typename T > inline bool compareVecGreaterThanOrEqual(const V<T> &A, const V<T> &B)
{
    return compareVecGreaterThan(A, B) || compareVecEqual(A, B);
}


} // /namespace dim

} // /namespace sp


#endif



// ================================================================================
